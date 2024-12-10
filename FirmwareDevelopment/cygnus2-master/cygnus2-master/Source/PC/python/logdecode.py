#
# Script to decode messages in log format. This takes an input from a
# serial port or from a file
#
# This script requires that the serial module is loaded into the python install before running
#
# Written in Python 3.6 
#

import argparse
import re
import serial
import ctypes
from ctypes import cdll, windll, c_long, c_int, c_char_p, create_string_buffer

# define DLL and function
lib = ctypes.CDLL('Mesh.dll')
func = lib['decode']
#func.argtypes = [ctypes.c_char_p]
func.restype = ctypes.c_int

boutstring = create_string_buffer(2048)

# define constants
# log fields
DATETIME_FIELD = 1
SLOTID_FIELD = 2
DIRECTION_FIELD = 3
ADDR_FIELD = 4
FREQ_FIELD = 5
RSSI_FIELD = 6
SNR_FIELD = 7
DATA_FIELD = 8


def mesh_decode(instring):
    global boutstring
    binstring = instring.encode('ascii')
    result = func(binstring, boutstring)
    return result


def serial_ports():
    """ Lists serial port names

        :returns:
            A list of the serial ports available on the system
    """
    ports = ['COM%s' % (i + 1) for i in range(256)]

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


def decodeline(line):
   p = re.compile(b"(\[.*\] )?\[([0-9]*),([rt]),([0-9]*),([0-9]*),([-0-9]*),([-0-9]*),([0-9A-F]*).*\]")
   m = p.search(line)
   if m:
        direction = m.group(DIRECTION_FIELD).decode()
        data = m.group(DATA_FIELD).decode()
        result = mesh_decode(data)

        if result > 0:
            print(direction, boutstring.value.decode())


if '__main__' == __name__:

    # instantiate the parser
    parser = argparse.ArgumentParser(description='Log decode routine')
    
    # Define arguments
    parser.add_argument('-l', '--listcomports', action='store_true',
                        help='list com ports')

    parser.add_argument('-f', '--file', action='store', dest='infile_name',
                        help='input file')
    
    parser.add_argument('-c', '--comport', action='store', dest='comport_name',
                        help='COM port')
    
    # parse options
    args = parser.parse_args()

    if args.listcomports == True:
        print(serial_ports())

    elif args.comport_name:
        ser = serial.Serial(args.comport_name, 115200, timeout=0.1, xonxoff=False, rtscts=False, dsrdtr=False)
        ser.flushInput()
        ser.flushOutput()
        while True:
          serline = ser.readline()
          if len(serline) > 0:
            decodeline(serline)

    elif args.infile_name:
        lines = open(args.infile_name,'r').readlines()
        for line in lines:
            decodeline(line.encode('ascii'))


   
