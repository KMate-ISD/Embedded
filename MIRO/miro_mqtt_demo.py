from src.rbpi_mqtt.miro_mqtt_action_handler import Miro_mqtt_action_handler

try:
    mx = Miro_mqtt_action_handler("192.168.1.85", 1883, "admin/debug", "auth/user")
    creds = mx.generate_credentials(12)
    mx.save_credentials(creds)
    print(creds)
    print(type(creds))
    print(dict({creds}))
    print(type(dict({creds})))
    mx.add_client(**dict({creds}))
    mx.register_on_connect(mx.on_connect)
    mx.register_on_message(mx.on_message)
    mx.register_on_disconnect(mx.on_disconnect)
    mx.connect()
    for topic in mx.topics:
        mx.subscribe(topic)
    mx.start()
    input("Press ENTER to add next w/o client authentication...")
    
    my = Miro_mqtt_action_handler("192.168.1.85", 1883, "admin/debug", "auth/user")
    creds = my.generate_credentials(12)
    print(creds)
    my.add_client(**dict({creds}))
    my.register_on_connect(my.on_connect)
    my.register_on_message(my.on_message)
    my.register_on_disconnect(my.on_disconnect)
    my.connect()
    for topic in my.topics:
        my.subscribe(topic)
    my.start()
    input("Press ENTER to finish...")

except Exception as e:
    print(f"{e}")
finally:
    mx.stop()
    mx.disconnect()
    my.stop()
    my.disconnect()