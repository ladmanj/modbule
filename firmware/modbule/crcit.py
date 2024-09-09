#!/usr/bin/env python3

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

async def compute_crc_block(binfile: str, size: int, identifier: str):
    with open(binfile, 'rb') as file_t:
        binary = bytearray(file_t.read())
    l = len(binary)
    if(l<size):
        binary = binary + b'\xff' * (size-l)

    calculator = Calculator(config)
    crc = calculator.checksum(binary)

    print("#define " + identifier + " " + hex(crc))


if __name__ == "__main__":

    if (len(sys.argv) > 1):
        file = sys.argv[1]
    else:
        file = "binary"
    if (len(sys.argv) > 2):
        filesize = int(sys.argv[2])
    else:
        filesize = 0;
    if (len(sys.argv) > 3):
        identifier = sys.argv[3]
    else:
        identifier = ""
    asyncio.run(
        compute_crc_block(file, filesize, identifier)
    )
