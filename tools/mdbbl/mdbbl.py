#!/usr/bin/env python3
"""Pymodbus asynchronous client example.

An example of a single threaded synchronous client.

usage: simple_client_async.py

All options must be adapted in the code
The corresponding server must be started before e.g. as:
    python3 server_sync.py
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

config = Configuration(
    width=32,
    polynomial=0x04C11DB7,
    init_value=0xffffffff,
    final_xor_value=0,
    reverse_input=False,
    reverse_output=False,
)

async def run_async_simple_client(port: str, slave: int, baud: int, file: str):
    """Run async client."""
    # activate debugging
    pymodbus_apply_logging_config("DEBUG")

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
        
        stack = unpack('<I',binary[0:4])
        stack = stack[0]
        reset = unpack('<I',binary[4:8])
        reset = reset[0]

        calculator = Calculator(config)

        flash = True
        if (stack & int(0x20000000) == 0):
            #probably not flash image
            flash = False
        if (reset & int(0x8000000) == 0):
            flash = False

        if flash:
            if vectab == int(0x8000800):
                target = 'A'
            elif vectab == int(0x8008000):
                target = 'B'
            if reset > int(0x8000800) and reset < int(0x8008000):
                source = 'A'
            elif reset > int(0x8008000):
                source = 'B'
            if (source != target):
                print(f"Binary ({source}) is incompatible with target ({target})")
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
        if(len(binary_block) <= length):
            # send the actual packet
            vals = [int.from_bytes(binary_block[i:i+2],byteorder="little",signed=False) for i in range(0, len(binary_block), 2)]
            await client.write_registers(address=90,slave=slave,values=vals)
            val = unpack('<H',pack('<2s',b"bl"))
            vals = [int(val[0]), int(val[0]^0x5a5a)]
            await client.write_registers(address=0, slave=slave,values=vals)
            rr = await client.read_holding_registers(address=0, slave=slave,count=2)
            resp = pack('<2H',rr.registers[0],rr.registers[1])
            print(resp)        


    print("close connection")
    client.close()


if __name__ == "__main__":

    if (len(sys.argv) > 1):
        file = sys.argv[1]
    else:
        file = "tools/mdbbl/payload/pl_test/payload.bin"

    if (len(sys.argv) > 2):
        port = sys.argv[2]
    else:
        port = "/dev/ttyUSB3"

    if (len(sys.argv) > 3):
        slave = int(sys.argv[3])
    else:
        slave = 1;
    
    if (len(sys.argv) > 4):
        baud = int(sys.argv[4])
    else:
        baud = 57600;

    asyncio.run(
        run_async_simple_client(port, slave, baud, file), debug=True
    )
