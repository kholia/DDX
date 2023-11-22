#!/usr/bin/env python3
#pylint: disable-msg=missing-function-docstring,line-too-long,invalid-name,too-many-instance-attributes,consider-using-f-string

"""
WSJT-X plugin (helper script) for radios using the Direct-RF-Synthesis technique.

Sources:

- https://github.com/rstagers/WSJT-X/blob/master/WSJTXClass.py
- https://github.com/agustinmartino/wsjtx_transceiver_interface
"""

import sys
import time
import socket
import struct
import datetime

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("The 'pyserial' library is missing. Please install it with 'python3 -m pip install pyserial' command.")
    sys.exit(1)

a = serial.tools.list_ports.comports()
serial_port = ""
for w in a:
    if "cafe" in w.hwid.lower() or "4011" in w.hwid.lower():
        serial_port = w.device

if not serial_port:
    print("We could NOT find the 'DDX' radio. Please re-connect the radio and try again.")
    sys.exit(1)

BAUD_RATE = 115200

import serial
import serial.tools.list_ports

sport = None
while not sport:
    try:
        sport = serial.Serial(serial_port, BAUD_RATE, timeout=0.5)
    except serial.serialutil.SerialException:
        print("> Unable to open serial port (%s)..." % serial_port)
    time.sleep(1)

# Global variables
current_msg = ""

def load_msg(msg):
    print("> Load message into transmitter...")
    sport.write(b'*m')
    sport.write(msg.encode("ascii"))
    sport.write(b'*')
    resp = sport.read(1)
    if resp == b'm':
        print("> Load OK!")
    else:
        print(resp)

def transmit():
    if not current_msg:
        time.sleep(1)
        return
    print("> TX!")
    sport.write(b'*t*')

def new_msg(msg):
    global current_msg
    if msg != current_msg:
        print(("> Message: {0}".format(msg)))
        load_msg(msg)
        current_msg = msg

def main():
    print("> Waiting for transceiver to be ready...")
    while True:
        time.sleep(0.5)
        sport.write(b'*v*')
        r = sport.read()
        if r == b'r':
            print("> Transceiver is ready!")
            break

    new_msg("CQ VU3CER MK68")
    transmit()


main()
