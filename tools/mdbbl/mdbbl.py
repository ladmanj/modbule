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
    vector = rr.registers[3]
    print("buffer address", hex(address),"length", length,"actual app. base", hex(vector))

    # uint8_t[] binary payload
    with open(file, 'rb') as file_t:
        binary = bytearray(file_t.read())

    # prepend LE uint32_t load address (for payload with metadata)
    binary = pack('<I',address) + binary
    blen = len(binary)

    # compute CRC for load address + payload
    calculator = Calculator(config)
    crc = calculator.checksum(binary)

    # prepend LE uint32_t CRC and uint32_t length of CRC'ed buffer
    binary = pack('<2I', crc, blen) + binary

    # if the whole packet fits into anounced buffer, send it
    if(len(binary) <= length):
        # send the actual packet
        vals = [int.from_bytes(binary[i:i+2],byteorder="little",signed=False) for i in range(0, len(binary), 2)]
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
        port = "payload/pl_test/payload.bin"

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
