from src.rbpi_mqtt.miro_mqtt_client_handler import Miro_mqtt_client_handler
from src.rbpi_mqtt.miro_mqtt_action_handler import Miro_mqtt_action_handler

try:
    mr = Miro_mqtt_client_handler("192.168.1.85", 1883, "admin/debug", "auth/user")
    mr.add_client(kmate_mqtt="lorimmer_mqtt")
    mr.clients["kmate_mqtt"].on_connect = mr.on_connect
    mr.clients["kmate_mqtt"].on_message = mr.on_message
    mr.connect("kmate_mqtt")
    for topic in mr.topics:
        mr.clients["kmate_mqtt"].subscribe(topic)
    mr.clients["kmate_mqtt"].loop_start()
    input("Press ENTER to next...")

    mc = Miro_mqtt_action_handler("192.168.1.85", 1883, "admin/debug", "auth/user")
    mc.add_client(kmate_mqtt="lorimmer_mqtt")
    mc.register_on_connect(mc.on_connect)
    mc.register_on_message(mc.on_message)
    mc.connect()
    for topic in mc.topics:
        mc.subscribe(topic)
    mc.start()
    input("Press ENTER to next...")

except Exception as e:
    print(f"{e}")
finally:
    mr.clients["kmate_mqtt"].disconnect()
    mr.clients["kmate_mqtt"].loop_stop()
    mc.clients["kmate_mqtt"].loop_stop()
    mc.clients["kmate_mqtt"].disconnect()