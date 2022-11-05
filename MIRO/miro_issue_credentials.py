import RPi.GPIO as GPIO
import sys
import time
from functools import reduce
from mfrc522 import MFRC522
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

AUTH_TIME   = 30
SUPER_U     = "admin"
SUPER_P     = "1234"

OK          = 0
NOK         = 1


global btn, rgb, rc, rfid, mqtt, is_busy

def initialize_context(btn=BTN_PIN, rgb=(LED_R, LED_G, LED_B), broker=BROKER, port=PORT):
    btn = Miro_btn(btn)
    rgb = Miro_rgb(*rgb)
    rc = MFRC522()
    rfid = Miro_rfid(rc, rgb)
    mqtt = Miro_mqtt_action_handler(broker, port)

    return(btn, rgb, rc, rfid, mqtt)

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
        pprint(cla)

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

def write_creds_to_tag():
    ret = NOK

    # check if writing is already happening
    if is_busy:
        return(ret)
    
    # lock process
    is_busy = True

    # generate username and password for a node
    creds = mqtt.generate_credentials(12)
    mqtt.save_credentials(creds)

    # add channel for authentication
    auth = f"auth/{creds[0]}"
    mqtt.add_topic(auth)
    mqtt.subscribe(auth)

    # write credentials to rfid tag
    rfid.write(''.join(creds))

    # the node should confirm delivery in {AUTH_TIME} seconds
    t0 = t = time.time()
    while t - t0 < AUTH_TIME and auth not in mqtt.last_msgs:
        t = time.time()
        d = t - t0
        if not round(d, 1)%1:
            rgb.blue_pulse(0.1)
        time.sleep(0.1)
    
    # no feedback received, terminating access
    if d > AUTH_TIME:
        mqtt.revoke_access(creds[0])
        mqtt.topics.pop(auth)
        rgb.red_pulse(1)

    # confirmation of delivery received through the authentication channel
    else:
        ret = OK
        rgb.green_pulse(1)
    
    # release lock
    is_busy = False
    return(ret)

try:
    if __name__ == "__main__":
        args = sys.argv[:]
        
        btn, rgb, rc, rfid, mqtt = setup(args)
        is_busy = False
        start_listener()

        btn.add_handler(next(iter(btn.pins)), GPIO.RISING, write_creds_to_tag)

        exit = input('Type "EXIT" to exit.\n')
        while (exit != "EXIT"):
            exit = input()

except Exception as e:
    print(f"main: {e}")
finally:
    mqtt.stop()
    mqtt.disconnect()
    GPIO.cleanup()
    print("Pin behaviour set to GPIO.IN on all pins.")