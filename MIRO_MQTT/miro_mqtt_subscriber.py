import paho.mqtt.client as mqtt

mqtt_username = "kmate_mqtt"
mqtt_password = "lorimmer_mqtt"
mqtt_topic = "admin/debug"
mqtt_broker_ip = "192.168.1.85"

client = mqtt.Client()
client.username_pw_set(mqtt_username, mqtt_password)

# rc = error code returned on connecting to the broker
def on_connect(client, userdata, flags, rc):
    print(f"Connected. (rc: {str(rc)})")

    # Subscribe after connect
    client.subscribe(mqtt_topic)

# msg = message
# userdata = sender details
def on_message(client, userdata, msg):
    #print(f"Topic: {msg.topic}\nMessage: {str(msg.payload)}")
    pl = int.from_bytes(msg.payload, "big", signed=False)
    print("%#0.8X"%pl)

# Subscribe to events
client.on_connect = on_connect
client.on_message = on_message

# Connect to broker
client.connect(mqtt_broker_ip, 1883)

# Do the loop
client.loop_forever()
client.disconnect()