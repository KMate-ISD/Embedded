import paho.mqtt.client as mqtt

class Miro_mqtt_client_catalogue():
    def __init__(self, broker_ip, port, *topics) -> None:
        self.broker_ip = broker_ip
        self.port = port
        self.topics = [topic for topic in topics]
        self.clients = {}

    @staticmethod
    def display_message(msg):
        print(f"{msg.topic} | {msg.payload}")

    def add_client(self, **credentials):
        self.clients.update({key : mqtt.Client() for key in credentials})
        for key in self.clients:
            self.clients[key].username_pw_set(key, credentials[key])

    def connect_client(self, username):
        self.clients[username].connect(self.broker_ip, self.port)

    def on_connect(self, client, userdata, flags, rc):
        if rc:
            print(f"Connection refused: {rc}")
        else:
            print("Connected.")

    def on_message(self, client, userdata, msg):
        Miro_mqtt_client_catalogue.display_message(msg)
        pl = [f"{msg.payload[i]:#04x}\n" if i%4==3 else f"{msg.payload[i]:#04x} " for i in range(len(msg.payload))]
        print(''.join(pl))

try:
    mr = Miro_mqtt_client_catalogue("192.168.1.85", 1883, "admin/debug")
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