#
# Script to update the Firmware for a test rig
# Written in Python 3.6
#
import subprocess
import os
import string
import terminal_stop
import terminal_configurations
import msvcrt
import platform
from tkinter import *
from tkinter import filedialog
from tkinter.filedialog import askopenfilename


if '__main__' == __name__:
    script = os.path.join(os.getcwd(),'..','teraterm', 'serial_reprogramming.ttl')
    #get the configuration file based on the Machine name
    sys.path.append(os.path.join('.', 'terminal_configurations'))
    configuration_file = __import__(platform.node())

    #extract the data from the file
    terminalConfigurations = getattr (configuration_file, 'terminalConfigurations')
    TERATERM_PATH = getattr (configuration_file, 'TERATERM_PATH')
    BAUDRATE = getattr (configuration_file, 'BAUDRATE')   
    teraterm_macro = os.path.join(TERATERM_PATH,'ttpmacro.exe')
    
    root = Tk()
    errors = 0
    jobs = []

    # Close any Tera Term application
    p = subprocess.Popen(["pythonw", "terminal_stop.pyw"])
    p.wait()

    # Ask the user to select the Binary application
    root.withdraw()
    root.filename =  filedialog.askopenfilename(initialdir = os.path.join(os.getcwd(),'..', '..', 'MCU', 'Application', 'MCU_ForBootloader'),title = "select the binary image",filetypes = (("Binary files","*.bin"),("all files","*.*")))
    print('Please Wait: Firmware Update of ' + str(len(terminalConfigurations)) + ' units in progress')

    
    # step through terminals
    for index in range(0,len(terminalConfigurations)):
        # Extract Unit name
        name = terminalConfigurations[index][0]

        #Build the TeraTerm Macro command
        port = terminalConfigurations[index][1]
        command = [teraterm_macro,script,port,BAUDRATE,root.filename]
        #print (command)
        #execute the command
        p = subprocess.Popen(command)
        jobs.append(p)
        

    #Wait for all the jobs to finish   
    for index in range(0,len(terminalConfigurations)):
        name = terminalConfigurations[index][0]
        jobs[index].wait()
        if jobs[index].returncode == 0:
            print(name + ' Firmware Update: SUCCESS')
        else:
            print(name + ' Firmware Update: FAIL')
            errors +=1

    #Terminate all the teraTerm processes
    p = subprocess.Popen(["pythonw", "terminal_stop.pyw"])
    p.wait()

    #Print a summary for the user
    print('\n###############################################')
    print('SUMMARY:')
    print('\tSuccessful Updates: ' + str(len(terminalConfigurations) - errors))
    print('\tFailed Updates    : ' + str(errors))
    print('\n###############################################')

root.destroy()

print("Press a key to close this window...\n")
msvcrt.getch()
