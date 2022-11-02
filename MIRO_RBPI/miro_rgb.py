import RPi.GPIO as GPIO
import sys
from miro_gpio import Miro_output
from time import sleep

class Miro_rgb(Miro_output):
    def __init__(self, *pins, numbering=GPIO.BOARD, initial=GPIO.LOW) -> None:
        super().__init__(*pins, numbering=numbering)
        self.leds = dict(zip(("red", "green", "blue"), zip(pins[:3], [initial]*3)))
        self.leds = {key:list(self.leds[key]) for key in self.leds}
        self.setup_all(initial)

    def __led_toggle(self, color):
        led = self.leds[color]
        led[1] = not led[1]
        GPIO.output(led[0], led[1])

    def __led_flash(self, color, rest, cycles):
        for i in range(cycles):
            self.__led_toggle(color)
            sleep(rest)
        if cycles%2:
            self.__led_off(color)

    def __led_pulse(self, color, rest):
        if not self.leds[color][1]:
            self.__led_toggle(color)
            sleep(rest)
        self.__led_toggle(color)
    
    def __led_off(self, color):
        self.leds[color][1] = 0
        self.clear_pin(self.leds[color][0])
    
    def __led_on(self, color):
        self.leds[color][1] = 1
        self.set_pin(self.leds[color][0])

    def red_off(self):
        self.__led_off("red")
    def green_off(self):
        self.__led_off("green")
    def blue_off(self):
        self.__led_off("blue")

    def red_on(self):
        self.__led_on("red")
    def green_on(self):
        self.__led_on("green")
    def blue_on(self):
        self.__led_on("blue")

    def red_flash(self, rest=0.2, cycles=10):
        self.__led_flash("red", rest, cycles)
    def green_flash(self, rest=0.2, cycles=10):
        self.__led_flash("green", rest, cycles)
    def blue_flash(self, rest=0.2, cycles=10):
        self.__led_flash("blue", rest, cycles)

    def red_pulse(self, rest=1):
        self.__led_pulse("red", rest)
    def green_pulse(self, rest=1):
        self.__led_pulse("green", rest)
    def blue_pulse(self, rest=1):
        self.__led_pulse("blue", rest)

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