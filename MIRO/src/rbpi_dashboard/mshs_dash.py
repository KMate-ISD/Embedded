import urllib.request as url
from src.miro_helper import Miro_helper

class Mshs_dash():
  @staticmethod
  def upload_to_dashboard(field, value):
    ts_write = f'https://api.thingspeak.com/update?api_key={Miro_helper.ThingSpeak_API_key}&field'
    u = url.urlopen(f"{ts_write}{field}={value}")
    
    ret = u.read()
    u.close()

    return(ret)