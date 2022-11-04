from src.rbpi_mqtt.miro_mqtt_action_handler import Miro_mqtt_action_handler

try:
    ah = Miro_mqtt_action_handler("192.168.1.85", 1883, "admin/debug", "auth/user")
    creds = ah.generate_credentials(12)
    ah.save_credentials(creds)
    ah.add_client(**dict({creds}))
    ah.register_on_connect(ah.on_connect)
    ah.register_on_message(ah.on_message)
    ah.register_on_disconnect(ah.on_disconnect)
    ah.connect()
    for topic in ah.topics:
        ah.subscribe(topic)
    ah.start()
    input("Press ENTER to finish...")

except Exception as e:
    print(f"{e}")
finally:
    ah.stop()
    ah.disconnect()