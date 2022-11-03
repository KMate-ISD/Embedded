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

    @staticmethod
    def display_message_hex(msg):
        pl = [f"{msg.payload[i]:#04x}\n" if i%4==3 else f"{msg.payload[i]:#04x} " for i in range(len(msg.payload))]
        print(''.join(pl))

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
        Miro_mqtt_client_catalogue.display_message_hex(msg)
        if msg.topic == "auth/user" and msg.paylod == "OK":
            print("save pw, send confirm")