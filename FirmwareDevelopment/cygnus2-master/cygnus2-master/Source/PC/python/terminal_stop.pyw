#
# Script to stop test terminals for a test rig
# Written in Python 3.6
#

import subprocess

#Check if the TeraTerm Macro exe is running
CheckCommand = 'tasklist /fi \"imagename eq ttpmacro.exe\" |find \":\" > nul'
error = subprocess.call(CheckCommand, shell=True)

if error==1:
    # define command
    StopCommand = ['taskkill', '/f', '/im', 'ttpmacro.exe', '/f', '>nul', '2>&1']

    # run the command
    p = subprocess.Popen(StopCommand, shell=True)
    p.wait()

#Check if the TeraTerm console is running
CheckCommand = 'tasklist /fi \"imagename eq ttermpro.exe\" |find \":\" > nul'
error = subprocess.call(CheckCommand, shell=True)

if error==1:
    # define command
    StopCommand = ['taskkill', '/im', 'ttermpro.exe', '/f', '>nul', '2>&1']

    # run the command
    p = subprocess.Popen(StopCommand, shell=True)
    p.wait()
