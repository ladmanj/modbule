#!/usr/bin/env python3

__author__ = "Jakub Ladman"
__copyright__ = "Copyright 2024"
__credits__ = ["Jakub Ladman, Pymodbus authors"]
__license__ = "GPL"
__version__ = "0.1"
__maintainer__ = "Jakub Ladman"
__email__ = "ladmanj@volny.cz"
__status__ = "Testing"

# Pymodbus modbule config updater.

import asyncio
import sys
import pymodbus.client as ModbusClient
from pymodbus import (
    ExceptionResponse,
    Framer,
    ModbusException,
    pymodbus_apply_logging_config,
)
import struct
import construct
import argparse

config = construct.Struct(
    "pri_app_crc"   / construct.Int32ul,
    "sec_app_crc"   / construct.Int32ul,
    "slave_addr"    / construct.Int8ul,
    "parity"        / construct.Int8ul,
    "baudrate"      / construct.Int16ul,
    "hwc"           / construct.Int16ul,
    "logic_conf"    / construct.Struct(
        "pli"           / construct.Array(8, construct.Struct(
            "invert"        / construct.Int16ul,
            "enable"        / construct.Int16ul
        )),
        "plo"           / construct.Struct(
            "invert"        / construct.Int8ul,
            "output"        / construct.Int8ul
        )
    ),
    "relay"         / construct.Struct(
        "index"         / construct.Int8ul,
        "mask"          / construct.Int8ul
    ),
    "logic"         / construct.Array(8, construct.Struct(
        "index"         / construct.Int8ul,
        "mask"          / construct.Int8ul
    )),
    "comparators" / construct.Array(8, construct.Struct(
        "index"         / construct.Int16ul, 
	    "greater"       / construct.Int16sl, 
	    "equal"         / construct.Int16sl, 
	    "less"          / construct.Int16sl 
    )),    
    "button_cfg" / construct.Array(7, construct.Struct(
        "debouncetck"   / construct.Int16ul,
	    "clicktck"      / construct.Int16ul,
	    "presstck"      / construct.Int16ul
    ))
)

#config.parse(b)
async def read_write_mbl_config(args):        #port: str, slave: int, baud: int, 
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
        rr = await client.read_holding_registers(address=4, count=90-4, slave=args.slave)
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

    data = bytearray()
    for val in rr.registers:
        data = data + struct.pack('<H',val)
    
#    print(config.sizeof())
#    print(len(data))

    cfg = config.parse(data)

    print(cfg)

    if args.pri_app_crc != None:
         cfg.pri_app_crc = args.pri_app_crc

    if args.sec_app_crc != None:
         cfg.sec_app_crc = args.sec_app_crc

    if args.slave_addr != None:
         cfg.slave_addr = args.slave_addr

    if args.parity != None:
         match args.parity:
            case 'N':
                 cfg.parity = 0
            case 'O':
                 cfg.parity = 1
            case 'E':
                 cfg.parity = 2

    if args.baudrate != None:
         match args.baudrate:
             case 9600:
                cfg.baudrate = 96
             case 19200:
                cfg.baudrate = 192
             case 38600:
                cfg.baudrate = 386
             case 57600:
                cfg.baudrate = 576
             case 115200:
                cfg.baudrate = 1152

    if args.hwc != None:
         match args.hwc:
            case 'auto':
                 cfg.hwc = 0
            case 'switch':
                 cfg.hwc = 1
            case 'therm':
                 cfg.hwc = 2
            case 'humid':
                 cfg.hwc = 3
         
    if args.pli != None:
        for pli in args.pli:
            cfg.logic_conf.pli[int(pli[0])].invert = int(pli[1])
            cfg.logic_conf.pli[int(pli[0])].enable = int(pli[2])

    if args.plo != None:
        cfg.logic_conf.plo.invert = int(args.plo[0])
        cfg.logic_conf.plo.output = int(args.plo[1])

    if args.relay != None:
        cfg.relay.index = int(args.relay[0])
        cfg.relay.mask = int(args.relay[1])

    if args.logic != None:
        for logic in args.logic:
            cfg.logic[int(logic[0])].index = int(logic[1])
            cfg.logic[int(logic[0])].mask  = int(logic[2])

    if args.comparators != None:
        for comparators in args.comparators:
            cfg.comparators[int(comparators[0])].index   = int(comparators[1])
            cfg.comparators[int(comparators[0])].greater = int(comparators[2])
            cfg.comparators[int(comparators[0])].equal   = int(comparators[3])
            cfg.comparators[int(comparators[0])].less    = int(comparators[4])

    if args.button_cfg != None:
        for button in args.button_cfg:
            cfg.button_cfg[int(button[0])].debouncetck = int(button[1])
            cfg.button_cfg[int(button[0])].clicktck    = int(button[2])
            cfg.button_cfg[int(button[0])].presstck    = int(button[3])

#    print(cfg)
    if args.write == True:
        data = config.build(cfg)
        vals = [int.from_bytes(data[i:i+2],byteorder="little",signed=False) for i in range(0, len(data), 2)]
        await client.write_registers(address=4,slave=args.slave,values=vals)
        vals = struct.unpack('<2H',struct.pack('<4s',b"wc-9"))
        await client.write_registers(address=0, slave=args.slave,values=vals)


    print("close connection")
    client.close()

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Retrieve and modify config')

    parser.add_argument('--port',default="/dev/ttyUSB3", type=str)
    parser.add_argument('--slave',default=1, type=int)
    parser.add_argument('--baud',default=57600, type=int)
    parser.add_argument('--par', choices=['N','O','E'], default='N')
    parser.add_argument('--write', action='store_true',help='Enable the writeout')
    parser.add_argument('--pri_app_crc', type=int)
    parser.add_argument('--sec_app_crc', type=int)
    parser.add_argument('--slave_addr', type=int)
    parser.add_argument('--parity', choices=['N','O','E'], default='N')
    parser.add_argument('--baudrate', type=int)
    parser.add_argument('--hwc', choices=['auto','switch','therm','humid'])
    parser.add_argument('--pli', nargs=3, action='append', metavar=('input','invert','enable'))
    parser.add_argument('--plo', nargs=2, metavar=('invert','output'))
    parser.add_argument('--relay', nargs=2, metavar=('index','mask'))
    parser.add_argument('--logic',nargs=3, action='append',metavar=('input','index','mask'))
    parser.add_argument('--comparators', nargs=5, action='append',
                        metavar=('input','index','greater','equal','less'))
    parser.add_argument('--button_cfg', nargs=4, action='append',
                        metavar=('button','debounce','click','press'))
 
    args = parser.parse_args()
    print(args)
    
    asyncio.run(
        read_write_mbl_config(args), debug=False
    )
