#!/usr/bin/env python

import RPi.GPIO as GPIO
from src.rbpi_gpio.miro_rgb import Miro_rgb
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
            self.led.blue_on()
            blocks = [int(c) for c in blocks]
            id, data = self.__read_block_loop(blocks)
            print(id)
            self.print_data_readable(data, blocks[0])
            self.led.blue_off()
            self.led.green_pulse(0.25)
        except Exception:
            self.led.red_pulse(0.25)
            raise

    def write(self, text, block_initial):
        try:
            self.led.blue_on()
            data = self.break_down_text(text)
            block_count = len(data)
            blocks = range(block_initial, block_count + block_initial)
            id, data = self.__write_block_loop(data, blocks)
            print(id)
            self.print_data_readable(data, block_initial)
            self.led.blue_off()
            self.led.green_flash(0.05, block_count)
        except Exception:
            self.led.red_flash(0.05, 10)
            raise
