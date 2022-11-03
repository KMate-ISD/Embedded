import RPi.GPIO as GPIO
from miro_gpio import Miro_output
from time import sleep

class Miro_led(Miro_output):
    def __init__(self, *pins, numbering=GPIO.BOARD, initial=GPIO.LOW) -> None:
        super().__init__(*pins, numbering=numbering, initial=initial)
        self.setup_all(initial)

    def __led_toggle(self, led_id):
        self.statuses[led_id] = not self.statuses[led_id]
        GPIO.output(self.pins[0], self.statuses[1])
        GPIO.output(self.pins[led_id], self.statuses[led_id])

    def __led_flash(self, led_id, rest, cycles, off=False):
        for i in range(cycles):
            self.__led_toggle(led_id)
            sleep(rest)
        if off:
            self.__led_off(led_id)

    def __led_pulse(self, led_id, rest):
        self.__led_toggle(led_id)
        sleep(rest)
        self.__led_toggle(led_id)
    
    def __led_off(self, led_id):
        self.statuses[led_id] = 0
        self.clear_pin(self.pins[led_id])
    
    def __led_on(self, led_id):
        self.statuses[led_id] = 1
        self.set_pin(self.pins[led_id])