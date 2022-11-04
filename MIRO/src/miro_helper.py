class Miro_helper():
    @staticmethod
    def display_message(msg):
        print(f"{msg.topic} | {msg.payload}")

    @staticmethod
    def display_message_hex(msg):
        pl = [f"{msg.payload[i]:#04x}\n" if i%4==3 else f"{msg.payload[i]:#04x} " for i in range(len(msg.payload))]
        print(''.join(pl))
    
    def get_mqtt_users():
        users = dict()
        with open("/etc/mosquitto/pwfile", 'r') as file:
            lines = file.readlines()
            iterator = iter(lines)

            for i in iterator:
                user, pw = i.split(':')
                users.update({user : pw})

        return(users)
    
    def get_wifi_credentials():
        with open("/etc/wpa_supplicant/wpa_supplicant.conf", 'r') as file:
            lines = file.readlines()
            ssid = [line for line in lines if line.startswith("ssid")][0].split('=')[1]
            psk = [line for line in lines if line.startswith("psk")][0].split('=')[1]
        
        return(ssid, psk)