#!/usr/bin/env python

import RPi.GPIO as GPIO
import time
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
    
    @staticmethod
    def parse_lock_bytes(buf, arr):
        for i in range (4,8):
            buf.append(arr[0] >> i & 1)
        for i in range (0,8):
            buf.append(arr[1] >> i & 1)
        return(buf)

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
        lock_bytes = self.reader.MFRC522_Read(2)[2:]
        lock_bits = self.parse_lock_bytes([], lock_bytes)
        for i in range(len(blocks)):
            if status == self.reader.MI_OK and not lock_bits[blocks[i] - 4]:
                self.reader.MFRC522_Write(blocks[i], data[i])
            else:
                error_list.append(blocks[i])
        self.reader.MFRC522_StopCrypto1()
        if error_list:
            raise Exception(f"Erroneous or locked block(s): {error_list}.")
        compare_i = [data[i][0:4] for i in range(len(blocks))]
        compare_o = [self.reader.MFRC522_Read(n)[0:4] for n in blocks]
        assert compare_i == compare_o, f"Part of the data couldn't be written. Check tag data blocks for reference: {compare_o}."
        return(id, compare_o)

    def __read_block_loop(self, blocks):
        id, data = self.__read_block(blocks)

        t0 = t = time.time()
        while t - t0 < 10 and not id:
            id, data = self.__read_block(blocks)
            t = time.time()
        if not t - t0 < 10:
            raise TimeoutError("Reading timeout.")

        return(id, data)

    def __write_block_loop(self, data, blocks):
        id, text_in = self.__write_block(data, blocks)

        t0 = t = time.time()
        while t - t0 < 10 and not id:
            id, text_in = self.__write_block(data, blocks)
            t = time.time()
        if not t - t0 < 10:
            raise TimeoutError("Writing timeout.")

        return(id, text_in)

    def read(self, blocks):
        try:
            self.led.blue_on()
            blocks = [int(c) for c in blocks]
            id, data = self.__read_block_loop(blocks)
            print(f"({id})")
            self.print_data_readable(data, blocks[0])
            self.led.green_pulse(0.25)
        except TimeoutError:
            self.led.blue_flash(0.25, 7)
            raise
        except Exception:
            self.led.red_pulse(0.25)
            raise
        finally:
            self.led.all_off()

    def write(self, text, block_initial):
        try:
            self.led.blue_on()
            data = self.break_down_text(text)
            block_count = len(data)
            blocks = range(block_initial, block_count + block_initial)
            id, data = self.__write_block_loop(data, blocks)
            print(f"({id})")
            self.print_data_readable(data, block_initial)
            self.led.green_flash(0.05, block_count)
        except TimeoutError:
            self.led.blue_flash(0.25, 7)
            raise
        except Exception as e:
            self.led.red_flash(0.05, 10)
            raise Exception(f"Error during write loop: {e}")
        finally:
            self.led.all_off()