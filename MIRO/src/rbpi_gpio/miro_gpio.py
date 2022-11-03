import RPi.GPIO as GPIO

class Miro_gpio:
    def __init__(self, *pins, numbering, pin_mode) -> None:
        self.numbering = numbering
        self.pin_mode = pin_mode
        self.pins = []
        for pin in pins:
            self.pins.append(pin)
        GPIO.setmode(self.numbering)

    def setup(self, pin, pull_up_down=GPIO.PUD_OFF, initial=GPIO.UNKNOWN):
        GPIO.setup(pin, self.pin_mode, pull_up_down=pull_up_down, initial=initial)


class Miro_input(Miro_gpio):
    def __init__(self, *pins, numbering) -> None:
        super().__init__(*pins, numbering=numbering, pin_mode=GPIO.IN)
        self.callbacks = []
    
    def setup_all(self, pull_up_down):
        for pin in self.pins:
            self.setup(pin, pull_up_down=pull_up_down)
    

class Miro_output(Miro_gpio):
    def __init__(self, *pins, numbering, initial) -> None:
        super().__init__(*pins, numbering=numbering, pin_mode=GPIO.OUT)
        self.statuses = [initial]*len(self.pins)

    def setup(self, pin, initial):
        super().setup(pin, initial=initial)
        self.statuses[self.pins.index(pin)] = initial
    
    def setup_all(self, initial):
        for pin in self.pins:
            self.setup(pin, initial=initial)

    def set_pin(self, pin):
        GPIO.output(pin, GPIO.HIGH)

    def clear_pin(self, pin):
        GPIO.output(pin, GPIO.LOW)