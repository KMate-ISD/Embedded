import RPi.GPIO as GPIO
import sys
from time import sleep

class miro_led:
    def __init__(self, led_pins, numbering=GPIO.BOARD):
        self.pins = led_pins
        self.numbering = numbering
        self.leds = {
            "red"   : self.pins[0],
            "green" : self.pins[1],
            "blue"  : self.pins[2]}
        self.__init_pins()
    
    def __init_pins(self, pin_mode=GPIO.OUT, initial=GPIO.LOW):
        GPIO.setmode(self.numbering)
        for pin in self.pins:
            GPIO.setup(pin, pin_mode, initial)
    
    @staticmethod
    def __set_pin(pin):
        GPIO.output(pin, GPIO.HIGH)

    @staticmethod
    def __clear_pin(pin):
        GPIO.output(pin, GPIO.LOW)

    def change_red(self, pin):
        self.leds["red"] = pin
    def change_green(self, pin):
        self.leds["green"] = pin
    def change_blue(self, pin):
        self.leds["blue"] = pin

    def red_off(self):
        self.__clear_pin(self.leds["red"])
    def green_off(self):
        self.__clear_pin(self.leds["green"])
    def blue_off(self):
        self.__clear_pin(self.leds["blue"])

    def red_on(self):
        self.__set_pin(self.leds["red"])
    def green_on(self):
        self.__set_pin(self.leds["green"])
    def blue_on(self):
        self.__set_pin(self.leds["blue"])

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
            else:
                cla[flag] = int(cla[flag])
    print(cla)

    #led = miro_led(cla["-L"])