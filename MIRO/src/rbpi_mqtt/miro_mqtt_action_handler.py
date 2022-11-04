from src.rbpi_mqtt.miro_mqtt_client_handler import Miro_mqtt_client_handler

class Miro_mqtt_action_handler(Miro_mqtt_client_handler):
    def __init__(self, broker_ip, port, *topics) -> None:
        super().__init__(broker_ip, port, *topics)
    
    def on_message(self, client, userdata, msg):
        super().on_message(client, userdata, msg)
        if msg.payload == bytearray("OK", "ascii"):
            pass
    
    def generate_credentials():
        pass
    
    def save_credentials():
        pass