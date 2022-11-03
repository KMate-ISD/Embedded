from enum import Enum
import RPi.GPIO as GPIO
from miro_led import Miro_led

class Color(Enum):
    RED     = 0
    GREEN   = 1
    BLUE    = 2

class Miro_rgb(Miro_led):
    def __init__(self, *pins, numbering=GPIO.BOARD, initial=GPIO.LOW) -> None:
        if len(pins) > 3:
            raise Exception("Too many pins. Add 3 in RGB order.")
        super().__init__(*pins, numbering=numbering, initial=initial)

    def red_off(self):
        self.led_off(Color.RED.value)
    def green_off(self):
        self.led_off(Color.GREEN.value)
    def blue_off(self):
        self.led_off(Color.BLUE.value)

    def red_on(self):
        self.led_on(Color.RED.value)
    def green_on(self):
        self.led_on(Color.GREEN.value)
    def blue_on(self):
        self.led_on(Color.BLUE.value)

    def red_flash(self, rest=0.2, cycles=10):
        self.led_flash(Color.RED.value, rest, cycles)
    def green_flash(self, rest=0.2, cycles=10):
        self.led_flash(Color.GREEN.value, rest, cycles)
    def blue_flash(self, rest=0.2, cycles=10):
        self.led_flash(Color.BLUE.value, rest, cycles)

    def red_pulse(self, rest=1):
        self.led_pulse(Color.RED.value, rest)
    def green_pulse(self, rest=1):
        self.led_pulse(Color.GREEN.value, rest)
    def blue_pulse(self, rest=1):
        self.led_pulse(Color.BLUE.value, rest)