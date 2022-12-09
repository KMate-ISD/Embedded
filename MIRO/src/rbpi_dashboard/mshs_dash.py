from time import sleep
import urllib.request as url

ts_write = 'https://api.thingspeak.com/update?api_key=9DIN8SPGW71TKIYD&field'
field = 5

print("Type 'qq' to exit the program.")
value = input("Type a value to update the dashboard.\n")

while (value != "qq"):
  u = url.urlopen(f"{ts_write}{field}={value}")
  print(u.read())
  u.close()

  value = input()

print("Run concluded. Bye!")