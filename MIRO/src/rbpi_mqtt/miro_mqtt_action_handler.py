import os
import random
import subprocess
from src.rbpi_mqtt.miro_mqtt_client_handler import Miro_mqtt_client_handler
from src.miro_helper import Miro_helper

class Miro_mqtt_action_handler(Miro_mqtt_client_handler):
    def __init__(self, broker_ip, port, *topics) -> None:
        super().__init__(broker_ip, port, *topics)
        self.last_msgs = dict()
        # os.system(f"sudo chown kmate /run/mosquitto")
    
    def on_message(self, client, userdata, msg):
        super().on_message(client, userdata, msg)
        self.last_msgs.update({msg.topic: msg.payload})
    
    def generate_credentials(self, i=4):
        user = self.generate_username()

        while user in Miro_helper.get_mqtt_users():
            user = self.generate_username()

        pw = list()
        for i in range(i):
            r = random.randint(33, 126)
            while (r < 65 and r > 57):
                r = random.randint(33, 126)
            pw.append(chr(r))

        return(user, ''.join(pw))
    
    def generate_username(self, i=4):
        user = list()
        for i in range(i):
            r = random.randint(48, 122)
            while (r < 65 and r > 57) or (r < 97 and r > 90):
                r = random.randint(48, 122)
            user.append(chr(r))

        return(''.join(user))

    def revoke_access(self, *users):
        for user in users:
            subprocess.run(["mosquitto_passwd", "-D", Miro_helper.passwordfile, user])
        os.system(f"sudo kill -SIGHUP $(cat {Miro_helper.pid_path}{Miro_helper.pid_file})")

    def save_credentials(self, *credentials):
        for cred in credentials:
            subprocess.run(["mosquitto_passwd", "-b", Miro_helper.passwordfile, cred[0], cred[1]])
        os.system(f"sudo kill -SIGHUP $(cat {Miro_helper.pid_path}{Miro_helper.pid_file})")