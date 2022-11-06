import RPi.GPIO as GPIO
import sys
import time
from functools import reduce
from mfrc522 import MFRC522
from src.miro_helper import Miro_helper
from pprint import pprint
from src.rbpi_gpio.miro_btn import Miro_btn
from src.rbpi_mqtt.miro_mqtt_action_handler import Miro_mqtt_action_handler
from src.rbpi_rfid.miro_rfid import Miro_rfid
from src.rbpi_gpio.miro_rgb import Miro_rgb


BTN_PIN     = 36

DATA_BEGIN  = 4

LED_R       = 11
LED_G       = 13
LED_B       = 15
REST        = 0.2

BROKER      = "192.168.1.85"
PORT        = 1883

AUTH_TIME   = 20
SUPER_U     = "user"
SUPER_P     = "pass"

OK          = 0
NOK         = 1

DEBUG       = 0


global btn, rgb, rc, rfid, mqtt, is_busy

def debug(msg):
    if DEBUG:
        pprint(msg)

def initialize_context(btn=BTN_PIN, rgb=(LED_R, LED_G, LED_B), broker=BROKER, port=PORT):
    btn = Miro_btn(btn)
    rgb = Miro_rgb(*rgb)
    rc = MFRC522()
    rfid = Miro_rfid(rc, rgb)
    mqtt = Miro_mqtt_action_handler(broker, port)

    return(btn, rgb, rc, rfid, mqtt)

def notify_OK(msg, pulse=1):
    rgb.green_pulse(pulse)
    print(msg)

def notify_NOK(msg, pulse=1):
    rgb.red_pulse(pulse)
    print(msg)

def setup(args):
    if len(args) == 13:
        params = [
            "-b", # Pushbutton pin  2 digits hexa   board numbering   (e.g. -b 24)
            "-L", # RGB led pins    2 digits hexa   board numbering   (e.g. -L 0B0D0F)
            "-s", # rfid first block to write, decimal
            "-r", # rest time in seconds, decimal
            "--ip", # mqtt broker
            "--port"]

        args.pop(0)
        flags = args[::2]
        args = args[1::2]
        cla = dict(zip(flags, args))
        print(f"Arguments count: {len(cla)}")

        for flag in params:
            if flag in cla:
                if flag == "-b":
                    cla[flag] = next(iter([int(i, 16) for i in cla[flag]]))
                elif flag == "-L":
                    cla[flag] = [int(i, 16) for i in cla[flag]]
                    cla[flag] = [(n * 16 + cla[flag][i + 1]) for i, n in enumerate(cla[flag]) if not i%2]
                if flag == "-s" or flag == "--port":
                    cla[flag] = int(cla[flag])
                elif flag == "-r":
                    cla[flag] = float(cla[flag])
        debug(cla)

        is_parameterized = reduce(lambda a, b: a and b, [key in cla.keys() for key in params])

        if is_parameterized:
            return(initialize_context(*cla.values()))
    
    else:
        return(initialize_context())

def start_listener():
    mqtt.add_client(**dict({SUPER_U: SUPER_P}))
    mqtt.register_on_connect(mqtt.on_connect)
    mqtt.register_on_message(mqtt.on_message)
    mqtt.register_on_disconnect(mqtt.on_disconnect)
    mqtt.connect()
    mqtt.start()

def write_creds_to_tag(ctx):
    try:
        ret = NOK
        global is_busy

        print("Issuing credentials...")

        # check if writing is already happening
        if is_busy:
            return(ret)
        
        # lock process
        is_busy = True

        # generate username and password for a node
        creds = mqtt.generate_credentials(4)
        mqtt.save_credentials(creds)

        # add channel for authentication
        auth = f"auth/{creds[0]}"
        mqtt.add_topic(auth)
        mqtt.subscribe(auth)
        print(f"User created: {creds[0]}.")

        # Preparing data to write to rfid tag
        userpass = ''.join(creds)
        ssid, psk = Miro_helper.get_wifi_credentials()
        start = f"{chr(0xE0)}{chr(len(ssid))}"
        next = f"{chr(0xED)}{chr(len(psk))}"
        stop = f"{chr(0xEA)}{chr(len(ssid) + (len(psk)))}"
        data = f"{userpass}{start}{ssid}{next}{psk}{stop}"
        debug(data)

        # save prepared data to rfid tag
        rfid.write(data, DATA_BEGIN)

        # release lock on exception
        
        notify_OK("Credentials saved to rfid tag.", 0.15)

        # the node should confirm tag delivery in {AUTH_TIME} seconds
        msg = f"Bring your tag near the MIRO node. Time available: "

        t0 = t = time.time()
        while t - t0 < AUTH_TIME and auth not in mqtt.last_msgs:
            t = time.time()
            d = t - t0
            if not round(d, 1)%1:
                rgb.blue_pulse(0.1)
                print("\r%s%s"%(msg, f"{AUTH_TIME - round(d)}".ljust(2)), end='')
            time.sleep(0.1)
        print()
        
        # no feedback received, terminating access
        if d > AUTH_TIME:
            raise Exception(f"No confirmation received.")

        # confirmation of delivery received through the authentication channel
        else:
            ret = OK
            notify_OK(f"Credentials successfully transferred.\nUser saved.")

    except Exception as e:
        msg = "User not saved."
        mqtt.revoke_access(creds[0])
        notify_NOK(f"{e}\n{msg}") if type(e) is not TimeoutError else print(f"{e}\n{msg}")

    finally:
        # clean up
        mqtt.topics.remove(auth)
        mqtt.unsubscribe(auth)
        is_busy = False
        print()
        return(ret)

try:
    if __name__ == "__main__":
        args = sys.argv[:]
        
        btn, rgb, rc, rfid, mqtt = setup(args)
        is_busy = False
        start_listener()

        btn.add_handler(next(iter(btn.pins)), GPIO.RISING, write_creds_to_tag)

        exit = input('Type "EXIT" to exit.\n')
        while (exit != "EXIT" and exit != "qq"):
            exit = input()

except Exception as e:
    print(f"main: {e}")
finally:
    mqtt.stop()
    mqtt.disconnect()
    GPIO.cleanup()
    print("Pin behaviour set to GPIO.IN on all pins.")
