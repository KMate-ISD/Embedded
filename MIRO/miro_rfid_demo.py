#!/usr/bin/env python

import RPi.GPIO as GPIO
import sys
from src.rbpi_rfid.miro_rfid import Miro_rfid
from src.rbpi_gpio.miro_rgb import Miro_rgb
from mfrc522 import MFRC522

try:
    if __name__ == "__main__":
        params = [
            "-B", # "--blocks"
            "-m", # "--mode"
            "-s", # "--start"
            "-t"] # "--text"

        args = sys.argv[:]
        args.pop(0)
        flags = args[::2]
        args = args[1::2]
        cla = dict(zip(flags, args))
        print(f"Arguments count: {len(cla)}")

        for flag in params:
            if flag in cla:
                if flag == "-s":
                    cla[flag] = int(cla[flag])
                elif flag == "-B":
                    cla[flag] = [int(i, 16) for i in cla[flag]]
                else:
                    pass
        print(cla)

        reader = MFRC522()
        led = Miro_rgb(11, 13, 15)
        miro = Miro_rfid(reader, led)

        if "-m" in cla:
            if cla["-m"] == 'w' and "-t" in cla and "-s" in cla:
                if cla["-s"] < 4:
                    raise Exception("Editing manufacturer data, lock bytes or OTP bytes is prohibited. Exiting script.")
                miro.write(cla["-t"], cla["-s"])
            elif cla["-m"] == 'r' and "-B" in cla:
                miro.read(cla["-B"])

except Exception as e:
    led.red_pulse(1)
    print(f"{e}")
finally:
    led.blue_pulse(0.1)
    GPIO.cleanup()
