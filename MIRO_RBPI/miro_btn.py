import RPi.GPIO as GPIO
from miro_gpio import Miro_input
from miro_rgb import Miro_rgb

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

try:
    if __name__ == "__main__":
        pin = 36
        btn = Miro_btn(pin)
        rgb = Miro_rgb(11, 13, 15)
        def cb(channel):
            print("Button pressed")
            rgb.blue_flash()
        btn.add_handler(pin, GPIO.RISING, cb)
        
        exit = input("Press 'Enter' to remove handlers...\n")
        btn.remove_handlers(pin)

        exit = input("Press 'Enter' to exit...\n")

except Exception as e:
    print(f"{e}")
finally:
    GPIO.cleanup()
    print("Exiting script.")