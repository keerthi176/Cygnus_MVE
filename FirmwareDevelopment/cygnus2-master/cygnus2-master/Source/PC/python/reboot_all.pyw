#
# Script to start test terminals for a test rig
# Written in Python 3.6
#

import subprocess
import argparse
import datetime
import os
import sys
import platform
import terminal_stop
import terminal_configurations

if '__main__' == __name__:
   MACRO_FILE = os.path.join(os.getcwd(),'..','teraterm', 'reboot_unit.ttl')
   sys.path.append(os.path.join('.', 'terminal_configurations'))
   configuration_file = __import__(platform.node())

   #extract the data from the file
   terminalConfigurations = getattr (configuration_file, 'terminalConfigurations')
   TERATERM_PATH = getattr (configuration_file, 'TERATERM_PATH')
   BAUDRATE = getattr (configuration_file, 'BAUDRATE')
   teraterm_exe = os.path.join(TERATERM_PATH,'TTERMPRO.exe')

   # Close any Tera Term application
   p = subprocess.Popen(["pythonw", "terminal_stop.pyw"])
   p.wait() 
        
   # step through terminals
   for config in terminalConfigurations:
       # add parameters
       port = config[1]
       name = '/W=' + config[0]
       x_pos = config[2]
       y_pos =  config[3]
       command = [teraterm_exe,port,'/BAUD=' + BAUDRATE, name, '/M='+ MACRO_FILE,  x_pos, y_pos]
       subprocess.Popen(command, shell=True)

