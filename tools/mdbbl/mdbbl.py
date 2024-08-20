#!/usr/bin/env python3
"""Pymodbus modbule code uploader.

usage: todo

"""
import asyncio

import pymodbus.client as ModbusClient
from pymodbus import (
    ExceptionResponse,
    Framer,
    ModbusException,
    pymodbus_apply_logging_config,
)
from struct import *
from crc import Calculator, Configuration
import sys
import math
from enum import Enum

config = Configuration(
    width=32,
    polynomial=0x04C11DB7,
    init_value=0xffffffff,
    final_xor_value=0,
    reverse_input=False,
    reverse_output=False,
)

class Boot_type(Enum):

    NONE = 0
    PRIMARY = 1
    SECONDARY = 2

def chunker(seq, size):
    return (seq[pos:pos + size] for pos in range(0, len(seq), size))

async def upload_mbl_firmware(port: str, slave: int, baud: int, file: str, boot: Boot_type):
    """Run async client."""
    # activate debugging
    #pymodbus_apply_logging_config("DEBUG")

    print("get client")
    client = ModbusClient.AsyncModbusSerialClient(
        port,
        # timeout=10,
        # retries=3,
        # retry_on_empty=False,
        # strict=True,
        baudrate=baud,
        bytesize=8,
        parity="N",
        stopbits=1,
        # handle_local_echo=False,
    )

    print("connect to server")
    await client.connect()
    # test client is connected
    assert client.connected

    print("get and verify data")
    try:
        # See all calls in client_calls.py
        rr = await client.read_input_registers(address=17, count=4, slave=slave)
    except ModbusException as exc:
        print(f"Received ModbusException({exc}) from library")
        client.close()
        return
    if rr.isError():
        print(f"Received Modbus library error({rr})")
        client.close()
        return
    if isinstance(rr, ExceptionResponse):
        print(f"Received Modbus library exception ({rr})")
        # THIS IS NOT A PYTHON EXCEPTION, but a valid modbus message
        client.close()

    address = rr.registers[0] << 16 | rr.registers[1]
    length = rr.registers[2]
    vectab = int(0x8000000) + rr.registers[3]
    print("buffer address", hex(address),"length", length,"actual app. base", hex(vectab))

    # uint8_t[] binary payload
    with open(file, 'rb') as file_t:
        binary = bytearray(file_t.read())
        blocks = int(math.ceil(len(binary)/length))
        tofill = length - len(binary)%length
        
        # if this is a image of flash, those are initial stack pointer and reset vector
        stack = unpack('<I',binary[0:4])[0]
        reset = unpack('<I',binary[4:8])[0]

        calculator = Calculator(config)

        flash = True
        if (stack & int(0x20000000) == 0) or (reset & int(0x8000000) == 0):
            #probably not flash image
            flash = False

        if flash:
            if vectab == int(0x8000800):
                target = 'A'
            elif vectab == int(0x8008000):
                target = 'B'
            if reset > int(0x8000800) and reset < int(0x8008000):
                source = 'A'
                address = int(0x8000800)
            elif reset > int(0x8008000):
                source = 'B'
                address = int(0x8008000)
            if (source == target):
                print(f"Binary {source} can't be uploaded when {target} is running.")
                client.close()
                return
            binary = binary + b'\xff' * tofill
            app_crc = calculator.checksum(binary)
            print("Crc of whole application " + hex(app_crc))
            
            

    for block in  range(blocks):
        binary_block = binary[block*length:(1+block)*length]
        # prepend LE uint32_t load address (for payload with metadata)
            
        binary_block = pack('<I',address) + binary_block
        blen = len(binary_block)
    
        # compute CRC for load address + payload
        
        crc = calculator.checksum(binary_block)
    
        # prepend LE uint32_t CRC and uint32_t length of CRC'ed buffer
        binary_block = pack('<2I', crc, blen) + binary_block
    
        # if the whole packet fits into anounced buffer, send it
        if(len(binary_block) <= (length + 3 * 4)):
            # send the actual packet
            vals = [int.from_bytes(binary_block[i:i+2],byteorder="little",signed=False) for i in range(0, len(binary_block), 2)]
            register = 90
            for batch in chunker(vals, 64):
                await client.write_registers(address=register,slave=slave,values=batch)
                register += len(batch)
            val = unpack('<H',pack('<2s',b"bl"))
            vals = [int(val[0]), int(val[0]^0x5a5a)]
            await client.write_registers(address=0, slave=slave,values=vals)
            rr = await client.read_holding_registers(address=0, slave=slave,count=2)
            resp = pack('<2H',rr.registers[0],rr.registers[1])
            print(resp)

    if flash:
        vals = pack('<I',app_crc)
        if boot == Boot_type.PRIMARY:
            register = 4
        if boot == Boot_type.SECONDARY:
            register = 6
        if boot != Boot_type.NONE:
            await client.write_registers(address=register,slave=slave,values=vals)
            val = unpack('<H',pack('<2s',b"wc"))
            vals = [int(val[0]), int(val[0]^0x5a5a)]
            await client.write_registers(address=0, slave=slave,values=vals)

    print("close connection")
    client.close()

if __name__ == "__main__":

    if (len(sys.argv) > 1):
        file = sys.argv[1]
    else:
        file = "tools/mdbbl/payload/pl_test/payload.bin"

    if (len(sys.argv) > 2):
        if (sys.argv[2] == "primary"):
            boot = Boot_type.PRIMARY
        else: 
            if (sys.argv[2] == "secondary"):
                boot = Boot_type.SECONDARY
    else:
        boot = Boot_type.NONE

    if (len(sys.argv) > 3):
        port = sys.argv[3]
    else:
        port = "/dev/ttyUSB3"

    if (len(sys.argv) > 4):
        slave = int(sys.argv[4])
    else:
        slave = 1;
    
    if (len(sys.argv) > 5):
        baud = int(sys.argv[5])
    else:
        baud = 57600;
    


    asyncio.run(
        upload_mbl_firmware(port, slave, baud, file, boot), debug=False
    )
