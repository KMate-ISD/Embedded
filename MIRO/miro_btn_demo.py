import RPi.GPIO as GPIO
from src.rbpi_gpio.miro_btn import Miro_btn
from src.rbpi_gpio.miro_rgb import Miro_rgb

try:
    if __name__ == "__main__":
        pin = 36
        btn = Miro_btn(pin)
        rgb = Miro_rgb(11, 13, 15)
        def cb(channel):
            print("Button pressed")
            rgb.blue_pulse(0.125)
        btn.add_handler(pin, GPIO.RISING, cb)
        
        exit = input("Press 'Enter' to remove handlers...\n")
        btn.remove_handlers(pin)

        exit = input("Press 'Enter' to exit...\n")

except Exception as e:
    print(f"{e}")
finally:
    GPIO.cleanup()
    print("Exiting script.")
