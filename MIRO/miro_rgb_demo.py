import RPi.GPIO as GPIO
import sys
from miro_rgb import Miro_rgb

try:
    if __name__ == "__main__":
        params = [
            "-L", # "--leds 11 13 15"
            "-r", # "--red"
            "-g", # "--green"
            "-b", # "--blue"
            "-s"] # "--sleep"

        args = sys.argv[:]
        args.pop(0)
        flags = args[::2]
        args = args[1::2]
        cla = dict(zip(flags, args))
        print(f"Arguments count: {len(cla)}")

        for flag in params:
            if flag in cla:
                if flag == "-L":
                    cla[flag] = [int(i, 16) for i in cla[flag]]
                elif flag == "-s":
                    cla[flag] = float(cla[flag])
                else:
                    cla[flag] = int(cla[flag])
        print(cla)

        leds = cla["-L"]
        rest = cla["-s"]

        led = Miro_rgb(*leds)

        led.red_flash(rest)
        led.green_flash(rest)
        led.blue_flash(rest)

        led.red_pulse()
        led.green_pulse()
        led.blue_pulse()

except Exception as e:
    print(f"{e}")
finally:
    GPIO.cleanup()
    print("Exiting script.")