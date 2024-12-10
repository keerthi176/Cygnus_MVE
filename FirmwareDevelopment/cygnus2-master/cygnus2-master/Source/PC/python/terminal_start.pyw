#
# Script to start test terminals for a test rig
# It includes the following options:
#    enable logging
#    define a configuration file
#
# Written in Python 3.6
#

import subprocess
import argparse
import datetime
import os
import sys
import platform

ABOVE_NORMAL_PRIORITY_CLASS = 0x00008000
BELOW_NORMAL_PRIORITY_CLASS = 0x00004000
HIGH_PRIORITY_CLASS         = 0x00000080
IDLE_PRIORITY_CLASS         = 0x00000040
NORMAL_PRIORITY_CLASS       = 0x00000020
REALTIME_PRIORITY_CLASS     = 0x00000100


if '__main__' == __name__:

    sys.path.append(os.path.join('.', 'terminal_configurations'))
    configuration_file = __import__(platform.node())

    #extract the data from the file
    terminalConfigurations = getattr (configuration_file, 'terminalConfigurations')
    TERATERM_PATH = getattr (configuration_file, 'TERATERM_PATH')
    BAUDRATE = getattr (configuration_file, 'BAUDRATE')
    teraterm_exe = os.path.join(TERATERM_PATH,'TTERMPRO.exe')

    # instantiate the parser
    parser = argparse.ArgumentParser(description='Terminal start routine')

    # Define arguments
    parser.add_argument('--logging', '-l', action='store_true',
                        help='Flag to enable logging')

    parser.add_argument('--config', '-c', action='store', dest='config_name',
                        help='Config file')

    # parse options
    args = parser.parse_args()

    if args.config_name:
        execfile(args.config_name)

    if args.logging == True:
        # create directory name from date / time
        now = datetime.datetime.now()
        logdir = now.strftime("%Y-%m-%d_%H-%M-%S")

        # create directory for logs
        if not os.path.exists(logdir):
            os.makedirs(logdir)
            logpath = os.getcwd() + '\\' + logdir

    # step through terminals
    for config in terminalConfigurations:
        # add parameters
        port = config[1]
        name = '/W=' + config[0]
        x_pos = config[2]
        y_pos =  config[3]
        if args.logging == True:
          logfile = '/L=' + logpath + '\\' + config[0] + '.log'
          command = [teraterm_exe,port,'/BAUD=' + BAUDRATE, name, x_pos, y_pos, logfile]
        else:
          command = [teraterm_exe,port,'/BAUD=' + BAUDRATE, name, x_pos, y_pos]

        # start terminal
        print ('starting ' + config[0])
        subprocess.Popen(command, shell=False, creationflags=HIGH_PRIORITY_CLASS)

