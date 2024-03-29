import subprocess
from pprint import pprint

DEBUG = 1

class Miro_helper():
    mosquitto_config = "/etc/mosquitto/mosquitto.conf"
    passwordfile = "/etc/mosquitto/passwd"
    pid_path = "/run/mosquitto/"
    pid_file = "mosquitto.pid"
    wifi_details = "/etc/wpa_supplicant/wpa_supplicant.conf"
    ThingSpeak_API_key = "9DIN8SPGW71TKIYD"

# STATIC
    @staticmethod
    def debug(msg):
        if DEBUG:
            if type(msg) is list:
                pprint(msg)
            else:
                print(msg)

    @staticmethod
    def display_message(msg):
        return(f"{msg.topic} | {msg.payload}")

    @staticmethod
    def display_message_hex(msg):
        pl = [f"{msg.payload[i]:#04x}\n" if i%4==3 else f"{msg.payload[i]:#04x} " for i in range(len(msg.payload))]
        return(''.join(pl)[:-1])
    
    @staticmethod
    def get_ip():
        host = subprocess.run(["hostname", "-I"], capture_output=True)
        ip_str = host.stdout.decode().split(' ')[0]
        ip = ip_str.split('.')
        return(ip_str, [int(i) for i in ip])

# CLASS
    @classmethod
    def get_port(cls):
        with open(cls.mosquitto_config, 'r') as file:
            lines = file.readlines()
        listener = [line for line in lines if line.startswith("listener")]
        return(int(next(iter(listener)).split(' ')[-1].strip()))
    
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