7#
# Script to decode messages in log format. This takes an input from a
# serial port or from a file
#
# This script requires that the serial module is loaded into the python install before running
#
# Written in Python 3.6
#

import argparse
import re
import sys
from ctypes import create_string_buffer
import PY_MC_PUP
from PY_MC_PUP import *
from CO_Message import *
import msvcrt as m

binstring = create_string_buffer(2048)
boutstring = create_string_buffer(2048)



def usage():
    print("logconvert.py <filename>")


def mesh_decode(instring):
    global boutstring
    assert isinstance(instring, object)

    boutstring = ""

    decoded_line = PY_MC_PUP.packet_decode(packet=instring.decode('utf-8'))
    if decoded_line is not None:
        if 'FrameType' in decoded_line:
            frame_type = FrameType(decoded_line['FrameType'])
            decoded_line['FrameType'] = frame_type.name + '(%d)' % frame_type.value
            if 'appMsgType' in decoded_line:
                app_msg_type = ApplicationLayerMessageType(decoded_line['appMsgType'])
                decoded_line['appMsgType'] = app_msg_type.name + '(%d)' % app_msg_type.value
                if app_msg_type is app_msg_type.FAULT_SIGNAL:
                    channel_index = ChannelIndex(decoded_line['Channel'])
                    decoded_line['Channel'] = channel_index.name + '(%d)' % channel_index.value

                    fault_type = FaultType(decoded_line['FaultType'])
                    decoded_line['FaultType'] = fault_type.name + '(%d)' % fault_type.value
                elif app_msg_type is app_msg_type.OUTPUT_SIGNAL:
                    output_profile = OutputProfile(decoded_line['OutputProfile'])
                    decoded_line['OutputProfile'] = output_profile.name + '(%d)' % output_profile.value
                elif app_msg_type is app_msg_type.STATUS_INDICATION:
                    mesh_event = MeshEvent(decoded_line['Event'])
                    decoded_line['Event'] = mesh_event.name + '(%d)' % mesh_event.value

    
        boutstring = str(decoded_line)
        result = len(decoded_line)
    else:
        boutstring = str(instring)
        result = 0
    return result



def decodeline(line):
    global binstring
    binstring = ""
    p = re.compile(b',([0-9A-F]{18,})')
    m = p.findall(line)
    if m:
        result = mesh_decode(m[0])

        if result > 0:
            binstring = line.decode('utf-8')
        else:
            binstring = "Failed to decode line : "


if '__main__' == __name__:

    # check for arguments
    if len(sys.argv) < 2:
        usage()
        sys.exit(2)

    print('logconvert')

    # instantiate the parser
    parser = argparse.ArgumentParser(description='Log convert routine')

    # read the filename
    file_name = sys.argv[1]
    out_file = file_name
    out_file_name = out_file.replace(".", "_conv.")

    outf = open(out_file_name, "x", encoding='utf-8')
    if outf is not None:
        lines = open(file_name, 'r', encoding='utf-8').readlines()
        for line in lines:
            decodeline(line.encode('utf-8'))
            outstr = str(boutstring)
            if line.find("+INF:s=") == -1:
                if line.find("+INF:dl wk=") == -1:
                    if outstr.startswith("<ctypes"):
                        outf.write(str(line))
                    elif len(binstring) == 0:
                        outf.write(str(line))
                    else:
                        outf.write(str(binstring))
                        outf.write(str(boutstring))
                        outf.write("\r\n")
    else:
        print("Failed to create output file")
        m.getch()
        sys.exit(2)

