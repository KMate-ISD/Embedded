from src.rbpi_mqtt.miro_mqtt_client_catalogue import Miro_mqtt_client_catalogue

try:
    mr = Miro_mqtt_client_catalogue("192.168.1.85", 1883, "admin/debug", "auth/user")
    mr.add_client(kmate_mqtt="lorimmer_mqtt")
    mr.clients["kmate_mqtt"].on_connect = mr.on_connect
    mr.clients["kmate_mqtt"].on_message = mr.on_message
    mr.connect_client("kmate_mqtt")
    for topic in mr.topics:
        mr.clients["kmate_mqtt"].subscribe(topic)
    mr.clients["kmate_mqtt"].loop_start()
    input("Press ENTER to exit...")

except Exception as e:
    print(f"{e}")
finally:
    mr.clients["kmate_mqtt"].disconnect()
    mr.clients["kmate_mqtt"].loop_stop()