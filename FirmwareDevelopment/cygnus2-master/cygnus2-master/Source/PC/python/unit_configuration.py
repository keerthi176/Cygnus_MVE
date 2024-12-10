import os
import sys
import platform
import subprocess
import time
from multiprocessing.pool import ThreadPool
import serial
from tkinter import ttk
from tkinter import *
from tkinter import filedialog
from tkinter.filedialog import askopenfilename
from importlib.machinery import SourceFileLoader
import msvcrt

at_commands = [
    'ATUA',
    'ATSYNC',
    'ATFREQ',
    'ATDEVCF',
    'ATSYSID',
    'ATTXPLO',
    'ATTXPHI',
    'ATSERNO'
]


def write_unit_config(*my_config):
    my_config = my_config[0]
    my_name = my_config[0]
    sync_flag = 1 if my_name is 'NCU' else 0
    port = "COM" + my_config[1].strip("/C=")
    my_address = my_config[4]
    my_dev_cfg = my_config[5]
    tx_power_low = my_config[6]
    tx_power_high = my_config[7]
    my_serial_nbr = my_config[8]
    my_baud = my_config[9]
    my_freq = my_config[10]
    my_sys_id = my_config[11]
    serial_object = serial.Serial(port, my_baud)

    at_values = [my_address,
                 str(sync_flag),
                 my_freq,
                 my_dev_cfg,
                 my_sys_id,
                 tx_power_low,
                 tx_power_high,
                 my_serial_nbr]

    for at_command in enumerate(at_commands):
        send_at_command(serial_object, at_command[1] + '=' + at_values[at_command[0]])
        time.sleep(1)
    return 0


def send_at_command(serial_object, at_command):
    command = at_command.encode("ASCII")

    if command is not None and serial_object is not None and serial_object.isOpen() is True:
        serial_object.write("+++".encode("ASCII") + '\r'.encode("ASCII") + '\n'.encode("ASCII"))
        time.sleep(0.2)
        serial_object.write("+++".encode("ASCII") + '\r'.encode("ASCII") + '\n'.encode("ASCII"))

        serial_object.write(command + '\r'.encode("ASCII") + '\n'.encode("ASCII"))
        print(command + '\r'.encode("ASCII") + '\n'.encode("ASCII"))


if __name__ == "__main__":
    root = Tk()
    root.withdraw()

    ext = ""
    while ext != ".py":
        filename = filedialog.askopenfilename(initialdir=os.path.join(os.getcwd(),'terminal_configurations'),
                                              title="select the configuration file",
                                              filetypes=(("Python files", "*.py"), ("all files", "*.*")))

        if not filename:
            # The user most likely canceled the operation
            exit()

        file, ext = os.path.splitext(filename)
    config_module = SourceFileLoader(file, filename).load_module()
    terminalConfigurations = config_module.terminalConfigurations
    BAUDRATE = config_module.BAUDRATE
    FREQUENCY = config_module.FREQUENCY
    SYSTEM_ID = config_module.SYSTEM_ID

    threads = []
    async_results = []
    # step through terminals
    pool = ThreadPool(processes=len(terminalConfigurations))
    print('Please wait ...')
    for config in terminalConfigurations:
        myconfig = list(config)
        myconfig.extend([BAUDRATE, FREQUENCY, SYSTEM_ID])
        async_result = pool.apply_async(write_unit_config, (myconfig,))
        async_results.append(async_result)

    errors = 0
    result = None
    for index in range(0,len(terminalConfigurations)):
        name = terminalConfigurations[index][0]
        result = async_results[index].get()
        if result is 0:
            print(name + ' configuration Update: SUCCESS')
        else:
            print(name + ' configuration Update: FAIL')
            errors +=1

    if errors is 0:
        print('All successful')
    else:
        print('Failed config: ' + str(errors))

root.destroy()

print("\n\n\nPress a key to close this window...\n")
msvcrt.getch()