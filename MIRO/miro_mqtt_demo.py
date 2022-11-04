from src.rbpi_mqtt.miro_mqtt_action_handler import Miro_mqtt_action_handler

try:
    mc = Miro_mqtt_action_handler("192.168.1.85", 1883, "admin/debug", "auth/user")
    mc.add_client(kmate_mqtt="lorimmer_mqtt")
    mc.register_on_connect(mc.on_connect)
    mc.register_on_message(mc.on_message)
    mc.connect()
    for topic in mc.topics:
        mc.subscribe(topic)
    mc.start()
    input("Press ENTER to finish...")
    mc.register_on_disconnect(mc.on_disconnect)

except Exception as e:
    print(f"{e}")
finally:
    mc.clients["kmate_mqtt"].loop_stop()
    mc.clients["kmate_mqtt"].disconnect()