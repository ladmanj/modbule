#!/usr/bin/env python3

# Pymodbus modbule code uploader.

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
import argparse
import time

config = Configuration(
    width=32,
    polynomial=0x04C11DB7,
    init_value=0xffffffff,
    final_xor_value=0,
    reverse_input=False,
    reverse_output=False,
)

def chunker(seq, size):
    return (seq[pos:pos + size] for pos in range(0, len(seq), size))

async def upload_mbl_firmware(args):
    # Run async client.

    # activate debugging
    #pymodbus_apply_logging_config("DEBUG")

    print("get client")
    client = ModbusClient.AsyncModbusSerialClient(
        port=args.port,
        # timeout=10,
        # retries=3,
        # retry_on_empty=False,
        # strict=True,
        baudrate=args.baud,
        bytesize=8,
        parity=args.par,
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
        rr = await client.read_input_registers(address=17, count=4, slave=args.slave)
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
    if address == 0 or length == 0:
        print(f"Invalid response")
        client.close()
        return

    vectab = int(0x8000000) + rr.registers[3]
    print("buffer address", hex(address),"length", length,"actual app. base", hex(vectab))

    # uint8_t[] binary payload
    with open(args.payload, 'rb') as file_t:
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
                await client.write_registers(address=register,slave=args.slave,values=batch)
                register += len(batch)
            vals = unpack('<2H',pack('<4s',b"bl86"))
            await client.write_registers(address=0, slave=args.slave,values=vals)
            rr = await client.read_holding_registers(address=0, slave=args.slave,count=2)
            resp = pack('<2H',rr.registers[0],rr.registers[1]).decode("utf-8")
            print(block+1,"/",blocks,resp)
            if resp == "Err-":
                print(f"Block rejected.")
                client.close()
                return

    if flash:
        vals = unpack('<2H',pack('<I',app_crc))       # bug here
        if args.boot == 'primary':
            register = 4                # or here
        if args.boot == 'secondary':
            register = 6                # and/or here
        if args.boot != 'none':
            await client.write_registers(address=register,slave=args.slave,values=vals)  # or here
            vals = unpack('<2H',pack('<4s',b"wc-9"))
            await client.write_registers(address=0, slave=args.slave,values=vals)
            rr = await client.read_holding_registers(address=0, slave=args.slave,count=2)
            resp = pack('<2H',rr.registers[0],rr.registers[1]).decode("utf-8")
            print(resp)
            
    if args.restart == 'clean':
        vals = unpack('<2H',pack('<4s',b"rs()"))
    if args.restart == 'fail':
        vals = unpack('<2H',pack('<4s',b"fail"))
    if args.restart:
        time.sleep(0.5)
        await client.write_registers(address=0, slave=args.slave,values=vals)

    print("close connection")
    client.close()

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Upload code')

    parser.add_argument('--port',default="/dev/ttyUSB3", type=str)
    parser.add_argument('--slave',default=1, type=int)
    parser.add_argument('--baud',default=57600, type=int)
    parser.add_argument('--par', choices=['N','O','E'], default='N')
    parser.add_argument('--boot', choices=['primary','secondary','none'], default='none')
    parser.add_argument('--payload',default="tools/mdbbl/payload/pl_test/payload.bin", type=str)
    parser.add_argument('--restart',choices=['clean','fail'])
    args = parser.parse_args()
    print(args)


    asyncio.run(
        upload_mbl_firmware(args), debug=False
    )
