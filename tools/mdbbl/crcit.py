#!/usr/bin/env python3
"""Pymodbus asynchronous client example.

An example of a single threaded synchronous client.

usage: simple_client_async.py

All options must be adapted in the code
The corresponding server must be started before e.g. as:
    python3 server_sync.py
"""
import asyncio

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

async def compute_crc_block(binfile: str, size: int):
    with open(binfile, 'rb') as file_t:
        binary = bytearray(file_t.read())
    l = len(binary)
    if(l<size):
        binary = binary + b'\xff' * (size-l)

    calculator = Calculator(config)
    crc = calculator.checksum(binary)

    print(crc)


if __name__ == "__main__":

    if (len(sys.argv) > 1):
        file = sys.argv[1]
    else:
        file = "binary"
    if (len(sys.argv) > 2):
        filesize = int(sys.argv[2])
    else:
        filesize = 0;

    asyncio.run(
        compute_crc_block(file, filesize)
    )
