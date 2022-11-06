from pprint import pprint

DEBUG = 0

class Miro_helper():
    passwordfile = "/etc/mosquitto/pwfile"
    pid_path = "/run/mosquitto/"
    pid_file = "mosquitto.pid"
    wifi_details = "/etc/wpa_supplicant/wpa_supplicant.conf"

    @staticmethod
    def debug(msg):
        if DEBUG:
            pprint(msg)

    @staticmethod
    def display_message(msg):
        print(f"{msg.topic} | {msg.payload}")

    @staticmethod
    def display_message_hex(msg):
        pl = [f"{msg.payload[i]:#04x}\n" if i%4==3 else f"{msg.payload[i]:#04x} " for i in range(len(msg.payload))]
        print(''.join(pl))
    
    @classmethod
    def get_mqtt_users(cls):
        users = dict()
        with open(cls.passwordfile, 'r') as file:
            lines = file.readlines()
            iterator = iter(lines)

            for i in iterator:
                user, pw = i.split(':')
                users.update({user : pw})

        return(users)

    @classmethod
    def get_wifi_credentials(cls):
        with open(cls.wifi_details, 'r') as file:
            lines = file.readlines()
            ssid = [line for line in lines if "ssid=" in line][0].split('=')[1].strip()[1:-1]
            psk = [line for line in lines if "psk=" in line][0].split('=')[1].strip()[1:-1]
        
        return(ssid, psk)