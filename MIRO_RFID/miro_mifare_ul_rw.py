#!/usr/bin/env python

import RPi.GPIO as GPIO
from mfrc522 import MFRC522

def read_block(reader, blocks):
        (status, TagType) = reader.MFRC522_Request(reader.PICC_REQIDL)
        if status != reader.MI_OK:
                return(None, None)
        (status, uid) = reader.MFRC522_Anticoll()
        if status != reader.MI_OK:
                return(None, None)
        id = uid_to_num(uid)
        reader.MFRC522_SelectTag(uid)
        data = []
        text_read = ''
        if status == reader.MI_OK:
                for block_num in blocks:
                        block_data = reader.MFRC522_Read(block_num)
                        if block_data:
                                data.append(block_data[:4])
        reader.MFRC522_StopCrypto1()
        return(id, data)

def write_block(reader, data, blocks):
        error_list = []
        (status, TagType) = reader.MFRC522_Request(reader.PICC_REQIDL)
        if status != reader.MI_OK:
                return(None, None)
        (status, uid) = reader.MFRC522_Anticoll()
        if status != reader.MI_OK:
                return(None, None)
        id = uid_to_num(uid)
        reader.MFRC522_SelectTag(uid)
        for i in range(len(blocks)):
                reader.MFRC522_Read(blocks[i])
                if status == reader.MI_OK:
                        reader.MFRC522_Write(blocks[i], data[i])
                else:
                        error_list.append(blocks[i])
        reader.MFRC522_StopCrypto1()
        if error_list:
                raise Exception(f"Write operation complete with errors. Block(s) {error_list} unreadable.")
        compare_i = [data[i][0:4] for i in range(len(blocks))]
        compare_o = [reader.MFRC522_Read(n)[0:4] for n in blocks]
        assert compare_i == compare_o, f"Write operation complete with errors. Part of the data couldn't be written. Check tag data blocks for reference: {compare_o}."
        return(id, compare_o)

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

def miro_read(blocks):
        blocks = [int(c) for c in blocks]
        id, data = read_block_loop(reader, blocks)
        print(id)
        print_data_readable(data)

def miro_write(text, block_initial):
        data = break_down_text(text)
        block_count = len(data)
        blocks = range(block_initial, block_count + block_initial)
        id, data = write_block_loop(reader, data, blocks)
        print(id)
        print_data_readable(data)

def print_data_readable(data):
        for i in range(len(data)):
                data_hex = ["%0.2x"%d for d in data[i]]
                print(f"BLOCK {i}\t{data_hex}")

def read_block_loop(reader, blocks):
        id, data = read_block(reader, blocks)
        while not id:
                id, data = read_block(reader, blocks)
        return(id, data)

def write_block_loop(reader, data, blocks):
        id, text_in = write_block(reader, data, blocks)
        while not id:
                id, text_in = write_block(reader, data, blocks)
        return(id, text_in)

def uid_to_num(uid):
        n = 0
        for i in range(0, 5):
            n = n * 256 + uid[i]
        return n

reader = MFRC522()

try:
        pass
finally:
        GPIO.cleanup()