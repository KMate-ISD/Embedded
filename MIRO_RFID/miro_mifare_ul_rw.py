#!/usr/bin/env python

import RPi.GPIO as GPIO
import sys
from miro_led import Miro_led
from mfrc522 import MFRC522

class Miro_rfid:
    def __init__(self, reader, led):
        self.reader = reader
        self.led = led

    @staticmethod
    def __uid_to_num(uid):
        n = 0
        for i in range(0, 5):
            n = n * 256 + uid[i]
        return n

    @staticmethod
    def break_down_text(text):
        size = len(text)
        ret = []
        buffer = []
        for i in range(size):
            buffer.append(text[i])
            if i%4 == 3:
                ret.append(buffer[:])
                buffer.clear()
        if buffer:
            ret.append(buffer[:])
        ret_bytes = []
        for block in ret:
            block_bytes = [ord(i) for i in block]
            ret_bytes.append(block_bytes)
        for block in ret_bytes:
            block.extend([0]*(16 - len(block)))
        return(ret_bytes)

    @staticmethod
    def print_data_readable(data, block_initial):
        for i in range(len(data)):
            data_hex = ["%0.2x"%d for d in data[i]]
            print(f"BLOCK {i + block_initial:>3}\t{data_hex}")

    def __read_block(self, blocks):
        (status, TagType) = self.reader.MFRC522_Request(self.reader.PICC_REQIDL)
        if status != self.reader.MI_OK:
            return(None, None)
        (status, uid) = self.reader.MFRC522_Anticoll()
        if status != self.reader.MI_OK:
            return(None, None)
        id = self.__uid_to_num(uid)
        self.reader.MFRC522_SelectTag(uid)
        data = []
        if status == self.reader.MI_OK:
            for block_num in blocks:
                block_data = self.reader.MFRC522_Read(block_num)
                if block_data:
                    data.append(block_data[:4])
        self.reader.MFRC522_StopCrypto1()
        return(id, data)

    def __write_block(self, data, blocks):
        error_list = []
        (status, TagType) = self.reader.MFRC522_Request(self.reader.PICC_REQIDL)
        if status != self.reader.MI_OK:
            return(None, None)
        (status, uid) = self.reader.MFRC522_Anticoll()
        if status != self.reader.MI_OK:
            return(None, None)
        id = self.__uid_to_num(uid)
        self.reader.MFRC522_SelectTag(uid)
        for i in range(len(blocks)):
            self.reader.MFRC522_Read(blocks[i])
            if status == self.reader.MI_OK:
                self.reader.MFRC522_Write(blocks[i], data[i])
            else:
                error_list.append(blocks[i])
        self.reader.MFRC522_StopCrypto1()
        if error_list:
            raise Exception(f"Write operation complete with errors. Block(s) {error_list} unreadable.")
        compare_i = [data[i][0:4] for i in range(len(blocks))]
        compare_o = [self.reader.MFRC522_Read(n)[0:4] for n in blocks]
        assert compare_i == compare_o, f"Write operation complete with errors. Part of the data couldn't be written. Check tag data blocks for reference: {compare_o}."
        return(id, compare_o)

    def __read_block_loop(self, blocks):
        id, data = self.__read_block(blocks)
        while not id:
            id, data = self.__read_block(blocks)
        return(id, data)

    def __write_block_loop(self, data, blocks):
        id, text_in = self.__write_block(data, blocks)
        while not id:
            id, text_in = self.__write_block(data, blocks)
        return(id, text_in)

    def read(self, blocks):
        try:
            blocks = [int(c) for c in blocks]
            id, data = self.__read_block_loop(blocks)
            print(id)
            self.print_data_readable(data, blocks[0])
            self.led.green_pulse()
        except Exception:
            self.led.red_pulse()
            raise

    def write(self, text, block_initial):
        try:
            data = self.break_down_text(text)
            block_count = len(data)
            blocks = range(block_initial, block_count + block_initial)
            id, data = self.__write_block_loop(data, blocks)
            print(id)
            self.print_data_readable(data, block_initial)
            self.led.green_flash(0.25)
        except Exception:
            self.led.red_flash(0.25)
            raise

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
        led = Miro_led([11, 13, 15])
        miro = Miro_rfid(reader, led)

        if "-m" in cla:
            if cla["-m"] == 'w' and "-t" in cla and "-s" in cla:
                if cla["-s"] < 4:
                    raise Exception("Editing manufacturer data, lock bytes or OTP bytes is prohibited. Exiting script.")
                miro.write(cla["-t"], cla["-s"])
            elif cla["-m"] == 'r' and "-B" in cla:
                miro.read(cla["-B"])

except Exception as e:
    led.red_pulse()
    print(f"{e}")
finally:
    led.blue_pulse(0.25)
    GPIO.cleanup()
    print("Exiting script.")