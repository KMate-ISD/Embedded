from enum import Enum
import RPi.GPIO as GPIO
import sys
from miro_led import Miro_led

class Miro_rgb(Miro_led):
    def __init__(self, *pins, numbering=GPIO.BOARD, initial=GPIO.LOW) -> None:
        if len(pins) > 3:
            raise Exception("Too many pins. Add 3 in RGB order.")
        super().__init__(*pins, numbering=numbering, initial=initial)

    def red_off(self):
        self.__led_off(Color.RED.value)
    def green_off(self):
        self.__led_off(Color.GREEN.value)
    def blue_off(self):
        self.__led_off(Color.BLUE.value)

    def red_on(self):
        self.__led_on(Color.RED.value)
    def green_on(self):
        self.__led_on(Color.GREEN.value)
    def blue_on(self):
        self.__led_on(Color.BLUE.value)

    def red_flash(self, rest=0.2, cycles=10):
        self.__led_flash(Color.RED.value, rest, cycles)
    def green_flash(self, rest=0.2, cycles=10):
        self.__led_flash(Color.GREEN.value, rest, cycles)
    def blue_flash(self, rest=0.2, cycles=10):
        self.__led_flash(Color.BLUE.value, rest, cycles)

    def red_pulse(self, rest=1):
        self.__led_pulse(Color.RED.value, rest)
    def green_pulse(self, rest=1):
        self.__led_pulse(Color.GREEN.value, rest)
    def blue_pulse(self, rest=1):
        self.__led_pulse(Color.BLUE.value, rest)

class Color(Enum):
    RED     = 0
    GREEN   = 1
    BLUE    = 2

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

        led.RED.value_flash(rest)
        led.GREEN.value_flash(rest)
        led.BLUE.value_flash(rest)

        led.RED.value_pulse()
        led.GREEN.value_pulse()
        led.BLUE.value_pulse()

except Exception as e:
    print(f"{e}")
finally:
    GPIO.cleanup()
    print("Exiting script.")