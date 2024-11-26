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


class WSJTX_Packet():
    """
    Header class for all WSJT received packets. This class handles the header
    and will create class for handling the packet depending on the packet type
    in the header.
    """
    def __init__(self, pkt, idx):
        self.index = idx  # Keeps track of where we are in the packet parsing
        self.packet = pkt
        self.MagicNumber = 0
        self.SchemaVersion = 0
        self.PacketType = 0
        self.ClientID = ""

    # Methods to extract the different types of data in the packet.  These are shared with all
    # the other packet classes.
    def readutf8(self):
        strLength = self.getInt32()
        # BUG what happens if string length is zero?
        if strLength > 0:
            stringRead = struct.unpack(">"+str(strLength)+"s",
                    self.packet[self.index:strLength+self.index])
            self.index += strLength
            return stringRead[0].decode('utf-8')
        return ""

    def getDateTime(self):
        TimeOffset = 0
        DateOff = self.getLongLong()
        TimeOff = self.getuInt32()
        TimeSpec = self.getByte()
        if TimeSpec == 2:
            TimeOffset = self.getInt32()
        return (DateOff, TimeOff, TimeSpec, TimeOffset)

    def getByte(self):
        data = struct.unpack(">B", self.packet[self.index:self.index+1])
        self.index += 1
        return data[0]

    def getBool(self):
        data = struct.unpack(">?", self.packet[self.index:self.index+1])
        self.index += 1
        return data[0]

    def getInt32(self):
        data = struct.unpack(">i", self.packet[self.index:self.index+4])
        self.index += 4
        return data[0]

    def getuInt32(self):
        data = struct.unpack(">I", self.packet[self.index:self.index+4])
        self.index += 4
        return data[0]

    def getLongLong(self):
        data = struct.unpack(">Q", self.packet[self.index:self.index+8])
        self.index += 8
        return data[0]

    def getDouble(self):
        data = struct.unpack(">d", self.packet[self.index:self.index+8])
        self.index += 8
        return data[0]

    def Decode(self):
        # print(self.packet, len(self.packet))
        self.MagicNumber = self.getuInt32()
        self.SchemaVersion = self.getuInt32()
        self.PacketType = self.getuInt32()
        self.ClientID = self.readutf8()

class WSJTX_Status(WSJTX_Packet):
    """
    Packet Type 1 Status:
    WSJT-X  sends this  status message  when various  internal state changes to
    allow the server to  track the relevant state of each client without the
    need for  polling commands. The current state changes that generate status
    messages are:

    - Application start up,
    - "Enable Tx" button status changes,
    - Dial frequency changes,
    - Changes to the "DX Call" field,
    - Operating mode, sub-mode or fast mode changes,
    - Transmit mode changed (in dual JT9+JT65 mode),
    - Changes to the "Rpt" spinner,
    - After an old decodes replay sequence (see Replay below),
    - When switching between Tx and Rx mode,
    - At the start and end of decoding,
    - When the Rx DF changes,
    - When the Tx DF changes,
    - When the DE call or grid changes (currently when settings are exited),
    - When the DX call or grid changes,
    - When the Tx watchdog is set or reset.
    """
    def __init__(self, pkt, idx):
        WSJTX_Packet.__init__(self, pkt, idx)
        self.Frequency = 0
        self.Mode = ""
        self.DXCall = ""
        self.Report = ""
        self.TxMode = ""
        self.TxEnabled = False
        self.Transmitting = False
        self.Decoding = False
        self.RxDF = 0
        self.TxDF = 0
        self.DECall = ""
        self.DEgrid = ""
        self.DXgrid = ""
        self.TxWatchdog = False
        self.Submode = ""
        self.Fastmode = False
        self.SpecialOpMode = False
        self.FrequencyTolerance = 0
        self.TrPeriod = 0
        self.ConfigurationName = ""
        self.TxMessage = ""

    def Decode(self):
        """
        https://github.com/jtdx-project/jtdx/blob/master/MessageServer.cpp#L221

        Note: JTDX and jtdx_improved do NOT send 'TxMessage', hence they are NOT compatible!

        https://sourceforge.net/p/wsjt/wsjtx/ci/master/tree/UDPExamples/MessageServer.cpp#l253
        """
        self.Frequency = self.getLongLong()
        self.Mode = self.readutf8()
        self.DXCall = self.readutf8()
        self.Report = self.readutf8()
        self.TxMode = self.readutf8()
        self.TxEnabled = self.getBool()
        self.Transmitting = self.getBool()
        self.Decoding = self.getBool()
        self.RxDF = self.getuInt32()
        self.TxDF = self.getuInt32()
        self.DECall = self.readutf8()
        self.DEgrid = self.readutf8()
        self.DXgrid = self.readutf8()
        self.TxWatchdog = self.getBool()
        self.Submode = self.readutf8()
        self.Fastmode = self.getBool()
        self.SpecialOpMode = self.getBool()
        self.FrequencyTolerance = self.getuInt32()
        self.TrPeriod = self.getuInt32()
        self.ConfigurationName = self.readutf8()
        self.TxMessage = self.readutf8()

sport = None
while not sport:
    try:
        sport = serial.Serial(serial_port, BAUD_RATE, timeout=0.5)
    except serial.serialutil.SerialException:
        print("> Unable to open serial port (%s)..." % serial_port)
    time.sleep(1)

# Global variables
freq_offset = 1200
frequency = 28074000
mode = "FT8"
current_msg = ""

# Connection for WSJT-X
UDP_IP = "127.0.0.1"
UDP_PORT = 2237
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

def load_msg(msg):
    print("> Load message into transmitter...")
    sport.write(b'*m')
    sport.write(msg.encode("ascii"))
    sport.write(b'*')

def change_offset(new_offset):
    global freq_offset
    freq_offset = new_offset
    print("> Change TX offset to", new_offset)
    sport.write(b'*o')
    sport.write(str(new_offset).encode("ascii"))
    sport.write(b'*')
    sport.flush()

def change_mode(new_mode):
    print("> Change mode to", new_mode)
    sport.write(b'*s')
    sport.write(str(new_offset).encode("ascii"))
    sport.write(b'*')
    sport.flush()

def change_frequency(new_frequency):
    global frequency
    if new_frequency >= 30000000: # 30 MHz
        print("[!] Ignoring bogus frequency change from WSJT-X...")
        return
    # check for valid and common ft8 frequency
    if new_frequency not in [1840000, 3573000, 7074000, 10136000, 14074000, 18100000, 21074000, 24915000, 28074000]:
        return
    frequency = new_frequency
    print("> Change frequency to:", new_frequency)
    sport.write(b'*f')
    sport.write(str(new_frequency).encode("ascii"))
    sport.write(b'*')
    # print(str(new_frequency).encode("ascii"), "XXX")

def transmit():
    if not current_msg:
        time.sleep(1)
        return
    print("> TX!")
    sport.write(b'*t*')

def check_time_window(utc_time):
    time_window = 15 if 'FT8' in mode else 7
    rm = utc_time.second % time_window
    if rm > 1 and rm < time_window-1:
        return False
    return True

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

    # go to the default frequency?

    try:
        while True:
            packet_data, _ = sock.recvfrom(1024)
            new_packet = WSJTX_Packet(packet_data, 0)
            new_packet.Decode()
            if new_packet.PacketType == 1:
                print("> Got status packet...")
                StatusPacket = WSJTX_Status(packet_data, new_packet.index)
                StatusPacket.Decode()

                # Check TX frequency and update transceiver
                new_offset = StatusPacket.TxDF
                new_mode = StatusPacket.TxMode.strip()
                new_frequency = StatusPacket.Frequency

                if new_frequency != frequency:
                    change_frequency(new_frequency)

                if new_offset != freq_offset:
                    change_offset(new_offset)

                if new_mode != mode:
                    change_mode(new_mode)

                # Check if TX is enabled
                if StatusPacket.Transmitting == 1:
                    # Check time, avoid transmitting out of the time slot
                    utc_time = datetime.datetime.utcnow()
                    tx_now = check_time_window(utc_time)
                    if tx_now:
                        sport.write(b'*p*')
                    message = StatusPacket.TxMessage
                    message = message.replace('<', '')
                    message = message.replace('>', '')
                    new_msg(message.strip())
                    if tx_now:
                        time.sleep(0.300)  # this improves 'dt' timings
                        transmit()
                    print("> Time: {0}:{1}:{2}".format(utc_time.hour, utc_time.minute, utc_time.second))
    finally:
        sock.close()

if __name__ == "__main__":
    main()
