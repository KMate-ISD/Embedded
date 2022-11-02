import RPi.GPIO as GPIO
import sys
from time import sleep

class Miro_led:
    def __init__(self, led_pins, numbering=GPIO.BOARD, init_value=GPIO.LOW):
        self.pins = led_pins
        self.numbering = numbering
        self.leds = {
            "red"   : [self.pins[0], init_value],
            "green" : [self.pins[1], init_value],
            "blue"  : [self.pins[2], init_value]}
        self.__init_pins(init_value)

    def __init_pins(self, pin_mode=GPIO.OUT):
        GPIO.setmode(self.numbering)
        for key in self.leds:
            GPIO.setup(self.leds[key][0], pin_mode, initial=self.leds[key][1])

    @staticmethod
    def __set_pin(pin):
        GPIO.output(pin, GPIO.HIGH)

    @staticmethod
    def __clear_pin(pin):
        GPIO.output(pin, GPIO.LOW)

    def __led_toggle(self, color):
        led = self.leds[color]
        led[1] = not led[1]
        GPIO.output(led[0], led[1])

    def __led_flash(self, color, rest, cycles):
        for i in range(cycles):
            self.__led_toggle(color)
            sleep(rest)
        if cycles%2:
            self.__clear_pin(self.leds[color][0])

    def __led_pulse(self, color, rest):
        if not self.leds[color][1]:
            self.__led_toggle(color)
            sleep(rest)
        self.__led_toggle(color)

    def red_off(self):
        self.__clear_pin(self.leds["red"][0])
    def green_off(self):
        self.__clear_pin(self.leds["green"][0])
    def blue_off(self):
        self.__clear_pin(self.leds["blue"][0])

    def red_on(self):
        self.__set_pin(self.leds["red"][0])
    def green_on(self):
        self.__set_pin(self.leds["green"][0])
    def blue_on(self):
        self.__set_pin(self.leds["blue"][0])

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

    def led_off(self):
        self.red_off()
        self.green_off()
        self.blue_off()

try:
    if __name__ == "__main__":
        params = [
            "-L", # "--leds"
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

        led = miro_led(leds)

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