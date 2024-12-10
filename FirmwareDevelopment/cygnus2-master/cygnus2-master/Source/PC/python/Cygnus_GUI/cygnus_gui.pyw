"""
GUI Application for the Cygnus II RBU control and debug purposes
"""

import os
import errno
import re
import time
import threading
import traceback
import json
import sys
import ast
import bincopy
from datetime import datetime
import tkinter
from tkinter import ttk
from tkinter import *
from tkinter.scrolledtext import ScrolledText
from tkinter import filedialog
from tkinter import messagebox
from PIL import Image, ImageTk
import networkx as nx
import matplotlib
import operator
import shared
from radio_node import *

# To have the logo on the Taskbar
myappid = 'BullProduct.CygnusII.RBU.1.0'

try:
    ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(myappid)
except:
    print("Could not set the app model ID")


def exception_callback(*args):
    err = traceback.format_exception(*args)
    print(err)
    traceback.print_exc()


class MeshStatusPanel(Toplevel):
    """"""

    # ----------------------------------------------------------------------
    def __init__(self):
        """Constructor"""
        Toplevel.__init__(self)
        self.geometry("300x200")
        self.title("Mesh Status")
        self.resizable(False, False)

        self.draw_mesh_button = Button(self, text="Draw Mesh", command=self.draw_mesh, width=12)
        self.list_parents_button = Button(self, text="List Parents", command=self.list_parents, width=12)

        # Mesh state control
        self.mesh_state_var = StringVar(self, value=mesh_states_list[0])
        self.mesh_state_menu = OptionMenu(self, self.mesh_state_var, *mesh_states_list)
        self.mesh_state_menu.config(width=20, justify="center")
        self.mesh_state_button = Button(self, text="SET STATE", command=self.set_mesh_state, width=12)

        self.draw_mesh_button.place(x=10, y=10)
        self.list_parents_button.place(x=110, y=10)
        self.mesh_state_button.place(x=10, y=52)
        self.mesh_state_menu.place(x=110, y=50)

    def my_thread(self):
        pass

    @staticmethod
    def draw_mesh():
        global report_mesh_state_request
        report_mesh_state_request = True



    def list_parents(self):
        pass

    def set_mesh_state(self):
        ncu = None
        for node in nodes:
            if node.unit_name == 'NCU':
                ncu = node
        if ncu is not None:
            state = mesh_states_list.index(str(self.mesh_state_var.get()))
            ncu.set_mesh_state(state)
        else:
            messagebox.showerror("NCU", "please define the NCU node!")


class AddNodePanel(Toplevel):
    """"""

    # ----------------------------------------------------------------------
    def __init__(self):
        """Constructor"""
        Toplevel.__init__(self)
        self.geometry("300x200")
        self.title("Add Node")
        Label(self, text="Node name", font="Helvetica 10").place(x=5, y=10)
        self.name_entry = Entry(self, width=7, font="Helvetica 10", justify="left")
        self.name_entry.place(x=85, y=10)

        Label(self, text="port Name", font="Helvetica 10").place(x=5, y=40)
        self.port_entry = Entry(self, width=7, font="Helvetica 10", justify="left")
        self.port_entry.place(x=85, y=40)

        Label(self, text="Baudrate", font="Helvetica 10").place(x=5, y=70)
        self.baud_entry = Entry(self, width=7, font="Helvetica 10", justify="left")
        self.baud_entry.place(x=85, y=70)
        self.baud_entry.insert(0, '2000000')

        self.add_button = Button(self, text="Add", command=self.add_button, width=6)
        self.cancel_button = Button(self, text="Cancel", command=self.cancel_button, width=6)

        self.add_button.place(x=100, y=150)
        self.cancel_button.place(x=160, y=150)

    def my_thread(self):
        pass

    def add_button(self):
        global nodes
        this_node = {
            "unit name": self.name_entry.get(),
            "COM Port": self.port_entry.get(),
            "GUI x position": None,
            "GUI y position": None,
            "UART Baudrate": self.baud_entry.get(),
            "GUI Width": geometry_full_width,
            "GUI Height": geometry_default_height,
            "Decode Status": False,
            "Main Frame Status": 'SHOW',
            "Time-stamp": True
        }
        this_node = RbuNode(main_gui, config=this_node, node_list=nodes)
        this_node.run()
        nodes.append(this_node)

    def cancel_button(self):
        self.destroy()


class FirmwareUpdatePanel(Toplevel):
    """"""

    # ----------------------------------------------------------------------
    def __init__(self):
        """Constructor"""
        Toplevel.__init__(self)

        self.geometry("550x200")
        self.resizable(False, False)
        self.title("Firmware Update")
        self.firmware_file = None
        self.thread = None
        self.main_frame = Frame(self, height=146, width=305, bd=1, relief='groove')
        self.main_frame.place(x=5, y=5)

        Label(self.main_frame, text="SW Version", font="Helvetica 10").place(x=5, y=10)
        self.sw_version_box = Text(self.main_frame, height=1, width=12, font="Helvetica 10", bg=self['background'], state=DISABLED)
        self.sw_version_box.place(x=110, y=10)

        Label(self.main_frame, text="SW Date", font="Helvetica 10").place(x=5, y=40)
        self.sw_date_box = Text(self.main_frame, height=1, width=12, font="Helvetica 10", bg=self['background'], state=DISABLED)
        self.sw_date_box.place(x=110, y=40)

        Label(self.main_frame, text="SW Checksum", font="Helvetica 10").place(x=5, y=70)
        self.sw_checksum_box = Text(self.main_frame, height=1, width=12, font="Helvetica 10", bg=self['background'], state=DISABLED)
        self.sw_checksum_box.place(x=110, y=70)

        self.rescue_enable = IntVar(value=0)
        rescue_check_button = Checkbutton(self.main_frame, text="Rescue", font="Helvetica 8",  variable=self.rescue_enable)
        rescue_check_button.place(x=162, y=90)
        Tooltip(rescue_check_button, text='Rescue a unit by catching it at the start-up phase and set it in the bootlooder mode through the Back Door mechanism')
        self.program_button = Button(self.main_frame, text="PROGRAM", width=15, command=self.program_callback)
        self.program_button.place(x=165, y=112)
        Button(self.main_frame, text="SELECT FW", width=15, command=self.select_file).place(x=15, y=112)

        Label(self, text="RESULTS", font=("Helvetica", 9, "bold")).place(x=320, y=5)
        self.results_list = Listbox(self, height=8, bg='gray90')
        self.results_list.place(x=320, y=25)

        self.node_sel = VerticalScrolledFrame(self, height=145, width=80, bd=1, relief='groove')
        self.node_sel.pack_propagate(0)
        self.node_sel.place(x=450, y=5)

        self.fw_file_box = Text(self, wrap=NONE, font="Helvetica 10", height=1, width=75, bg=self['background'], state=DISABLED)
        self.fw_file_box.pack(fill=X, padx=5, anchor=S, expand=True, side=BOTTOM)
        self.fw_file_box.place(x=5, y=157)
        self.fw_file_scrollbar = Scrollbar(self, orient=HORIZONTAL, command=self.fw_file_box.xview)
        self.fw_file_box.configure(xscrollcommand=self.fw_file_scrollbar.set)
        self.fw_file_scrollbar.pack(side=BOTTOM, fill=X)

        self.node_selection_var = dict()
        for index, node in enumerate(nodes):
            self.node_selection_var[node.unit_name] = IntVar(value=1)
            this_node_box = Checkbutton(self.node_sel.interior, text=node.unit_name, variable=self.node_selection_var[node.unit_name])
            this_node_box.pack(anchor=W)

    def program_callback(self):
        # reload the file if a new compile just happened
        self.load_fw_file()

        for node in nodes:
            if self.node_selection_var[node.unit_name].get():
                node.firmware_update(self.firmware_file, self.rescue_enable.get())
        self.thread = threading.Thread(target=self.my_thread)
        self.thread.daemon = True
        self.thread.start()
        self.results_list.delete(0, END)

    def cancel_button(self):
        self.destroy()

    def select_file(self):
        ext = ""
        while ext != ".bin":
            filename = filedialog.askopenfilename(initialdir=os.getcwd(),
                                                  title="select the Binary file",
                                                  filetypes=(("Binary files", "*.bin"), ("all files", "*.*")))

            if not filename:
                # The user most likely canceled the operation
                return

            file, ext = os.path.splitext(filename)
        self.firmware_file = filename

        self.load_fw_file()

    def load_fw_file(self):
        # Open file with 'b' to specify binary mode
        with open(self.firmware_file, 'rb') as file:
            # read the last 4 bytes to get the checksum
            file.seek(-4, os.SEEK_END)
            checksum = ''
            for char in file.read():
                checksum = '{:02X}'.format(char) + checksum
            checksum = '0x' + checksum

            # Read the SW version
            file.seek(0x188, os.SEEK_SET)
            sw_version = ''

            idx = 0
            for char in file.read():
                if char is 0:
                    break
                sw_version += str(chr(char))
                idx += 1

            # Read the SW date
            file.seek(0x194, os.SEEK_SET)
            sw_date = ''
            for char in file.read():
                if char is 0:
                    break
                sw_date += str(chr(char))
            file.close()

            replace_txt(self.fw_file_box, self.firmware_file)
            replace_txt(self.sw_version_box, sw_version)
            replace_txt(self.sw_date_box, sw_date)
            replace_txt(self.sw_checksum_box, checksum)

    def my_thread(self):

        all_finished = False
        self.program_button.configure(relief=SUNKEN)
        results = dict()
        while all_finished is False:
            all_finished = True
            error = False
            for node in nodes:
                if self.node_selection_var[node.unit_name].get():
                    # check if this node has finished
                    if node.firmware_update_state != FirmwareUpdateState.COMPLETED\
                            and node.firmware_update_state != FirmwareUpdateState.FAILED:
                        # wait for this node
                        all_finished = False
                    else:
                        if node.firmware_update_state == FirmwareUpdateState.COMPLETED:
                            # check the SW info
                            if node.firmware_version_nbr != self.sw_version_box.get('1.0', END).rstrip() or \
                                node.firmware_version_date != self.sw_date_box.get('1.0', END).rstrip() or \
                                    node.firmware_checksum != self.sw_checksum_box.get('1.0', END).rstrip():
                                        # The SW information doesn't match
                                        error = True
                                        results[node.unit_name] = FirmwareUpdateState.FAILED
                            else:
                                results[node.unit_name] = FirmwareUpdateState.SUCCESS
                        else:
                            error = True
                            results[node.unit_name] = FirmwareUpdateState.FAILED

            time.sleep(0.5)

        for node in nodes:
            self.results_list.insert(END, node.unit_name)
            if self.node_selection_var[node.unit_name].get():
                if results[node.unit_name] == FirmwareUpdateState.SUCCESS:
                    self.results_list.itemconfig(END, {'bg': 'green2'})
                else:
                    self.results_list.itemconfig(END, {'bg': 'red2'})
            else:
                self.results_list.itemconfig(END, {'bg': 'grey'})
        if error:
            messagebox.showerror("Firmware Update", "Firmware Update Failed ")
        else:
            messagebox.showinfo("Firmware Update", "Successful Firmware Update")
        for node in nodes:
            node.firmware_update_state = FirmwareUpdateState.LEAVE
        self.program_button.configure(relief=RAISED)


class NodeParameterFrame(Frame):
    def __init__(self, parent, node, *args, **kw):
        Frame.__init__(self, parent, *args, **kw)

        self.select_status = IntVar(value=1)
        self.select_button = Checkbutton(self, text=node.unit_name, variable=self.select_status)
        self.select_button.place(x=1, y=4)

        self.Address_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.Address_Box.place(x=70, y=5)

        self.Zone_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.Zone_Box.place(x=120, y=5)

        self.SyncMaster_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.SyncMaster_Box.place(x=170, y=5)

        self.FrequencyChannel_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.FrequencyChannel_Box.place(x=220, y=5)

        self.DeviceCombination_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.DeviceCombination_Box.place(x=270, y=5)

        self.SystemId_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.SystemId_Box.place(x=320, y=5)

        self.TxLowPower_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.TxLowPower_Box.place(x=370, y=5)

        self.TxHighPower_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.TxHighPower_Box.place(x=420, y=5)

        self.BootUsePPU_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.BootUsePPU_Box.place(x=470, y=5)

        self.PpModeEnable_Box = Entry(self, width=6, font="Helvetica 10", state=DISABLED)
        self.PpModeEnable_Box.place(x=520, y=5)

        self.SerialNumber_Box = Entry(self, width=14, font="Helvetica 10", state=DISABLED)
        self.SerialNumber_Box.place(x=570, y=5)


class ParametersPanel(Toplevel):
    """"""

    # ----------------------------------------------------------------------
    def __init__(self):
        """Constructor"""
        Toplevel.__init__(self)
        ppu_present = False if next((node for node in nodes if node.unit_name == 'PPU'), None) is None else True
        my_height = min(35 + (len(nodes)-1 if ppu_present else len(nodes))*40 + 20, 600)
        self.geometry("700x%d" % my_height)
        self.resizable(False, False)
        self.bind("<Configure>", self.on_resize)
        self.title("Parameters")
        self.parameter_file = None
        self.config_file = None
        self.thread = None
        self.node_params = []
        self.operation = None
        self.common_param_value = None
        self.sel_common_param = None
        self.operation_wait_event = threading.Event()
        self.height = self.winfo_reqheight()
        self.width = self.winfo_reqwidth()
        self.main_frame = VerticalScrolledFrame(self, width=650, height=my_height - 55)
        self.main_frame.pack()
        self.main_frame.place(x=5, y=30)

        Label(self, text='UA', font=("Helvetica", 10, "italic")).place(x=77, y=5)
        Label(self, text='ZONE', font=("Helvetica", 10, "italic")).place(x=127, y=5)
        Label(self, text='SYNC', font=("Helvetica", 10, "italic")).place(x=177, y=5)
        Label(self, text='FREQ', font=("Helvetica", 10, "italic")).place(x=227, y=5)
        Label(self, text='DEVCF', font=("Helvetica", 10, "italic")).place(x=277, y=5)
        Label(self, text='SYSID', font=("Helvetica", 10, "italic")).place(x=327, y=5)
        Label(self, text='TXPLO', font=("Helvetica", 10, "italic")).place(x=377, y=5)
        Label(self, text='TXPHI', font=("Helvetica", 10, "italic")).place(x=427, y=5)
        Label(self, text='BPPU', font=("Helvetica", 10, "italic")).place(x=477, y=5)
        Label(self, text='PPEN', font=("Helvetica", 10, "italic")).place(x=527, y=5)
        Label(self, text='SERNO', font=("Helvetica", 10, "italic")).place(x=577, y=5)

        for index, node in enumerate(nodes):
            # The PPU doesn't support the same parameter set as the RBU/NCU
            if node.unit_name != 'PPU':
                this_node = NodeParameterFrame(self.main_frame.interior, node, width=690, height=40, bd=1, relief='groove', borderwidth=3)
                this_node.pack()
                self.node_params.append(this_node)

        menubar = Menu(self)
        filemenu = Menu(menubar, tearoff=0)
        filemenu.add_command(label="Load", command=self.load_from_file)
        filemenu.add_command(label="Save", command=self.save_to_file)
        filemenu.add_separator()

        filemenu.add_command(label="Close", command=self.destroy)
        menubar.add_cascade(label="File", menu=filemenu)
        editmenu = Menu(menubar, tearoff=0)
        editmenu.add_command(label="Unlock", command=self.unlock_parameters)
        editmenu.add_command(label="Lock", command=self.lock_parameters)
        editmenu.add_command(label="Common", command=self.common_parameters)
        editmenu.add_command(label="Select all", command=self.select_all)
        editmenu.add_command(label="Unselect all", command=self.unselect_all)
        editmenu.add_command(label="Invert Selection", command=self.invert_selection)

        menubar.add_cascade(label="Edit", menu=editmenu)

        actionmenu = Menu(menubar, tearoff=0)

        actionmenu.add_command(label="Read", command=self.read_all)
        actionmenu.add_command(label="Write", command=self.write_all)
        actionmenu.add_command(label="Verify", command=self.verify_all)
        menubar.add_cascade(label="Actions", menu=actionmenu)

        self.config(menu=menubar)

        self.thread = threading.Thread(target=self.my_thread)
        self.thread.daemon = True
        self.thread.start()

    def select_all(self):
        for this_node in self.node_params:
            this_node.select_status.set(1)

    def unselect_all(self):
        for this_node in self.node_params:
            this_node.select_status.set(0)

    def invert_selection(self):
        for this_node in self.node_params:
            this_node.select_status.set(not this_node.select_status.get())

    def on_resize(self, event):
        # determine the ratio of old width/height to new width/height
        wscale = float(event.width)/self.width
        hscale = float(event.height)/self.height
        self.width = event.width
        self.height = event.height

    def my_thread(self):

        while True:
            self.operation_wait_event.wait()

            while self.operation is not None:
                if self.operation == 'READ':
                    all_done = True
                    for this_node in self.node_params:
                        if this_node.select_status.get():
                            for node in nodes:
                                if node.unit_name == this_node.select_button.cget("text"):
                                    if node.unit_info_read_step == UnitParameterSteps.DONE:
                                        # Unlock all the Entry boxes
                                        for child in this_node.children.values():
                                            if child.winfo_class() == 'Entry':
                                                child.configure(state=NORMAL)
                                                child.delete(0, END)  # Doesn't kill to do another delete

                                        this_node.Address_Box.insert(0, node.parameters.Address)
                                        this_node.Zone_Box.insert(0, node.parameters.Zone)
                                        this_node.SyncMaster_Box.insert(0, node.parameters.SyncMaster)
                                        this_node.FrequencyChannel_Box.insert(0, node.parameters.FrequencyChannel)
                                        this_node.DeviceCombination_Box.insert(0, node.parameters.DeviceCombination)
                                        this_node.SystemId_Box.insert(0, node.parameters.SystemId)
                                        this_node.SerialNumber_Box.insert(0, node.parameters.SerialNumber)
                                        this_node.TxLowPower_Box.insert(0, node.parameters.TxLowPower)
                                        this_node.TxHighPower_Box.insert(0, node.parameters.TxHighPower)
                                        this_node.BootUsePPU_Box.insert(0, node.parameters.BootUsePPU)
                                        this_node.PpModeEnable_Box.insert(0, node.parameters.PpModeEnable)

                                        # Lock all the Entry boxes
                                        for child in this_node.children.values():
                                            if child.winfo_class() == 'Entry':
                                                child.configure(state=DISABLED)
                                    else:
                                        all_done = False
                                    break

                    if all_done:
                        for node in nodes:
                            node.unit_info_read_step = UnitParameterSteps.INIT
                            node.unit_info_action = None
                            node.unit_info_var = UnitParameterActions.INIT
                        self.operation = None

                elif self.operation == 'WRITE':
                    all_done = True
                    for this_node in self.node_params:
                        if this_node.select_status.get():
                            for node in nodes:
                                if node.unit_name == this_node.select_button.cget("text"):
                                    if node.unit_info_write_step != UnitParameterSteps.DONE:
                                        all_done = False
                                    break

                    if all_done:
                        for this_node in self.node_params:
                            if this_node.select_status.get():
                                for node in nodes:
                                    if node.unit_name == this_node.select_button.cget("text"):
                                        node.unit_info_write_step = UnitParameterSteps.INIT
                                        node.unit_info_action = None
                                        node.unit_info_var = UnitParameterActions.INIT
                                        break
                        result = messagebox.askyesno("Info", "Would you like to verify?", icon='info')
                        if result:
                            # Launch a read operation
                            for this_node in self.node_params:
                                if this_node.select_status.get():
                                    for node in nodes:
                                        if node.unit_name == this_node.select_button.cget("text"):
                                            node.parameters.reset()
                                            node.unit_info_var = UnitParameterActions.READ
                                            break

                            self.operation = 'VERIFY'
                        else:
                            self.operation = None

                if self.operation == 'VERIFY':
                    all_done = True
                    error = False
                    for this_node in self.node_params:
                        if this_node.select_status.get():
                            for node in nodes:
                                if node.unit_name == this_node.select_button.cget("text"):
                                    if node.unit_info_read_step == UnitParameterSteps.DONE:
                                        parameters = NodeParameters(UnitName=this_node.select_button.cget("text"),
                                                                    SyncMaster=this_node.SyncMaster_Box.get(),
                                                                    Address=this_node.Address_Box.get(),
                                                                    Zone=this_node.Zone_Box.get(),
                                                                    FrequencyChannel=this_node.FrequencyChannel_Box.get(),
                                                                    DeviceCombination=this_node.DeviceCombination_Box.get(),
                                                                    SystemId=this_node.SystemId_Box.get(),
                                                                    SerialNumber=this_node.SerialNumber_Box.get(),
                                                                    TxLowPower=this_node.TxLowPower_Box.get(),
                                                                    TxHighPower=this_node.TxHighPower_Box.get(),
                                                                    BootUsePPU=this_node.BootUsePPU_Box.get(),
                                                                    PpModeEnable=this_node.PpModeEnable_Box.get(),
                                                                    )

                                        if parameters != node.parameters:
                                            # Not identical
                                            error = True
                                    else:
                                        # Not all nodes finished their operation
                                        all_done = False
                                        break

                    if all_done:
                        if error:
                            messagebox.showerror("Parameters", "Verification Failed")
                        else:
                            messagebox.showinfo("Parameters", "Verification Success")

                        for this_node in self.node_params:
                            if this_node.select_status.get():
                                for node in nodes:
                                    node.unit_info_read_step = UnitParameterSteps.INIT
                                    node.unit_info_action = None
                                    node.unit_info_var = UnitParameterActions.INIT
                        self.operation = None
                time.sleep(0.5)

            self.operation_wait_event.clear()

    def cancel_button(self):
        self.destroy()

    def load_from_file(self):
        ext = ""
        while ext != ".json":
            filename = filedialog.askopenfilename(initialdir=os.getcwd(),
                                                  title="select the parameter file",
                                                  filetypes=(("JSON files", "*.JSON"), ("all files", "*.*")))

            if not filename:
                # The user most likely canceled the operation
                return

            file, ext = os.path.splitext(filename)

        try:
            with open(filename, "r") as read_file:
                if read_file is not None:
                    data_load = json.load(read_file)

                    for node_params in data_load:
                        unit_name = node_params['UnitName']
                        for this_node in self.node_params:
                            if this_node.select_button.cget("text") == unit_name:
                                # Unlock all the Entry boxes
                                for child in this_node.children.values():
                                    if child.winfo_class() == 'Entry':
                                        child.configure(state=NORMAL)
                                        child.delete(0, END)
                                this_node.Address_Box.insert(0, node_params['Address'])
                                this_node.Zone_Box.insert(0, node_params['Zone'])
                                this_node.SyncMaster_Box.insert(0, node_params['SyncMaster'])
                                this_node.FrequencyChannel_Box.insert(0, node_params['FrequencyChannel'])
                                this_node.DeviceCombination_Box.insert(0, node_params['DeviceCombination'])
                                this_node.SystemId_Box.insert(0, node_params['SystemId'])
                                this_node.SerialNumber_Box.insert(0, node_params['SerialNumber'])
                                this_node.TxLowPower_Box.insert(0, node_params['TxLowPower'])
                                this_node.TxHighPower_Box.insert(0, node_params['TxHighPower'])
                                this_node.BootUsePPU_Box.insert(0, node_params['BootUsePPU'])
                                this_node.PpModeEnable_Box.insert(0, node_params['PpModeEnable'])

                                # Lock all the Entry boxes
                                for child in this_node.children.values():
                                    if child.winfo_class() == 'Entry':
                                        child.configure(state=DISABLED)
                                break
        except:
            messagebox.showerror("ERROR", "Please select a valid Parameter JSON file !")

    def unlock_parameters(self):
        for node in self.node_params:
            for child in node.children.values():
                if child.winfo_class() == 'Entry':
                    child.configure(state=NORMAL)

    def lock_parameters(self):
        for node in self.node_params:
            for child in node.children.values():
                if child.winfo_class() == 'Entry':
                    child.configure(state=DISABLED)

    def common_parameters(self):
        values = ['Select Param']
        values.extend(node_parameters)

        win = Toplevel(self)
        frame = Frame(win, bd=1, relief='groove', borderwidth=3)
        self.sel_common_param = StringVar(self, value=values[0])

        OptionMenu(frame, self.sel_common_param, *values).pack()
        self.common_param_value = Entry(frame, width=12, font="Helvetica 10", justify="left")
        self.common_param_value.pack()
        Button(frame, text="SET", width=12, command=self.set_common_value).pack()

        frame.pack()
        win.resizable(False, False)

    def set_common_value(self):
        selected_item = self.sel_common_param.get()
        box = None
        box_name = selected_item + '_Box'

        if selected_item != 'Select Param':
            for this_node in self.node_params:
                box = getattr(this_node, box_name)
                box.configure(state=NORMAL)
                box.delete(0, END)  # Doesn't kill to do another delete
                box.insert(0, self.common_param_value.get())

                box.configure(state=DISABLED)

    def save_to_file(self):

        filename = filedialog.asksaveasfilename(initialdir=os.getcwd(),
                                                title="select the configuration file",
                                                filetypes=(("JSON files", "*.JSON"), ("all files", "*.*")),
                                                defaultextension='json')
        if not filename:
            # The user most likely canceled the operation
            return
        try:
            with open(filename, 'w+') as write_file:
                if write_file is not None:

                    all_nodes = []

                    for this_node in self.node_params:

                        this_node_parameters = {
                            'UnitName': this_node.select_button.cget("text"),
                            'SyncMaster': this_node.SyncMaster_Box.get(),
                            'Address': this_node.Address_Box.get(),
                            'Zone': this_node.Zone_Box.get(),
                            'FrequencyChannel': this_node.FrequencyChannel_Box.get(),
                            'DeviceCombination': this_node.DeviceCombination_Box.get(),
                            'SystemId': this_node.SystemId_Box.get(),
                            'SerialNumber': this_node.SerialNumber_Box.get(),
                            'TxLowPower': this_node.TxLowPower_Box.get(),
                            'TxHighPower': this_node.TxHighPower_Box.get(),
                            'BootUsePPU': this_node.BootUsePPU_Box.get(),
                            'PpModeEnable': this_node.PpModeEnable_Box.get(),
                        }
                        all_nodes.append(this_node_parameters)

                    json.dump(all_nodes, write_file, indent=4)
        except:
            messagebox.showerror("ERROR", "Error saving the JSON file !")

    def read_all(self):
        for this_node in self.node_params:
            if this_node.select_status.get():
                for child in this_node.children.values():
                    if child.winfo_class() == 'Entry':
                        child.configure(state=NORMAL)
                        child.delete(0, END)
                        child.configure(state=DISABLED)

                for node in nodes:
                    if node.unit_name == this_node.select_button.cget("text"):
                        node.parameters.reset()
                        node.unit_info_var = UnitParameterActions.READ
                        break

        self.operation = 'READ'
        self.operation_wait_event.set()

    def write_all(self):
        for this_node in self.node_params:
            if this_node.select_status.get():
                for node in nodes:
                    if node.unit_name == this_node.select_button.cget("text"):
                        node.parameters.set(UnitName=this_node.select_button.cget("text"),
                                            SyncMaster=this_node.SyncMaster_Box.get(),
                                            Address=str(this_node.Address_Box.get()),
                                            Zone=this_node.Zone_Box.get(),
                                            FrequencyChannel=this_node.FrequencyChannel_Box.get(),
                                            DeviceCombination=this_node.DeviceCombination_Box.get(),
                                            SystemId=this_node.SystemId_Box.get(),
                                            SerialNumber=this_node.SerialNumber_Box.get(),
                                            TxLowPower=this_node.TxLowPower_Box.get(),
                                            TxHighPower=this_node.TxHighPower_Box.get(),
                                            BootUsePPU=this_node.BootUsePPU_Box.get(),
                                            PpModeEnable=this_node.PpModeEnable_Box.get(),
                                            )
                        node.unit_info_var = UnitParameterActions.WRITE
                        break

        self.operation = 'WRITE'
        self.operation_wait_event.set()

    def verify_all(self):
        # Launch a fresh read operation
        for this_node in self.node_params:
            if this_node.select_status.get():
                for node in nodes:
                    if node.unit_name == this_node.select_button.cget("text"):
                        node.parameters.reset()
                        node.unit_info_var = UnitParameterActions.READ
                        break
        self.operation = 'VERIFY'
        self.operation_wait_event.set()


class DebugPanel(Toplevel):
    """"""

    # ----------------------------------------------------------------------
    def __init__(self):
        """Constructor"""
        Toplevel.__init__(self)


    def my_thread(self):

        pass

    def cancel_button(self):
        self.destroy()

class StatsWorker(Toplevel):
    """"""

    # ----------------------------------------------------------------------
    def __init__(self):
        """Constructor"""
        Toplevel.__init__(self)
        self.geometry("300x200")
        self.title("STATs")


def add_node_callback():
    AddNodePanel()

def return_main_page_callback():
    global load_configuration_button
    global save_configuration_button
    global add_node_button
    global  reset_all_button
    load_configuration_button.place(x=5, y=10)
    save_configuration_button.place(x=5, y=40)
    add_node_button.place(x=5, y=70)
    reset_all_button.place(x=125, y=10)


def main_save_configuration_callback():
    global nodes
    global global_logging_state

    filename = filedialog.asksaveasfilename(initialdir=os.getcwd(),
                                            title="select the configuration file",
                                            filetypes=(("JSON files", "*.JSON"), ("all files", "*.*")),
                                            defaultextension='json')
    if not filename:
        # The user most likely canceled the operation
        return
    try:
        with open(filename, 'w+') as write_file:
            if write_file is not None:
                global_config = {
                    "logging state": global_logging_state
                }

                all_nodes = []
                for this_node in nodes:
                    node_config = {
                        "unit name": this_node.unit_name,
                        "COM Port": this_node.port_var.get(),
                        "GUI x position": this_node.winfo_x(),
                        "GUI y position": this_node.winfo_y(),
                        "UART Baudrate": this_node.baud_entry.get(),
                        "GUI Width": this_node.winfo_width(),
                        "GUI Height": this_node.winfo_height(),
                        "Time-stamp": this_node.time_stamp_status,
                        "Decode Status": this_node.packet_decode_status,
                        "Main Frame Status": this_node.main_frame_new_status,
                    }
                    all_nodes.append(node_config)

                global_config.update({"all nodes": all_nodes})
                json.dump(global_config, write_file, indent=4)
    except:
        messagebox.showerror("ERROR", "Error saving the JSON file !")


def main_load_configuration_callback():
    global nodes

    ext = ""
    while ext != ".json":
        filename = filedialog.askopenfilename(initialdir=os.getcwd(),
                                              title="select the configuration file",
                                              filetypes=(("JSON files", "*.JSON"), ("all files", "*.*")))

        if not filename:
            # The user most likely canceled the operation
            return

        file, ext = os.path.splitext(filename)

    try:
        with open(filename, "r") as read_file:
            if read_file is not None:
                data_load = json.load(read_file)
                if "logging state" in data_load and "all nodes" in data_load:
                    log_control = data_load["logging state"]
                    node_config = data_load["all nodes"]
                    port_list = serial_ports()
                    for this_node in node_config:
                        this_node = RbuNode(main_gui, config=this_node, node_list=nodes, serial_ports_list=port_list)
                        nodes.append(this_node)
                else:
                    raise json.JSONDecodeError('JSON file ', filename, 0)

                log_all_callback(log_control)

                for node in nodes:
                    node.run()
    except json.JSONDecodeError:
        messagebox.showerror("ERROR", "Please select a valid configuration JSON file !")


def reset_all_callback():
    for node in nodes:
        node.unit_reset_operation()


def log_all_callback(control=None):
    global global_logging_state
    if control is None:
        global_logging_state = 'INACTIVE' if global_logging_state == 'ACTIVE' else 'ACTIVE'
    else:
        global_logging_state = control

    log_path = None
    if global_logging_state == 'ACTIVE':
        # create the receiving folder
        date_time = datetime.now().strftime('%a, %d %b %Y %H %M %S')
        log_path = os.path.join(os.getcwd(), 'logs', date_time)
        try:
            os.makedirs(log_path)
        except OSError:
            print('COULD not CREATE LOG FOLDER')
            raise

    for node in nodes:
        if global_logging_state == 'ACTIVE':
            # create a log file for this node
            log_name = node.unit_name + ' ' + date_time + ".log"
            log_file = os.path.join(log_path, log_name)
            try:
                node.logging_file = open(str(log_file), 'w')
            except OSError as exc:  # Guard against race condition
                if exc.errno != errno.EEXIST:
                    print('COULD not OPEN LOG FILE')
                    raise
        else:
            # stop logging
            if node.logging_file is not None:
                node.logging_file.close()
                node.logging_file = None


def statistic_button_callback():
    StatsWorker()


def close_all():
    global close_application
    close_application = True


def firmware_update_callback():
    if len(nodes):
        FirmwareUpdatePanel()
    else:
        messagebox.showerror("ERROR", "Please Load a System Configuration !")


def mesh_status_callback():
    MeshStatusPanel()


def parameters_callback():
    if len(nodes):
        ParametersPanel()
    else:
        messagebox.showerror("ERROR", "Please Load a System Configuration !")


def debug_callback():
    DebugPanel()


def main_gui_worker():
    global log_all_button
    global mesh_status_button
    global global_logging_state
    global report_mesh_state
    global mesh_graph_handle
    global nodes
    global close_application
    global report_mesh_state_request

    if global_logging_state == 'ACTIVE':
        log_all_button.configure(relief=SUNKEN)
    else:
        log_all_button.configure(relief=RAISED)

    if report_mesh_state_request is True and report_mesh_state is None:
        report_mesh_state = 'ACTIVE'
        for node in nodes:
            node.report_associated_nodes()
    elif report_mesh_state == 'ACTIVE':
        all_done = True
        error = False
        for node in nodes:
            if node.report_associated_node_state == ReportAssociatedNodeState.START or\
                    node.report_associated_node_state == ReportAssociatedNodeState.GET_NODES or\
                    node.report_associated_node_state == ReportAssociatedNodeState.PROCESS:
                all_done = False
                break
            elif node.report_associated_node_state == ReportAssociatedNodeState.ERROR or node.mesh_rank == -1:
                error = True

        if all_done:
            try:
                if not error:
                    mesh_graph_handle = nx.DiGraph()
                    primary_parents = []
                    secondary_parents = []
                    primary_tracking_nodes = []
                    secondary_tracking_nodes = []
                    children = []
                    names = [node.unit_name for node in nodes]
                    addresses = [node.unit_address for node in nodes]
                    parent_detailed_tab =[["Node", "Rank", "Primary Parent", "SNR", "Secondary Parent", "SNR", "Primary Tracking Node", "SNR", "Secondary Tracking Node", "SNR"]]
                    now_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

                    for node in nodes:
                        prim_parent = "NONE"
                        second_parent = "NONE"
                        prim_track_node = "NONE"
                        second_track_node = "NONE"
                        if node.mesh_parents[0][0] != -1:
                            prim_parent = names[addresses.index(node.mesh_parents[0][0])]
                            primary_parents.append((node.unit_name, prim_parent))
                        if node.mesh_parents[1][0] != -1:
                            second_parent = names[addresses.index(node.mesh_parents[1][0])]
                            secondary_parents.append((node.unit_name, second_parent))
                        if node.tracking_nodes[0][0] != -1:
                            prim_track_node = names[addresses.index(node.tracking_nodes[0][0])]
                            primary_tracking_nodes.append((node.unit_name, prim_track_node ))
                        if node.tracking_nodes[1][0] != -1:
                            second_track_node = names[addresses.index(node.tracking_nodes[1][0])]
                            secondary_tracking_nodes.append((node.unit_name, second_track_node))

                        for child in node.mesh_children:
                            children.append((node.unit_name, names[addresses.index(int(child[0]))]))

                        parent_detailed_tab.append([node.unit_name,
                                                    node.mesh_rank,
                                                    prim_parent,
                                                    node.mesh_parents[0][1],
                                                    second_parent,
                                                    node.mesh_parents[1][1],
                                                    prim_track_node,
                                                    node.tracking_nodes[0][1],
                                                    second_track_node,
                                                    node.tracking_nodes[1][1]
                                                    ])

                    table_window = Toplevel(main_gui)
                    table_window.title(now_time)

                    for row_index, row_data in enumerate(parent_detailed_tab):
                        for col_index, col_data in enumerate(row_data):
                            bg_color = "#ccc" if (row_index == 0 or col_index == 0) else "#fff"

                            Label001 = ttk.Label(
                                table_window,
                                background=bg_color,
                                borderwidth=1,
                                relief=RAISED,
                                text=col_data
                            )
                            Label001.grid(
                                row=row_index,
                                column=col_index,
                                sticky=tkinter.W + tkinter.E + tkinter.N + tkinter.S
                            )

                    if len(children):
                        mesh_graph_handle.add_edges_from(children, weight=5)

                    if len(secondary_tracking_nodes):
                        mesh_graph_handle.add_edges_from(secondary_tracking_nodes, weight=4)

                    if len(primary_tracking_nodes):
                        mesh_graph_handle.add_edges_from(primary_tracking_nodes, weight=3)

                    if len(secondary_parents):
                        mesh_graph_handle.add_edges_from(secondary_parents, weight=2)

                    if len(primary_parents):
                        mesh_graph_handle.add_edges_from(primary_parents, weight=1)

                    report_mesh_state = None
                    report_mesh_state_request = False
                    rank_dict = dict()

                    for node in nodes:
                        rank_dict[node.unit_name] = node.mesh_rank
                        node.report_associated_node_state = ReportAssociatedNodeState.INIT

                    fixed_positions = {'NCU': (0.5, 1)}
                    max_rank = max(rank_dict.items(), key=operator.itemgetter(1))[1]
                    y_step = 1.0/max_rank

                    for rank in range(1, max_rank+1):

                        nodes_with_this_rank = {k: v for k, v in rank_dict.items() if v == rank}
                        if len(nodes_with_this_rank):
                            x_step = 1.0/len(nodes_with_this_rank)

                            for index, item in enumerate(nodes_with_this_rank):
                                fixed_positions[item] = (x_step/2 + (index*x_step), 1-(rank*y_step))

                    val_map = {'A': 1.0,
                               'D': 0.5714285714285714,
                               'H': 0.0}

                    values = [val_map.get(node, 0.45) for node in mesh_graph_handle.nodes()]

                    def my_label(x):
                        return 'P-P' if x == 1 else 'S-P' if x == 2 else 'P-T-N' if x == 3 else 'S-T-N' if x == 4 else 'child'

                    edge_labels = dict([((u, v,), my_label(d['weight']))
                                        for u, v, d in mesh_graph_handle.edges(data=True)])

                    edge_colors = ['red' if edge in primary_parents else 'green' for edge in mesh_graph_handle.edges() ]

                    draw_win = Toplevel()
                    draw_win.wm_title(now_time)
                    # Quit when the window is done
                    draw_win.wm_protocol('WM_DELETE_WINDOW', draw_win.destroy)

                    f = plt.figure()
                    plt.axis('off')

                    a = f.add_subplot(111)
                    pos = nx.spring_layout(mesh_graph_handle, pos=fixed_positions, fixed=fixed_positions.keys())
                    nx.draw_networkx_edge_labels(mesh_graph_handle, pos, edge_labels=edge_labels, font_size=9)
                    nx.draw_networkx(mesh_graph_handle, pos, node_color='g', node_size=1000, edge_color=edge_colors,
                                     edge_cmap=plt.cm.Reds, with_labels=True, ax=a)

                    canvas = FigureCanvasTkAgg(f, master=draw_win)
                    NavigationToolbar2Tk(canvas, draw_win)

                    canvas.draw()
                    canvas.get_tk_widget().pack(side=TOP, fill=BOTH, expand=1)

                    def draw_all():
                        a.cla()
                        mesh_graph_handle.clear()

                        if len(children):
                            mesh_graph_handle.add_edges_from(children, weight=4)

                        if len(secondary_tracking_nodes):
                            mesh_graph_handle.add_edges_from(secondary_tracking_nodes, weight=4)

                        if len(primary_tracking_nodes):
                            mesh_graph_handle.add_edges_from(primary_tracking_nodes, weight=3)

                        if len(secondary_parents):
                            mesh_graph_handle.add_edges_from(secondary_parents, weight=2)

                        if len(primary_parents):
                            mesh_graph_handle.add_edges_from(primary_parents, weight=1)

                        nx.draw_networkx_edge_labels(mesh_graph_handle, pos, edge_labels=edge_labels, font_size=9)
                        nx.draw_networkx(mesh_graph_handle, pos, node_color='g', node_size=1000, edge_color=edge_colors,
                                         edge_cmap=plt.cm.Reds, with_labels=True)
                        plt.axis('off')
                        canvas.draw()

                    def draw_parents():
                        a.cla()
                        mesh_graph_handle.clear()

                        if len(primary_parents):
                            mesh_graph_handle.add_edges_from(primary_parents, weight=1)

                        if len(secondary_parents):
                            mesh_graph_handle.add_edges_from(secondary_parents, weight=2)
                        edge_labels = dict([((u, v,), my_label(d['weight']))
                                            for u, v, d in mesh_graph_handle.edges(data=True)])

                        edge_colors = ['red' if edge in primary_parents else 'green' for edge in
                                       mesh_graph_handle.edges()]
                        nx.draw_networkx_edge_labels(mesh_graph_handle, pos, edge_labels=edge_labels, font_size=9)
                        nx.draw_networkx(mesh_graph_handle, pos, node_color='g', node_size=1000, edge_color=edge_colors,
                                         edge_cmap=plt.cm.Reds, with_labels=True)
                        plt.axis('off')
                        canvas.draw()

                    def draw_tracking_nodes():
                        a.cla()
                        mesh_graph_handle.clear()
                        if len(primary_tracking_nodes):
                            mesh_graph_handle.add_edges_from(primary_tracking_nodes, weight=3)

                        if len(secondary_tracking_nodes):
                            mesh_graph_handle.add_edges_from(secondary_tracking_nodes, weight=4)

                        edge_labels = dict([((u, v,), my_label(d['weight']))
                                            for u, v, d in mesh_graph_handle.edges(data=True)])

                        edge_colors = ['red' if edge in primary_parents else 'green' for edge in
                                       mesh_graph_handle.edges()]
                        nx.draw_networkx_edge_labels(mesh_graph_handle, pos, edge_labels=edge_labels, font_size=9)
                        nx.draw_networkx(mesh_graph_handle, pos, node_color='g', node_size=1000,
                                         edge_color=edge_colors,
                                         edge_cmap=plt.cm.Reds, with_labels=True)
                        plt.axis('off')
                        canvas.draw()
                    def draw_children():
                        a.cla()
                        mesh_graph_handle.clear()

                        if len(children):
                            mesh_graph_handle.add_edges_from(children, weight=5)

                            edge_labels = dict([((u, v,), my_label(d['weight']))
                                                for u, v, d in mesh_graph_handle.edges(data=True)])

                            edge_colors = ['red' if edge in primary_parents else 'green' for edge in
                                           mesh_graph_handle.edges()]

                            nx.draw_networkx_edge_labels(mesh_graph_handle, pos, edge_labels=edge_labels, font_size=9)
                            nx.draw_networkx(mesh_graph_handle, pos, node_color='g', node_size=1000, edge_color=edge_colors,
                                             edge_cmap=plt.cm.Reds, with_labels=True)
                        plt.axis('off')
                        canvas.draw()

                    button_frame = Frame(draw_win, height=25, width=canvas.get_width_height()[0], bd=0, relief=FLAT)
                    button_frame.pack()
                    Button(button_frame, text="Show All", command=draw_all, width=10, relief=RAISED).place(x=50)
                    Button(button_frame, text="Show Parents", command=draw_parents, width=15, relief=RAISED).place(x=140)
                    Button(button_frame, text="Show Tracking Nodes", command=draw_tracking_nodes, width=20, relief=RAISED).place(x=260)
                    Button(button_frame, text="Show Children", command=draw_children, width=15, relief=RAISED).place(x=420)
                else:
                    messagebox.showerror("ERROR", "(Error)Please Make sure that all the nodes have joined the mesh !")
                    report_mesh_state_request = False
                    report_mesh_state = None
            except:
                messagebox.showerror("ERROR", "(Except)Please Make sure that all the nodes have joined the mesh !")
                report_mesh_state_request = False
                report_mesh_state = None

    if close_application is False:
        main_gui.after(500, main_gui_worker)
    else:
        # Block the threads first
        for node in nodes:
            node.thread_stop.set()

        while len(nodes):
            time.sleep(0.25)
        main_gui.destroy()


if __name__ == "__main__":
    version_number = '1.1.0'
    matplotlib.use('agg')
    matplotlib = matplotlib.reload(matplotlib)
    import matplotlib.pyplot as plt
    from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
    import pylab
    import ctypes
    main_gui = Tk()
    icon = None
    log_all_button = None
    mesh_status_button = None
    global_logging_state = None
    report_mesh_state = None
    close_application = False
    report_mesh_state_request = False
    mesh_graph_handle = None

    window_title = 'Cygnus2 - ' + version_number

    if TkVersion >= 8.5:
        try:
            icon = PhotoImage(file=os.path.join(os.getcwd(), 'logo', 'BP_Logo.gif'))
        except:
            icon = PhotoImage(file=resource_path('BP_Logo.gif'))

        main_gui.wm_iconphoto(True, icon, icon)
    else:
        icon = PhotoImage(file=os.path.join(os.getcwd(), 'logo', 'BP_Logo.ico'))
        main_gui.iconbitmap(True, icon)

    ttk.report_callback_exception = exception_callback
    main_gui.title(window_title)
    main_gui.geometry('250x270+10+10')
    main_gui.resizable(False, False)

    main_gui.protocol("WM_DELETE_WINDOW", close_all)

    main_frame = Frame(main_gui, height=245, width=265, bd=1, relief='groove')

    load_configuration_button = Button(main_frame, text="Load Configuration", width=15, command=main_load_configuration_callback)
    save_configuration_button = Button(main_frame, text="Save Configuration", width=15, command=main_save_configuration_callback)
    add_node_button = Button(main_frame, text="Add Node", width=15, command=add_node_callback)
    reset_all_button = Button(main_frame, text="Reset All", width=15, command=reset_all_callback)
    log_all_button = Button(main_frame, text="Logging", width=15, command=log_all_callback)
    statistics_button = Button(main_frame, text="STATS", width=15, command=statistic_button_callback)
    firmware_update_button = Button(main_frame, text="FW Update", width=15, command=firmware_update_callback)
    mesh_status_button = Button(main_frame, text="Mesh", width=15, command=mesh_status_callback)
    Parameter_button = Button(main_frame, text="Parameters", width=15, command=parameters_callback)

    load_configuration_button.place(x=5, y=10)
    save_configuration_button.place(x=5, y=40)
    add_node_button.place(x=5, y=70)
    firmware_update_button.place(x=5, y=100)
    Parameter_button.place(x=5, y=130)
    reset_all_button.place(x=125, y=10)
    log_all_button.place(x=125, y=40)
    statistics_button.place(x=125, y=70)
    mesh_status_button.place(x=125, y=100)
    main_frame.pack()

    try:
        cygnus_image = Image.open(os.path.join(os.getcwd(), 'logo', 'Cygnus.jpg'))
    except:
        try:
            cygnus_image = Image.open(resource_path('Cygnus.jpg'))
        except:
            messagebox.showerror("ERROR", "Problem with Cygnus.jpg")

    photo = ImageTk.PhotoImage(cygnus_image)
    label = Label(image=photo)
    label.pack()

    main_gui.update_idletasks()
    label.place(x=-1, y=main_gui.winfo_height()-photo.height())
    main_gui.after(1000, main_gui_worker)
    main_gui.mainloop()
