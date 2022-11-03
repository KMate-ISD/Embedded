import RPi.GPIO as GPIO
from miro_gpio import Miro_input

class Miro_btn(Miro_input):
    def __init__(self, *pins, numbering=GPIO.BOARD, pull_up_down=GPIO.PUD_DOWN, setup=1) -> None:
        super().__init__(*pins, numbering=numbering)
        if setup and pins:
            self.setup_all(pull_up_down)
    
    def add_handler(self, pin, event, *args):
        for callback in args:
            self.callbacks.append(callback)
            GPIO.add_event_detect(pin, event, callback=callback)
    
    def remove_handlers(self, pin):
        self.callbacks.clear()
        GPIO.remove_event_detect(pin)