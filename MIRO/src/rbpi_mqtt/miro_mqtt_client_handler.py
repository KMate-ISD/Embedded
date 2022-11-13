import paho.mqtt.client as mqtt
from src.miro_helper import Miro_helper

class Miro_mqtt_client_handler():
    def __init__(self, broker_ip, port, *topics) -> None:
        self.broker_ip = broker_ip
        self.port = port
        self.topics = [topic for topic in topics]
        self.clients = {}
    
    def __get_default_user_if_parameter_is_null__(self, username):
        if not username:
            return(next(iter(self.clients)))
        return(username)

    def on_connect(self, client, userdata, flags, rc):
        if rc:
            print(f"Connection refused: {rc}\n")
        else:
            print("Connected.\n")

    def on_disconnect(self, client, userdata, rc):
        print(f"Client disconnected: {client}")

    def on_message(self, client, userdata, msg):
        print(f"\n{Miro_helper.display_message(msg)}\n")
        Miro_helper.debug("\n")
        Miro_helper.debug(Miro_helper.display_message_hex(msg))

    def add_client(self, **credentials):
        self.clients.update({key : mqtt.Client() for key in credentials})
        for key in self.clients:
            self.clients[key].username_pw_set(key, credentials[key])
    
    def add_topic(self, topic):
        self.topics.append(topic)

    def connect(self, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].connect(self.broker_ip, self.port)
    
    def disconnect(self, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].disconnect()
    
    def loop(self, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].loop_forever()
    
    def publish(self, topic, payload=None, qos=0, retain=False, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].publish(topic, payload, qos, retain)
    
    def start(self, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].loop_start()
    
    def stop(self, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].loop_stop()
    
    def subscribe(self, topic, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].subscribe(topic)

    def unsubscribe(self, topic, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].unsubscribe(topic)
    
    def register_on_connect(self, callback, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].on_connect = callback

    def register_on_disconnect(self, callback, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].on_disconnect = callback

    def register_on_message(self, callback, username=0):
        username = self.__get_default_user_if_parameter_is_null__(username)
        self.clients[username].on_message = callback
