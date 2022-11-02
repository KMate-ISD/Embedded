import RPi.GPIO as GPIO

class Miro_gpio:
    def __init__(self, *pins, numbering, pin_mode) -> None:
        self.numbering = numbering
        self.pin_mode = pin_mode
        self.pins = []
        for pin in pins:
            self.pins.append(pin)
        GPIO.setmode(self.numbering)
    
    def __add_pin(self, pin):
        self.pins.append(pin)

    def setup(self, pin, pull_up_down=GPIO.PUD_OFF, initial=GPIO.UNKNOWN):
        GPIO.setup(pin, self.pin_mode, pull_up_down=pull_up_down, initial=initial)


class Miro_input(Miro_gpio):
    def __init__(self, *pins, numbering) -> None:
        super().__init__(*pins, numbering=numbering, pin_mode=GPIO.IN)
        self.callbacks = []
    
    def setup_all(self, pull_up_down=GPIO.PUD_DOWN):
        for pin in self.pins:
            self.setup(pin, pull_up_down=pull_up_down)
    
    def add_input(self, pin, setup=0):
        self.__add_pin(pin)
        if setup:
            self.setup(pin, pull_up_down=GPIO.PUD_DOWN)
    

class Miro_output(Miro_gpio):
    def __init__(self, *pins, numbering) -> None:
        super().__init__(*pins, numbering=numbering, pin_mode=GPIO.OUT)
    
    def setup_all(self, initial=GPIO.LOW):
        for pin in self.pins:
            super().setup(pin, initial=initial)

    def set_pin(pin):
        GPIO.output(pin, GPIO.HIGH)

    def clear_pin(pin):
        GPIO.output(pin, GPIO.LOW)