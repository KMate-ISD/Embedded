import functools
import paho.mqtt.client as mqtt
import sys
import time

DEBUG_MODE      = 1
default_payload = '\0'
is_exit_sent    = False

mqtt_username   = "kmate_mqtt"
mqtt_password   = "lorimmer_mqtt"
mqtt_topic      = "admin/debug"
mqtt_broker_ip  = "192.168.1.85"

def helper_prettify_bytes(bytes):
    payload = []
    for index in range(len(bytes)):
        if index%2 == 0:
            payload.append("sp")
        payload.append(bytes[index])
    payload[0] = "0x"
    return functools.reduce(lambda a, b: a + ("%0.2X"%b if b != "sp" else chr(0x20)), payload)

def on_connect(client, userdata, flags, rc):
    print(f"Client connected.")
    if DEBUG_MODE: print(f"Error code: {str(rc)}")

    client.subscribe(mqtt_topic, 0)

def on_disconnect(client, userdata, rc):
    print(f"Client disconnected.")
    if DEBUG_MODE: print(f"Error code: {str(rc)}")

def on_publish(client, userdata, mid):
    print(f"Message sent to broker.")
    if DEBUG_MODE: print(f"Mid variable: {str(mid)}")

def on_subscribe(client, userdata, mid, granted_qos):
    print(f"Successfully subscribed to a topic.")
    if DEBUG_MODE: print(f"Mid variable: {str(mid)}")

def on_message(client, userdata, msg):
    payload = int.from_bytes(msg.payload, "big", signed=False)
    # payload = helper_prettify_bytes(msg.payload)

    if payload == 0x45584954:
        global is_exit_sent
        print(f"Exit command received. Disconnecting client...")
        is_exit_sent = True
        client.disconnect()

    print(helper_prettify_bytes(msg.payload))

def main(argv):
    global default_payload
    global is_exit_sent

    opts = [opt for opt in argv if opt.startswith('-')]
    args = [arg for arg in argv if arg not in opts]

    if DEBUG_MODE:
        print(opts)
        print(args)

    if "-m" in opts:
        default_payload = args[opts.index("-m")]
        if DEBUG_MODE: print(default_payload)

    client = mqtt.Client()
    client.username_pw_set(mqtt_username, mqtt_password)

    client.on_connect    = on_connect
    client.on_disconnect = on_disconnect
    client.on_message    = on_message
    client.on_publish    = on_publish
    client.on_subscribe  = on_subscribe

    client.connect(mqtt_broker_ip, 1883)

    current_time = time.time()

    client.loop_start()
    while not is_exit_sent:
        ctim = time.time()

        if current_time + 5 < ctim:
            client.publish(mqtt_topic, default_payload, 0, False) # params: topic, payload, QoS, retain
            current_time = ctim
            print(f"is_exit_sent: {is_exit_sent}")

    client.disconnect()

if __name__ == "__main__":
    main(sys.argv[1:])