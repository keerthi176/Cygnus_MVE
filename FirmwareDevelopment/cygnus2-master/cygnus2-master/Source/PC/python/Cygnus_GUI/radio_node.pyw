"""
GUI Application for the Cygnus II RBU control and debug purposes
"""

import os
import errno
import time
import threading
import traceback
from datetime import datetime
from tkinter import ttk
from tkinter import *
from tkinter import messagebox
from collections import deque
import serial
from serial.tools import list_ports
from PIL import Image, ImageTk
import ctypes
import PY_MC_PUP
sys.path.append(os.path.join(os.getcwd(), 'xmodem-ymodem'))
from xmodem import YMODEM
from CO_Message import *
from shared import *

# To have the logo on the Taskbar
geometry_full_width = 820
geometry_default_height = 610
geometry_reduced_width = 520
geometry_reduced_height = 135
console_max_line_nbr = 10000

w32 = ctypes.windll.kernel32

mesh_data_pattern = re.compile("(\[.*\] )?\[([0-9]*),([rt]),([0-9]*),([0-9]*),([-0-9]*),([-0-9]*),([0-9A-F]*).*\]")


class RbuNode(Toplevel):
    def __init__(self, master, config=None, node_list=None, serial_ports_list=None):
        Toplevel.__init__(self)
        self.node_list = node_list
        self.master = master
        self.node_main_thread_id = None
        self.read_thread = None
        self.serial_object = None
        self.packet_decode_status = False
        self.time_stamp_status = False
        self.main_frame_actual_status = None
        self.main_frame_new_status = None
        self.main_frame_status_cfg = None
        self.show_full_view_req = None
        self.logging_file = None
        self.log_flush_time = datetime.now()
        self.unit_address = None
        self.unit_address_request_time = None
        self.decode_function = None
        self.unit_info_read_new_req = False
        self.unit_info_write_new_req = False
        self.unit_info_read_step = UnitParameterSteps.INIT
        self.unit_info_write_step = UnitParameterSteps.INIT
        self.last_at_command_time = datetime.now()
        self.main_frame = None
        self.port_var = None
        self.at_cmd_var = None
        self.serial_rx_queue = deque(maxlen=500)
        self.at_last_command = None
        self.at_last_operation_type = None
        self.at_cmd_selected_type = None
        self.unit_info_read_in_progress = False
        self.unit_info_write_in_progress = False
        self.hidden_frame = None
        self.console = None
        self.console_scrollbar = None
        self.console_freeze = None
        self.children_nbr_box = None
        self.parents_nbr_box = None
        self.tracking_node_nbr_box = None
        self.long_frame_idx_box = None
        self.rank_box = None
        self.state_box = None
        self.baud_entry = None
        self.connection_button = None
        self.decode_button = None
        self.timestamp_button = None
        self.poll_button = None
        self.clear_cie_button = None
        self.auto_alarm_checkbutton = None
        self.command_entry = None
        self.AT_command_rw_value = None
        self.AT_command_r_button = None
        self.AT_command_w_button = None
        self.AT_command_ctrl_button = None
        self.port_menu = None
        self.decoded_line = None
        self.firmware_update_state = FirmwareUpdateState.IDLE
        self.firmware_update_unit_rescue = False
        self.firmware_update_result = None
        self.firmware_file = None
        self.firmware_version_nbr = None
        self.firmware_version_date = None
        self.firmware_checksum = None
        self.modem = YMODEM(self.serial_getc, self.serial_putc)
        self.serial_object_lock = threading.Event()
        self.report_associated_node_state = ReportAssociatedNodeState.INIT
        self.mesh_rank = -1
        self.number_of_parents = -1
        self.number_of_tracking_nodes = -1
        self.number_of_children = -1
        self.mesh_parents = [(-1, -1), (-1, -1)]
        self.tracking_nodes = [(-1, -1), (-1, -1)]
        self.mesh_children = []
        self.report_assoc_node_time = datetime.now()
        self.wait_for_app_time = datetime.now()
        self.number_of_resets = 0
        self.reset_detected = False
        self.fw_active_bank = None
        self.thread_stop = threading.Event()
        self.unit_info_action = None
        self.unit_info_var = UnitParameterActions.INIT
        self.event_queue = deque(maxlen=50)
        self.parameters = NodeParameters()
        self.poll_ncu = False # change this to true if you are running the NCU application instead of the stub
        self.ncu_poll_count = 0
        self.msn_count = 0
        self.auto_alarm_enable = False

        """
        s = self.Style()
        s.theme_use("default")
        s.configure("ProgSuccess", foreground='green', background='green')
        s.configure("ProgFailure", foreground='orange', background='orange')
        """

        if config is not None:
            self.unit_name = config["unit name"]
            self.default_port = config["COM Port"]
            self.gui_pos_x = config["GUI x position"]
            self.gui_pos_y = config["GUI y position"]
            self.default_baud = config["UART Baudrate"]
            self.gui_width = config["GUI Width"]
            self.gui_height = config["GUI Height"]
            self.time_stamp_status = config["Time-stamp"]
            self.packet_decode_status = config["Decode Status"]
            self.main_frame_status_cfg = config["Main Frame Status"].strip()
            self.main_frame_new_status = self.main_frame_status_cfg
        else:
            self.unit_name = "CYGNUS II"
            self.default_port = None
            self.gui_pos_x = None
            self.gui_pos_y = None
            self.gui_width = 600
            self.gui_height = 610
            self.default_baud = '2000000'
            self.packet_decode_status = False
            self.main_frame_new_status = 'SHOW'

        self.title(self.unit_name)

        # Frames
        self.main_frame = Frame(self, height=620, width=320, bd=0, relief='groove')
        self.data_frame = Frame(self.main_frame, height=345, width=275, bd=3, relief='groove')
        self.data_frame.place(x=2, y=2)
        self.cie_frame = Frame(self.main_frame, height=140, width=275, bd=3, relief='groove')
        self.cie_frame.place(x=2, y=210)
        self.control_frame = Frame(self.main_frame, height=110, width=275, bd=3, relief='groove')
        self.control_frame.place(x=2, y=350)
        self.at_command_frame = Frame(self.main_frame, height=145, width=275, bd=3, relief='groove')
        self.at_command_frame.place(x=2, y=465)
        self.raw_at_command_frame = Frame(self.at_command_frame, height=40, width=260, bd=2, relief='groove')
        self.raw_at_command_frame.place(x=2, y=50)

        self.hidden_frame = Frame(self, height=620, width=25, bd=0, relief='groove')
        """
        self.frame_fw_up = Frame(self.main_frame, height=80, width=275, bd=3, relief='groove')
        Label(self.frame_fw_up, text="Firmware Update Progress", font="Helvetica 10").place(x=5, y=10)
        self.progress_bar = ttk.Progressbar(self.frame_fw_up, orient="horizontal",
                                            length=250, mode="determinate")
        self.progress_bar.place(x=5, y=35)
        self.frame_fw_up.pack()
        self.frame_fw_up.forget()
        """
        # Build the console widget
        self.console = Text(master=self, height=20, width=80, state=DISABLED, font=("Helvetica", 8), undo=False)
        self.console.place(x=495, y=5)

        self.console_scrollbar = Scrollbar(self, orient="vertical", command=self.console_yview)
        self.console.configure(yscrollcommand=self.console_scrollbar.set)
        self.console_scrollbar.pack(side="right", fill="y")
        self.console.pack(side="right", fill="y", expand=False)
        self.console.tag_configure("decoded_data", foreground="blue")
        self.console.tag_configure("pointing_hand", foreground="blue", font=28)
        self.console.tag_configure("trivial_tag", foreground="grey")
        self.console.tag_configure("dbg_error_tag", foreground="red")
        self.console.tag_configure("dbg_bit_tag", foreground="orange")

        if self.unit_name == 'PPU':
            Label(self.data_frame, text="PP Device List", font=("Helvetica", 10, "bold")).place(x=135, y=5)
            self.ppu_device_list = Listbox(self.data_frame, height=16, relief=SUNKEN, bg='gray90')
            self.ppu_device_list.bind('<Double-1>', self.ppu_unit_selection)
            self.ppu_device_list.place(x=135, y=25)
            Tooltip(self.ppu_device_list, text='List of devices detected in the Peer to Peer mode.\n'
                    'Double click on an entry to automatically populate the destination address in the AT Command field')

            self.ppu_enable_button = Button(self.data_frame, text="ENABLE", width=16, command=self.ppu_enable)
            self.ppu_enable_button.place(x=5, y=5)

            self.ppu_enable_button = Button(self.data_frame, text="ENABLE", width=16, command=self.ppu_enable)
            self.ppu_enable_button.place(x=5, y=5)

            self.ppu_frequency_var = StringVar()
            self.ppu_frequencies = OptionMenu(self.data_frame, self.ppu_frequency_var, *['CH %d - %d Hz' % (idx, freq) for idx, freq in enumerate(frequencyChannels)])
            self.ppu_frequencies.config(width=13, anchor=W)
            self.ppu_frequencies.place(x=5, y=35)
            Button(self.data_frame, text="SET FREQUENCY", width=16, justify=CENTER, command=self.ppu_set_freq).place(x=5, y=70)

            Button(self.data_frame, text="System ID", command=self.ppu_set_system_id).place(x=5, y=105)
            self.ppu_system_id = Entry(self.data_frame, width=6, font="Helvetica 10", justify=LEFT)
            self.ppu_system_id.place(x=75, y=108)
            Button(self.data_frame, text="Broadcast PP Request", command=self.broadcast_pp_request).place(x=5, y=140)
            self.ppu_refresh_period = Entry(self.data_frame, width=3, font="Helvetica 10", justify=RIGHT)
            self.ppu_refresh_status = IntVar(value=0)
            Checkbutton(self.data_frame, text="Refresh", variable=self.ppu_refresh_status).place(x=135, y=285)
            self.ppu_refresh_period.place(x=202, y=287)
            self.ppu_refresh_period.insert(END, 5)
            Label(self.data_frame, text="Sec", font="Helvetica 10").place(x=235, y=285)
            self.ppu_list_refresh_time = datetime.now()
            self.ppu_refresh_button = Button(self.data_frame, text="REFRESH", width=16, command=self.ppu_refresh_list)
            self.ppu_refresh_button.place(x=135, y=310)
        else:
            # Number of children
            Label(self.data_frame, text="Children", font="Helvetica 10").place(x=5, y=5)
            self.children_nbr_box = Text(self.data_frame, height=1, width=4, font="Helvetica 10", state=DISABLED)
            self.children_nbr_box.place(x=65, y=5)

            # Rank
            Label(self.data_frame, text="Rank", font="Helvetica 10").place(x=5, y=40)
            self.rank_box = Text(self.data_frame, height=1, width=4, font="Helvetica 10", state=DISABLED)
            self.rank_box.place(x=65, y=40)

            # Tracking Nodes
            Label(self.data_frame, text="State", font="Helvetica 10").place(x=110, y=5)
            self.state_box = Text(self.data_frame, height=1, width=6, font="Helvetica 10", state=DISABLED)
            self.state_box.place(x=170, y=5)

            Label(self.data_frame, text="L Frame ", font="Helvetica 10").place(x=110, y=40)
            self.long_frame_idx_box = Text(self.data_frame, height=1, width=6, font="Helvetica 10", state=DISABLED)
            self.long_frame_idx_box.place(x=170, y=40)

        # Baud control
        Label(self.control_frame, text="Baud", font="Helvetica 10").place(x=110, y=5)
        self.baud_entry = Entry(self.control_frame, width=14, font="Helvetica 10", justify="center")
        self.baud_entry.place(x=150, y=5)

        # Connection control
        self.connection_button = Button(self.control_frame, text="Connect", command=self.connection_control, width=10)
        self.connection_button.place(x=15, y=40)

        # Decode control
        self.decode_button = Button(self.control_frame, text="Decode", command=self.packet_decode_control, width=10)
        self.decode_button.place(x=100, y=40)

        # Time-stamp display
        self.timestamp_button = Button(self.control_frame, text="Time-stamp", command=self.time_stamp_control, width=10)
        self.timestamp_button.place(x=185, y=40)

        # Clear console button
        Button(self.control_frame, text="CLR Console", command=self.clear_console, width=10).place(x=15, y=70)

        # Unit Reboot
        Button(self.control_frame, text="REBOOT", command=self.unit_reset_operation, width=10).place(x=185, y=70)

        # CIE controls
        Label(self.cie_frame, text="NCU CIE Queue Controls", font="Helvetica 10").place(x=5, y=2)
        self.poll_button = Button(self.cie_frame, text="Poll", command=self.unit_poll_operation, width=10)
        self.poll_button.place(x=5, y=30)
        Tooltip(self.poll_button, text='Poll NCU for messages')
        # Clear Queues button
        self.clear_cie_button = Button(self.cie_frame, text="Reset Queues", command=self.reset_cie_queues_operation, width=10)
        self.clear_cie_button.place(x=100, y=30)
        Tooltip(self.clear_cie_button, text='Clear all NCU queues')
        #auto alarms checkbox (App will issue an output command to the NCU on receipt of a fire signal)
        self.auto_alarm_checkbutton = Checkbutton(self.cie_frame, text="Auto Alarm", command=self.auto_alarm_operation, width=10)
        self.auto_alarm_checkbutton.place(x=5, y=55)
        Tooltip(self.auto_alarm_checkbutton, text='Send output signal on receipt of FIRE/FIRST AID')
        #queue selection drop list
        self.cie_queue_var = StringVar(self)
        self.cie_queue_var.set("MISC")
        self.cie_queue_menu = OptionMenu(self.cie_frame, self.cie_queue_var, *cie_queue_list, command=self.cie_queue_changed)
        self.cie_queue_menu.config(width=12, anchor=W)
        self.cie_queue_menu.place(x=5, y=85)
        Tooltip(self.cie_queue_menu, text='Select CIE Queue')
        #queue read button
        self.read_cie_queue_button = Button(self.cie_frame, text="Read", command=self.read_queue_operation, width=6)
        self.read_cie_queue_button.place(x=125, y=87)
        Tooltip(self.read_cie_queue_button, text='Read selected CIE queue')
        #queue clear button
        self.clear_cie_queue_button = Button(self.cie_frame, text="Clear", command=self.clear_queue_operation, width=6)
        self.clear_cie_queue_button.place(x=180, y=87)
        Tooltip(self.clear_cie_queue_button, text='Clear selected CIE queue')


        if self.unit_name != 'NCU':
            self.poll_button.configure(state=DISABLED)
            self.clear_cie_button.configure(state=DISABLED)
            self.auto_alarm_checkbutton.configure(state=DISABLED)
            self.cie_queue_menu.configure(state=DISABLED)
            self.read_cie_queue_button.configure(state=DISABLED)
            self.clear_cie_queue_button.configure(state=DISABLED)

        # AT Command menu
        #self.at_cmd_var = StringVar(self, value=at_command_desc_list[0])
        #self.at_cmd_menu = OptionMenu(self.at_command_frame, self.at_cmd_var, *at_command_desc_list)
        self.at_cmd_var = StringVar(self)
        self.at_cmd_var.set(at_command_desc_list[0])
        self.at_cmd_menu = OptionMenu(self.at_command_frame, self.at_cmd_var, *at_command_desc_list, command=self.update_window_layout)
        self.at_cmd_menu.config(width=20, anchor=W)
        self.at_cmd_menu.place(x=15, y=5)
        Tooltip(self.at_cmd_menu, text='Select AT command')

        self.at_cmd_selected_type = None

        self.AT_command_rw_value = Entry(self.at_command_frame, width=22, font="Helvetica 10", justify="left")
        Tooltip(self.AT_command_rw_value, text='R/W Value')

        self.AT_command_r_button = Button(self.at_command_frame, text="READ", command=self.AT_command_read, width=10,
                                          justify="center")
        self.AT_command_w_button = Button(self.at_command_frame, text="WRITE", command=self.AT_command_write, width=10,
                                          justify="center")
        self.AT_command_ctrl_button = Button(self.at_command_frame, text="SEND", command=self.AT_command_control,
                                             width=10, justify="center")

        self.AT_cmd_transaction_id_var = IntVar(value=7)
        self.AT_cmd_transaction_id = OptionMenu(self.at_command_frame, self.AT_cmd_transaction_id_var, *list(range(0, 8)))
        self.AT_cmd_destination_id = Entry(self.at_command_frame, width=12, font="Helvetica 10", justify="left")
        if self.unit_name == 'PPU':
            Tooltip(self.AT_cmd_destination_id,
                    text='d - Destination node ID: Double click on an entry from the PP Device List for auto-fill')
        else:
            Tooltip(self.AT_cmd_destination_id, text='d - Destination node ID')

        self.AT_cmd_type_var = StringVar(self)
        self.AT_cmd_type = OptionMenu(self.at_command_frame, self.AT_cmd_type_var, *[e.name for e in ParameterType])
        self.AT_cmd_type.config(width=20, anchor=W)
        Tooltip(self.AT_cmd_type, text='c - Command type')

        self.AT_cmd_1st_parameter = Entry(self.at_command_frame, width=4, font="Helvetica 10", justify="left")
        Tooltip(self.AT_cmd_1st_parameter, text='p1 - First command parameter')

        self.AT_cmd_2nd_parameter = Entry(self.at_command_frame, width=4, font="Helvetica 10", justify="left")
        Tooltip(self.AT_cmd_2nd_parameter, text='p2 - Second command parameter')

        self.AT_cmd_1st_parameter.insert(INSERT, 0)
        self.AT_cmd_2nd_parameter.insert(INSERT, 0)
        self.AT_output_profile_var = StringVar(self)
        self.AT_output_profile = OptionMenu(self.at_command_frame, self.AT_output_profile_var, *[e.name for e in OutputProfile])
        self.AT_output_profile.config(width=6, anchor=W)

        self.AT_output_zone = Entry(self.at_command_frame, width=4, font="Helvetica 10", justify="left")
        self.AT_output_zone.config(width=4)
        Tooltip(self.AT_output_zone, text='Zone')

        # RAW AT Command
        Button(self.raw_at_command_frame, text="Send RAW AT CMD", command=self.send_at_command_callback, width=16).place(x=2, y=5)
        self.command_entry = Entry(self.raw_at_command_frame, width=16, font="Helvetica 10", justify="left")
        self.command_entry.place(x=130, y=7)
        Tooltip(self.command_entry, text='Type here your command e.g. ATUA?')

        # Collapse the main frame
        Button(self.main_frame, text="<<", command=self.collapse_main_frame, width=3).place(x=280, y=2)
        Button(self.main_frame, text="><", command=self.show_small_view, width=3).place(x=280, y=30)

        # Restore the main frame
        Button(self.hidden_frame, text=">>", command=self.expand_main_frame, width=2).place(x=0, y=2)
        Button(self.hidden_frame, text="<>", command=self.show_full_view, width=2).place(x=0, y=30)

        self.main_frame.pack()

        # Port option menu
        self.port_var = StringVar(self, value=self.default_port)

        self.baud_entry.insert(0, self.default_baud)

        if serial_ports_list is None:
            serial_ports_list = serial_ports()

        if len(serial_ports_list) is not 0:
            if self.default_port is None:
                self.port_var.set(serial_ports_list[0])
            else:
                self.port_var.set(self.default_port)

            self.port_menu = OptionMenu(self.control_frame, self.port_var, *list(serial_ports_list))
        else:
            self.port_menu = OptionMenu(self.control_frame, self.port_var, *list(['NONE']))
            self.port_var.set('NONE')

        if self.default_port is not None:
            self.connection_control()

        self.port_menu.config(width=6)
        self.port_menu.place(x=15, y=2)

        if self.gui_width is not None and self.gui_height is not None and self.gui_pos_x is not None and self.gui_pos_y is not None:
            self.geometry('%sx%s+%s+%s' % (self.gui_width, self.gui_height, self.gui_pos_x, self.gui_pos_y))
        else:
            self.geometry('%sx%s' % (geometry_full_width, geometry_default_height))
            self.gui_width = geometry_full_width
            self.gui_height = geometry_default_height

        if self.main_frame_new_status == 'SHOW':
            self.main_frame.place(x=0, y=0)
            pass
        else:
            self.hidden_frame.place(x=0, y=0)
            self.console.pack(fill='both')

        self.main_frame_actual_status = self.main_frame_new_status
        self.node_main_thread_id = threading.Thread(target=self.node_main_thread)
        self.node_main_thread_id.daemon = True

        self.serial_read_thread = threading.Thread(target=self.port_read_thread)
        self.serial_read_thread.daemon = True

        self.protocol("WM_DELETE_WINDOW", self.gui_closed_callback)
        self.resizable(False, True)

    def run(self):

        self.node_main_thread_id.start()
        self.serial_read_thread.start()

    def node_main_thread(self):
        '''
        This is the main Thread for a Node
        :return:
        '''

        #build command window layout for initial AT command selection
        self.update_window_layout(self.at_cmd_var.get())

        while self.thread_stop.is_set() is False:
            try:
                if self.show_full_view_req is not None:
                    if self.show_full_view_req is True:
                        self.hidden_frame.place_forget()
                        self.geometry('%sx%s' % (geometry_full_width, geometry_default_height))
                        self.main_frame.place(x=0, y=0)
                        self.main_frame_actual_status = 'SHOW'
                        self.main_frame_new_status = 'SHOW'
                    else:
                        # show the small view
                        self.main_frame.place_forget()
                        self.hidden_frame.place(x=0, y=0)
                        self.console.pack(fill='both')

                        if self.gui_height < geometry_default_height :
                            self.geometry('%sx%s' % (geometry_reduced_width, self.gui_height))
                        else:
                            self.geometry('%sx%s' % (geometry_reduced_width, geometry_reduced_height))

                        self.main_frame_actual_status = 'HIDE'
                        self.main_frame_new_status = 'HIDE'
                    self.show_full_view_req = None

                if self.main_frame_actual_status is not self.main_frame_new_status:
                    if self.main_frame_new_status == 'SHOW':
                        self.hidden_frame.place_forget()
                        self.geometry('%sx%s' % (geometry_full_width, self.winfo_height() if self.main_frame_actual_status is not None else self.gui_height))
                        self.main_frame.place(x=0, y=0)
                    else:
                        self.main_frame.place_forget()
                        self.hidden_frame.place(x=0, y=0)
                        self.console.pack(fill='both')
                        self.geometry('%sx%s' % (geometry_reduced_width, self.winfo_height()))

                    self.main_frame_actual_status = self.main_frame_new_status

                if self.serial_object is not None and self.serial_object.isOpen() is True:
                    if self.connection_button.cget("relief") != 'sunken':
                        self.connection_button.config(relief=SUNKEN, text="Disconnect")
                        self.port_menu.configure(state=DISABLED)
                else:
                    if self.connection_button.cget("relief") != 'raised':
                        self.connection_button.config(relief=RAISED, text="Connect")
                        self.port_menu.configure(state=NORMAL)

                if self.packet_decode_status is not False:
                    self.decode_button.configure(relief=SUNKEN)
                else:
                    self.decode_button.configure(relief=RAISED)

                if self.poll_ncu is not False:
                    self.poll_button.configure(relief=SUNKEN)
                else:
                    self.poll_button.configure(relief=RAISED)

                if self.time_stamp_status is not False:
                    self.timestamp_button.configure(relief=SUNKEN)
                else:
                    self.timestamp_button.configure(relief=RAISED)

                if self.unit_info_action is None:
                    if self.unit_info_var == UnitParameterActions.READ:
                        # Set the Flag
                        self.unit_info_read_in_progress = True

                        # Issue first AT command
                        self.unit_info_read_step = UnitParameterSteps.INIT

                        self.unit_info_action = 'READ'
                        # kick first request
                        self.unit_info_read_engine()
                    elif self.unit_info_var == UnitParameterActions.WRITE:
                        # Set the Flag
                        self.unit_info_write_in_progress = True

                        # Issue first AT command
                        self.unit_info_write_step = UnitParameterSteps.INIT

                        self.unit_info_action = 'WRITE'

                if self.unit_info_write_in_progress is True:
                    self.unit_info_write_engine()

                #if self.at_cmd_selected_type is not at_command_types[at_command_desc_list.index(self.at_cmd_var.get())]:
                    # There is a request from the user to change the window layout
                #    self.update_window_layout()

                if self.report_associated_node_state is not ReportAssociatedNodeState.INIT:
                    # Associated node report is in progress
                    self.report_associated_node_handler()

                if self.firmware_update_state is not FirmwareUpdateState.IDLE:
                    # Firmware update is active
                    self.firmware_update_handler()

                # Check timeouts
                if self.unit_info_read_in_progress is True:
                    if (datetime.now() - self.last_at_command_time).total_seconds() > 5:
                        self.unit_info_read_engine()
                        print('unit_info_read_engine timeout')

                if self.report_associated_node_state is not ReportAssociatedNodeState.INIT:
                    if (datetime.now() - self.report_assoc_node_time).total_seconds() > 5:
                        # self.report_associated_node_state = ReportAssociatedNodeState.ERROR
                        pass

                # Flush the log file every minute
                if (datetime.now() - self.log_flush_time).total_seconds() > 60:
                    if self.logging_file is not None:
                        self.logging_file.flush()
                        os.fsync(self.logging_file.fileno())
                        self.log_flush_time = datetime.now()

                if self.unit_name == 'PPU':
                    if self.ppu_refresh_status.get() != 0:
                        refresh_period = self.ppu_refresh_period.get()
                        if refresh_period.isdigit() and int(refresh_period) > 0:
                            if (datetime.now() - self.ppu_list_refresh_time).total_seconds() > int(self.ppu_refresh_period.get()):
                                self.ppu_list_refresh_time = datetime.now()
                                self.send_at_command('ATPPLST?')

                if len(self.event_queue):
                    new_event = self.event_queue.popleft()
                    if new_event == NodeEvents.PPU_ENABLE:
                        self.send_at_command('ATSTATE=1')
                    elif new_event == NodeEvents.PPU_DISABLE:
                        self.send_at_command('ATSTATE=0')
                    elif new_event == NodeEvents.PPU_REFRESH_LIST:
                        self.send_at_command('ATPPLST?')
                    elif new_event == NodeEvents.PPU_SET_FREQUENCY:
                        self.send_at_command('ATFREQ=' + self.ppu_frequency_var.get().split('-')[1].strip('Hz').strip())
                    elif new_event == NodeEvents.PPU_SET_SYSTEM_ID:
                        self.send_at_command('ATSYSID=' + self.ppu_system_id.get())
                    elif new_event == NodeEvents.BROADCAST_PP_REQUEST:
                        self.send_at_command('ATPPBST+')

                # Unlock the console for this round
                self.console.configure(state=NORMAL)

                while len(self.serial_rx_queue) is not 0:
                    new_item = self.serial_rx_queue.popleft()
                    time_stamp = new_item[0]
                    try:
                        new_message = new_item[1].decode("ASCII")
                    except UnicodeDecodeError:
                        # Probably Some non ASCII characters
                        new_message = ''.join([chr(i) if i < 128 else '@#@#' for i in new_item[1]])

                    # Strip any Formatting Characters from the left side of the line
                    new_message = new_message.lstrip('\n').lstrip('\r').rstrip('\n').rstrip('\r').lstrip().rstrip()


                    # Limit the number of lines on the console as this seems to consume RAM over a long test
                    current_idx = float(self.console.index(END))
                    if current_idx > console_max_line_nbr:
                        self.console.delete("1.0", current_idx-(console_max_line_nbr/2))

                    #Jump to a new line
                    self.console.insert(END, '\r\n')
                    if self.time_stamp_status is True:
                        if new_message:
                            # print the timestamp if the line contains some info
                            self.console.insert(END, time_stamp.split(" ")[1] + ' > ', "trivial_tag")

                    if new_message.startswith('+SYS:'):
                        self.console.insert(END, new_message, "dbg_sys_tag")
                    elif new_message.startswith('+INF:'):
                        self.console.insert(END, '+INF:', "trivial_tag")
                        self.console.insert(END, new_message.replace('+INF:', ''))
                    elif new_message.startswith('+BIT:'):
                        self.console.insert(END, new_message, "dbg_bit_tag")
                    elif new_message.startswith('+ERR:'):
                        self.console.insert(END, new_message, "dbg_error_tag")
                    else:
                        self.console.insert(END, new_message)

                    if self.unit_name != 'PPU':
                        if self.packet_decode_status or self.logging_file is not None:
                            direction, decoded_line = self.decode_and_process_line(new_message)

                    if self.packet_decode_status is not False:
                        if decoded_line is not None:
                            self.console.insert(END, '\n\r\t' + u'\u261e', "pointing_hand")
                            self.console.insert(END, direction + str(decoded_line) , "decoded_data")

                    if self.logging_file is not None:
                        self.logging_file.write('\n\r' + '[' + time_stamp + ']' + ' ' + new_message)
                        if decoded_line is not None:
                            self.logging_file.write('\n\r\t\t' + direction + str(decoded_line) )

                    # Move to the last line if possible and then lock the console
                    if self.console_freeze is not True:
                        self.console.see(END)
                    # Ask the RBU for its unit address if we don't know it.  Don't repeat the request for 5 seconds.
                    if self.unit_address is None:
                        if self.unit_address_request_time is None or ((datetime.now() - self.unit_address_request_time).total_seconds() > 5):
                            self.at_last_operation_type = ATCommandType.READ_ONLY
                            self.unit_address_request_time = datetime.now();
                            self.at_last_command = 'UA'
                            self.send_at_command('ATUA?')

                    # look for associated nodes debug info if in progress
                    if self.report_associated_node_state is not ReportAssociatedNodeState.INIT:
                        self.look_for_associated_nodes_data(new_message)


                    # The poll response from the NCU, listing how many messages are in each queue, comes back
                    # with line-feeds in it, so it occurs over 5 lines in the console.  The msn_count is used
                    # to count these lines as they are read and divert the code to handling the appropriate
                    # queue request.
                    if self.msn_count != 0:
                        self.request_queued_message_from_ncu(new_message)
                    elif (self.at_last_command is not None and new_message.startswith(self.at_last_command)) or self.unit_info_read_in_progress is True:
                        # Process any AT responses
                        self.look_for_at_responses(new_message)
#                    elif self.at_last_command == 'QMC':
#                            self.send_at_command('ATXMC+')
#                            self.at_last_command = 'XMC'
#                            self.at_last_operation_type = ATCommandType.READ_ONLY


                    if self.unit_name == 'PPU':
                        self.ppu_mode_process_line(new_message)

                    if self.firmware_update_state is not FirmwareUpdateState.IDLE:
                        # Hand this message to the
                        self.firmware_update_handler(new_message)


                #poll the NCU CIE queues every second ( every 10th cycle )
                if self.poll_ncu is True and self.unit_name == 'NCU':
                    self.ncu_poll_count += 1
                    if self.ncu_poll_count == 10:
                        self.ncu_poll_count = 0
                        self.send_at_command('ATMSN?')
                        self.at_last_command = 'MSN'
                        self.at_last_operation_type = ATCommandType.READ_ONLY

                # Lock back the console
                self.console.configure(state=DISABLED)

            except Exception as ex:
                print(ex)
                traceback.print_exc()

            time.sleep(0.1)

        if self.logging_file is not None:
            self.logging_file.close()

        if self.serial_object is not None and self.serial_object.isOpen() is True:
            self.serial_object_lock.clear()
            self.serial_object.cancel_read()
            self.serial_object.close()
        self.node_list.remove(self)
        self.destroy()

    def update_window_layout(self, value):
        self.at_cmd_selected_type = at_command_types[at_command_desc_list.index(value)]
        # Draw the AT command frame from scratch
        for child in self.at_command_frame.children.values():
            if child is not self.at_cmd_menu:
                child.place_forget()
        self.AT_command_rw_value.delete(0, END)

        if self.at_cmd_selected_type is None:
            self.raw_at_command_frame.place(x=2, y=50)
        else:
            if self.at_cmd_selected_type is ATCommandType.READ_WRITE:
                self.raw_at_command_frame.place(x=2, y=80)
                self.AT_command_r_button.place(x=187, y=10)
                self.AT_command_w_button.place(x=187, y=45)
                self.AT_command_rw_value.config(width=22)
                self.AT_command_rw_value.place(x=15, y=50)
            elif self.at_cmd_selected_type is ATCommandType.READ_ONLY:
                self.raw_at_command_frame.place(x=2, y=80)
                self.AT_command_r_button.place(x=187, y=10)
                self.AT_command_rw_value.config(width=35)
                self.AT_command_rw_value.place(x=15, y=50)
                self.AT_command_ctrl_button.place_forget()
            elif self.at_cmd_selected_type is ATCommandType.WRITE_ONLY:
                self.raw_at_command_frame.place(x=2, y=80)
                self.AT_command_w_button.place(x=187, y=10)
                self.AT_command_rw_value.config(width=22)
                self.AT_command_rw_value.place(x=15, y=50)
            elif self.at_cmd_selected_type is ATCommandType.SPECIAL:
                self.raw_at_command_frame.place(x=2, y=50)
                self.AT_command_ctrl_button.place(x=187, y=10)
            elif self.at_cmd_selected_type is ATCommandType.ATCMD:
                self.AT_command_r_button.place(x=187, y=10)
                self.AT_command_w_button.place(x=187, y=45)
                self.AT_cmd_transaction_id.place(x=15, y=42)
                self.AT_cmd_destination_id.place(x=70, y=45)
                self.AT_cmd_type.place(x=15, y=72)
                self.AT_cmd_1st_parameter.place(x=187, y=75)
                self.AT_cmd_2nd_parameter.place(x=230, y=75)
                self.AT_command_rw_value.config(width=35)
                self.AT_command_rw_value.place(x=15, y=110)
            elif self.at_cmd_selected_type is ATCommandType.ATOUT:
                self.AT_command_w_button.place(x=187, y=8)
                self.AT_cmd_destination_id.place(x=15, y=50)
                self.AT_output_profile.place(x=105, y=45)
                self.AT_output_zone.place(x=190, y=50)
                self.AT_command_rw_value.config(width=4)
                self.AT_command_rw_value.place(x=230, y=50)
                self.raw_at_command_frame.place(x=2, y=80)

    def report_associated_node_handler(self):
        if self.report_associated_node_state == ReportAssociatedNodeState.START:
            self.report_assoc_node_time = datetime.now()
            if self.unit_address is None and self.at_last_command is None:
                self.at_last_operation_type = ATCommandType.READ_ONLY
                self.at_last_command = 'UA'
                # Send the command
                self.send_at_command('ATUA?')
            else:
                self.report_associated_node_state = ReportAssociatedNodeState.GET_NODES
        elif self.report_associated_node_state == ReportAssociatedNodeState.GET_NODES:
            if self.unit_address is not None:
                self.number_of_parents = -1
                self.number_of_tracking_nodes = -1
                self.number_of_children = -1
                self.mesh_parents = [(-1, -1), (-1, -1)]
                self.tracking_nodes = [(-1, -1), (-1, -1)]
                self.mesh_children.clear()
                self.report_associated_node_state = ReportAssociatedNodeState.PROCESS
                self.at_last_command = "NODES"
                self.at_last_operation_type = ATCommandType.SPECIAL
                # Send the command
                self.send_at_command('AT' + self.at_last_command + '+')
        elif self.report_associated_node_state == ReportAssociatedNodeState.PROCESS:
            # TODO: Add Timeout
            pass

    def firmware_update_handler(self, new_message=None):
        if new_message is not None:
            if self.firmware_update_state == FirmwareUpdateState.WAIT_FOR_BOOT_MODE:
                if 'BOOTLOADER SEED' in new_message:
                    if self.firmware_update_unit_rescue:
                        self.serial_object.write(b"0x11223344\0")
                elif 'Waiting for the Application Image' in new_message:
                    self.serial_object_lock.clear()
                    self.serial_object.cancel_read()
                    self.firmware_update_state = FirmwareUpdateState.PROGRAMMING
            elif self.firmware_update_state == FirmwareUpdateState.WAIT_FOR_BOOTLOADER_END:
                # TODO: Explore results later
                if 'BOOTLOADER: Serial Firmware Update Successful' in new_message:
                    if self.firmware_update_unit_rescue:
                        # In rescue mode, the bootloader switches on its own to the other bank
                        self.wait_for_app_time = datetime.now()
                        self.firmware_update_state = FirmwareUpdateState.WAIT_FOR_NEW_APP
                    else:
                        # Programing was successful, move to the check state
                        self.wait_for_app_time = datetime.now()
                        self.firmware_update_state = FirmwareUpdateState.WAIT_FOR_APP
                elif 'BOOTLOADER: Serial Firmware Update Failed' in new_message:
                    # Programming failed
                    self.firmware_update_state = FirmwareUpdateState.FAILED
        else:
            # This is some sort of the state machine of the firmware update operation
            if self.firmware_update_state == FirmwareUpdateState.LEAVE:
                # self.frame_fw_up.place_forget()

                self.firmware_update_state = FirmwareUpdateState.IDLE
            elif self.firmware_update_state == FirmwareUpdateState.START:
                """
                self.frame_fw_up.place(x=2, y=260)
    
                self.progress_bar["value"] = 0
                self.progress_bar["maximum"] = 249
                """
                # self.progress_bar.configure(style="ProgSuccess")
                self.at_last_command = 'ATBOOT'
                self.at_last_operation_type = ATCommandType.SPECIAL
                self.send_at_command('ATBOOT+')
                self.firmware_update_state = FirmwareUpdateState.WAIT_FOR_BOOT_MODE
            elif self.firmware_update_state == FirmwareUpdateState.WAIT_FOR_BOOT_MODE:
                pass
            elif self.firmware_update_state == FirmwareUpdateState.PROGRAMMING:
                self.reset_detected = False
                self.serial_object.flush()
                self.serial_object.reset_input_buffer()
                self.serial_object.reset_output_buffer()
                self.serial_object.timeout = 10
                res = self.modem.send([self.firmware_file], callback=self.firmware_update_progress)
                self.serial_object.timeout = None
                # print('\n' + self.unit_name + ' Returned from YMODEM\n')
                self.serial_object_lock.set()

                if res is True:
                    # Ymodem was happy, wait for the result
                    self.firmware_update_state = FirmwareUpdateState.WAIT_FOR_BOOTLOADER_END
                else:
                    # Programming failed
                    self.firmware_update_state = FirmwareUpdateState.FAILED
            elif self.firmware_update_state == FirmwareUpdateState.WAIT_FOR_BOOTLOADER_END:
                pass
            elif self.firmware_update_state == FirmwareUpdateState.WAIT_FOR_APP:
                if self.reset_detected or (datetime.now() - self.wait_for_app_time).total_seconds() > 5:
                    self.firmware_update_state = FirmwareUpdateState.GET_ACTIVE_BANK
                    self.fw_active_bank = None
                    self.at_last_command = 'FA'
                    self.at_last_operation_type = ATCommandType.READ_ONLY
                    self.send_at_command('ATFA?')
            elif self.firmware_update_state == FirmwareUpdateState.GET_ACTIVE_BANK:
                if self.fw_active_bank is not None:
                    self.wait_for_app_time = datetime.now()
                    self.at_last_command = 'FA'
                    self.at_last_operation_type = ATCommandType.WRITE_ONLY
                    self.send_at_command('ATFA=%d' % (2 if self.fw_active_bank is 1 else 1))
                    self.firmware_update_state = FirmwareUpdateState.WAIT_FOR_NEW_APP
            elif self.firmware_update_state == FirmwareUpdateState.WAIT_FOR_NEW_APP:
                if self.reset_detected or (datetime.now() - self.wait_for_app_time).total_seconds() > 5:
                    self.firmware_version_nbr = None
                    self.firmware_version_date = None
                    self.firmware_checksum = None
                    self.at_last_command = 'FI'
                    self.at_last_operation_type = ATCommandType.READ_ONLY
                    self.send_at_command('ATFI?')
                    self.firmware_update_state = FirmwareUpdateState.WAIT_FOR_NEW_APP_INFO
            elif self.firmware_update_state == FirmwareUpdateState.WAIT_FOR_NEW_APP_INFO:
                if self.firmware_version_nbr is not None \
                        and self.firmware_version_date is not None \
                        and self.firmware_checksum is not None:
                    self.firmware_update_state = FirmwareUpdateState.COMPLETED

    def firmware_update_progress(self, total_packets, success_count, error_count):
        # self.progress_bar["value"] = success_count
        """
        if error_count:
            self.progress_bar.configure(style="ProgFailure")
        """
    def connection_control(self):
        baud = self.baud_entry.get()

        try:
            port_menu = self.port_menu['menu']
            port_menu.delete(0, 'end')

            serial_ports_list = serial_ports()

            for port in serial_ports_list:
                port_menu.add_command(label=port,command=lambda value=port: self.port_var.set(value))

            if self.serial_object is not None and self.serial_object.isOpen() is True:
                self.serial_object_lock.clear()
                self.serial_object.cancel_read()
                self.serial_object.flush()
                self.serial_object.reset_input_buffer()
                self.serial_object.reset_output_buffer()
                self.serial_object.close()
                self.serial_object = None
                if self.logging_file is not None:
                    self.logging_file.close()
                    self.logging_file = None
            else:
                self.serial_object = serial.Serial(self.port_var.get(), baud)
                if self.serial_object is not None:
                    self.serial_object.flush()
                    self.serial_object.reset_input_buffer()
                    self.serial_object.reset_output_buffer()
                    self.serial_object_lock.set()

                """
                handle = w32.OpenThread(0x20, False, self.serial_read_thread.daemon)  # THREAD_SET_INFORMATION = 0x20
                result = w32.SetThreadPriority(handle, 1)  # THREAD_PRIORITY_ABOVE_NORMAL = 1
                w32.CloseHandle(handle)
                if not result:
                    print('Failed to set priority of thread' + str(w32.GetLastError()))
                """
        except (ValueError, serial.SerialException) as ex:
            print(ex)
            traceback.print_exc()

    def packet_decode_control(self):
        self.packet_decode_status = not self.packet_decode_status

    def time_stamp_control(self):
        self.time_stamp_status = not self.time_stamp_status

    def unit_poll_operation(self):
        self.poll_ncu = not self.poll_ncu
        if self.poll_ncu == True:
            self.cie_queue_menu.configure(state=DISABLED)
            self.read_cie_queue_button.configure(state=DISABLED)
            self.clear_cie_queue_button.configure(state=DISABLED)
        else:
            self.cie_queue_menu.configure(state=ACTIVE)
            self.read_cie_queue_button.configure(state=ACTIVE)
            self.clear_cie_queue_button.configure(state=ACTIVE)


    def reset_cie_queues_operation(self):
        self.send_at_command('ATXMQ=1,1\n1,1\n1,1\n1,1\n1')
        self.at_last_command = 'XMQ'

    def read_queue_operation(self):
        if self.cie_queue_var.get() == 'MISC':
            self.send_at_command('ATQMC?')
            self.at_last_command = 'QMC'
        elif self.cie_queue_var.get() == 'FIRE':
            self.send_at_command('ATQFE?')
            self.at_last_command = 'QFE'
        elif self.cie_queue_var.get() == 'FIRST AID':
            self.send_at_command('ATQAM?')
            self.at_last_command = 'QAM'
        elif self.cie_queue_var.get() == 'FAULT':
            self.send_at_command('ATQFT?')
            self.at_last_command = 'QFT'

    def clear_queue_operation(self):
        if self.cie_queue_var.get() == 'MISC':
            self.send_at_command('ATXMQ=0,0\n0,0\n0,0\n1,0\n0')
            self.at_last_command = 'XMQ'
        elif self.cie_queue_var.get() == 'FIRE':
            self.send_at_command('ATXMQ=1,0\n0,0\n0,0\n0,0\n0')
            self.at_last_command = 'XMQ'
        elif self.cie_queue_var.get() == 'FIRST AID':
            self.send_at_command('ATXMQ=0,0\n1,0\n0,0\n0,0\n0')
            self.at_last_command = 'XMQ'
        elif self.cie_queue_var.get() == 'FAULT':
            self.send_at_command('ATXMQ=0,0\n0,0\n1,0\n0,0\n0')
            self.at_last_command = 'XMQ'
        elif self.cie_queue_var.get() == 'FIRE LOST':
            self.send_at_command('ATXMQ=0,1\n0,0\n0,0\n0,0\n0')
            self.at_last_command = 'XMQ'
        elif self.cie_queue_var.get() == 'FIRST AID LOST':
            self.send_at_command('ATXMQ=0,0\n0,1\n0,0\n0,0\n0')
            self.at_last_command = 'XMQ'
        elif self.cie_queue_var.get() == 'FAULT LOST':
            self.send_at_command('ATXMQ=0,0\n0,0\n0,1\n0,0\n0')
            self.at_last_command = 'XMQ'
        elif self.cie_queue_var.get() == 'MISC LOST':
            self.send_at_command('ATXMQ=0,0\n0,0\n0,0\n0,1\n0')
            self.at_last_command = 'XMQ'
        elif self.cie_queue_var.get() == 'TxBUFFER LOST':
            self.send_at_command('ATXMQ=0,0\n0,0\n0,0\n0,0\n1')
            self.at_last_command = 'XMQ'

    def cie_queue_changed(self, value):
        if value == 'MISC' or value == 'FIRE' or value == 'FIRST AID' or value == 'FAULT':
            self.read_cie_queue_button.configure(state=ACTIVE)
        else:
            self.read_cie_queue_button.configure(state=DISABLED)

    def auto_alarm_operation(self):
        self.auto_alarm_enable = not self.auto_alarm_enable

    def port_read_thread(self):
        while True:
            try:
                self.serial_object_lock.wait()
                new_line = self.serial_object.readline()
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
                self.serial_rx_queue.append([timestamp, new_line])
            except (ValueError, serial.SerialException) as ex:
                print(ex)
                time.sleep(1)

    def send_at_command(self, at_command):

        if at_command is not None and self.serial_object is not None and self.serial_object.isOpen() is True:
            self.serial_object.write("+++".encode("ASCII") + '\r\n'.encode("ASCII"))
            time.sleep(0.2)
            self.serial_object.write("+++".encode("ASCII") + '\r\n'.encode("ASCII"))

            self.serial_object.write(at_command.encode("ASCII") + '\r\n'.encode("ASCII"))
            self.last_at_command_time = datetime.now()

    def send_at_command_callback(self):
        self.send_at_command(self.command_entry.get())

    def AT_command_control(self):
        try:
            selected_cmd = self.at_cmd_var.get()
            if selected_cmd is not None:
                if self.serial_object is not None and self.serial_object.isOpen() is True:
                    # save the AT command we have just sent
                    cmd_idx = at_command_desc_list.index(selected_cmd)
                    self.at_last_command = at_command_list[cmd_idx]
                    self.at_last_operation_type = ATCommandType.SPECIAL
                    self.send_at_command('AT' + self.at_last_command + '+')

        except (ValueError, serial.SerialException):
            pass

    def AT_command_read(self):
        selected_cmd = self.at_cmd_var.get()
        if selected_cmd is not None:
            if self.serial_object is not None and self.serial_object.isOpen() is True:
                # save the AT command we have just sent
                cmd_idx = at_command_desc_list.index(selected_cmd)
                self.at_last_command = at_command_list[cmd_idx]
                self.at_last_operation_type = ATCommandType.READ_ONLY
                if self.at_last_command is 'CMD':
                    # Increment Transaction ID for next time
                    self.AT_cmd_transaction_id_var.set((self.AT_cmd_transaction_id_var.get()+1) % 8)

                    self.send_at_command('AT' + self.at_last_command
                                         + '?'
                                         + str(self.AT_cmd_transaction_id_var.get())
                                         + ','
                                         + self.AT_cmd_destination_id.get()
                                         + ','
                                         + str(getattr(ParameterType, self.AT_cmd_type_var.get()).value)
                                         + ','
                                         + self.AT_cmd_1st_parameter.get()
                                         + ','
                                         + self.AT_cmd_2nd_parameter.get())
                else:
                    # Send the command
                    self.send_at_command('AT' + self.at_last_command + '?')

    def AT_command_write(self):
        selected_cmd = self.at_cmd_var.get()
        new_value = self.AT_command_rw_value.get()
        if selected_cmd is not None and new_value is not None:
            if self.serial_object is not None and self.serial_object.isOpen() is True:
                # save the AT command we have just sent
                cmd_idx = at_command_desc_list.index(selected_cmd)
                self.at_last_command = at_command_list[cmd_idx]
                self.at_last_operation_type = ATCommandType.WRITE_ONLY
                if self.at_last_command is 'CMD':
                    # Increment Transaction ID for next time
                    self.AT_cmd_transaction_id_var.set((self.AT_cmd_transaction_id_var.get()+1) % 8)

                    self.send_at_command('AT' + self.at_last_command
                                         + '='
                                         + str(self.AT_cmd_transaction_id_var.get())
                                         + ','
                                         + self.AT_cmd_destination_id.get()
                                         + ','
                                         + str(getattr(ParameterType, self.AT_cmd_type_var.get()).value)
                                         + ','
                                         + self.AT_cmd_1st_parameter.get()
                                         + ','
                                         + self.AT_cmd_2nd_parameter.get()
                                         + ','
                                         + self.AT_command_rw_value.get())
                elif self.at_last_command is 'OUT':
                    self.send_at_command('AT' + self.at_last_command
                                         + '='
                                         + self.AT_cmd_destination_id.get()
                                         + ','
                                         + self.AT_output_zone.get()
                                         + ','
                                         + str(getattr(OutputProfile, self.AT_output_profile_var.get()).value)
                                         + ','
                                         + self.AT_command_rw_value.get())
                else:
                    # Send the command
                    self.send_at_command('AT' + self.at_last_command + '=' + new_value)

    def unit_reset_operation(self):
        try:
            if self.serial_object is not None and self.serial_object.isOpen() is True:
                self.send_at_command('ATRRU+Z4095U' + str(self.unit_address))
        except (ValueError, serial.SerialException):
            pass

    def set_mesh_state(self, state):
        try:
            if self.serial_object is not None and self.serial_object.isOpen() is True:
                self.at_last_command = None
                self.at_last_operation_type = None
                self.send_at_command('ATSTATE' + '=' + str(state))
        except (ValueError, serial.SerialException):
            pass

    def clear_console(self):
        self.console.configure(state=NORMAL)
        self.console.delete('1.0', END)
        self.console.configure(state=DISABLED)

    def report_associated_nodes(self):
        self.report_associated_node_state = ReportAssociatedNodeState.START

    def look_for_associated_nodes_data(self, new_line):
        if new_line.lstrip().startswith('+DAT:'):
            cut_line = new_line.lstrip().lstrip('+DAT:').rstrip()
            parts = cut_line.split(' ')
            sub_parts = parts[0].split(',')
            if int(sub_parts[0]) == MeshEvent.REPORT_NODES.value:
                if len(parts) >= 5:  # command,rank ID,SNR ID, SNR ID,SNR ID, SNR
                    error = False

                    if len(sub_parts) == 2:
                        self.mesh_rank = int(sub_parts[1])
                    else:
                        error = True

                    # Get Primary Parent
                    sub_parts = parts[1].split(',')
                    if len(sub_parts) == 2:
                        self.mesh_parents[0] = (int(sub_parts[0]), int(sub_parts[1]))
                    else:
                        error = True

                    # Get Secondary Parent
                    sub_parts = parts[2].split(',')
                    if len(sub_parts) == 2:
                        self.mesh_parents[1] = (int(sub_parts[0]), int(sub_parts[1]))
                    else:
                        error = True

                    # Get Primary Tracking node
                    sub_parts = parts[3].split(',')
                    if len(sub_parts) == 2:
                        self.tracking_nodes[0] = (int(sub_parts[0]), int(sub_parts[1]))
                    else:
                        error = True

                    # Get Secondary Tracking node
                    sub_parts = parts[4].split(',')
                    if len(sub_parts) == 2:
                        self.tracking_nodes[1] = (int(sub_parts[0]), int(sub_parts[1]))
                    else:
                        error = True

                    for i in range(5, len(parts)):

                        sub_parts = parts[i].split(',')
                        if len(sub_parts) == 2:
                            self.mesh_children.append((int(sub_parts[0]), int(sub_parts[1])))
                        else:
                            error = True

                    if error is False:
                        self.report_associated_node_state = ReportAssociatedNodeState.DONE
                    else:
                        self.report_associated_node_state = ReportAssociatedNodeState.ERROR

                else:
                    self.report_associated_node_state = ReportAssociatedNodeState.ERROR

    def look_for_at_responses(self, new_line):

        # check for the AT command response info
        if self.at_last_operation_type == ATCommandType.READ_ONLY:
            values = new_line.strip(' ').split(": ")
            if len(values) > 1:
                # We have something
                if self.at_last_command == 'UA' and values[0] == 'UA':
                    self.unit_address = values[1]
                    self.at_last_command = None
                elif self.at_last_command == 'FA' and values[0] == 'FA':
                    if values[1] is '1' or '2':
                        self.fw_active_bank = int(values[1])
                        self.at_last_command = None
                elif self.at_last_command == 'FI' and values[0] == 'FI':
                    sw_info = values[1].split(",")
                    self.firmware_version_nbr = sw_info[0]
                    self.firmware_version_date = sw_info[1]
                    self.firmware_checksum = sw_info[2]
                    self.at_last_command = None
                elif self.at_last_command == 'MSN' and values[0] == 'MSN' and values[1] != 'ERROR':
                    #NCU message queue poll response
                    self.request_queued_message_from_ncu(values[1])
                elif self.at_last_command == 'QFE' and values[0] == 'QFE' and values[1] != 'ERROR':
                    #self.send_at_command('ATXFE+')
                    if self.auto_alarm_enable is True:
                        outputs = values[1].split(',')
                        if outputs[2] == '1':
                            self.send_at_command('ATAOS+1,1,0')
                            #self.send_output_signal(OutputProfile.FIRE, OutputsActivated.DEACTIVATE)
                        else:
                            self.send_at_command('ATAOS+0,0,0')
                            #self.send_output_signal(OutputProfile.FIRE, OutputsActivated.ACTIVATE)
                elif self.at_last_command == 'QAM' and values[0] == 'QAM' and values[1] != 'ERROR':
                    #self.send_at_command('ATXAM+')
                    if self.auto_alarm_enable is True:
                        outputs = values[1].split(',')
                        if outputs[2] == '1':
                            self.send_at_command('ATAOS+2,2,0')
                            #self.send_output_signal(OutputProfile.FIRST_AID, OutputsActivated.DEACTIVATE)
                        else:
                            self.send_at_command('ATAOS+0,0,0')
                            #self.send_output_signal(OutputProfile.FIRST_AID, OutputsActivated.ACTIVATE)
                    self.at_last_command = None
                elif self.at_last_command == 'QFT' and values[0] == 'QFT' and values[1] != 'ERROR':
                    #self.send_at_command('ATXFT+')
                    self.at_last_command = None
                elif self.at_last_command == 'QMC' and values[1] != 'ERROR':
                    #self.send_at_command('ATXMC+')
                    self.at_last_command = None


                if self.at_cmd_selected_type is not None:
                    selected_cmd = at_command_list[at_command_desc_list.index(self.at_cmd_var.get())]
                    if selected_cmd == values[0]:
                        value = values[1]
                        if selected_cmd == 'CMD' and self.AT_cmd_type_var.get() in('RBU_SERIAL_NUMBER', 'PLUGIN_SERIAL_NUMBER'):
                            if value.isdigit():
                                value = decode_serial_number(value)

                        self.AT_command_rw_value.delete(0, END)
                        self.AT_command_rw_value.insert(0, value)
                        self.at_last_command = None

                if self.unit_info_read_in_progress is True:
                    self.unit_info_read_engine(values[1])

    def gui_closed_callback(self):
        self.thread_stop.set()

    def expand_main_frame(self):
        self.main_frame_new_status = 'SHOW'

    def show_full_view(self):
        self.show_full_view_req = True

    def show_small_view(self):
        self.show_full_view_req = False

    def collapse_main_frame(self):
        self.main_frame_new_status = 'HIDE'

    def unit_info_read_engine(self, value=None):
        self.at_last_operation_type = ATCommandType.READ_ONLY

        if self.unit_info_read_step is UnitParameterSteps.INIT:
            self.unit_info_read_step = UnitParameterSteps.ADDRESS
            self.at_last_command = 'UA'
            self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.ADDRESS:
            if value is None:
                # Retry
                self.at_last_command = 'UA'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.Address = value
                self.unit_info_read_step = UnitParameterSteps.ZONE
                self.at_last_command = 'ZONE'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.ZONE:
            if value is None:
                # Retry
                self.at_last_command = 'ZONE'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.Zone = value
                self.unit_info_read_step = UnitParameterSteps.SYNC_FLAG
                self.at_last_command = 'SYNC'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.SYNC_FLAG:
            if value is None:
                # Retry
                self.at_last_command = 'SYNC'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.SyncMaster = value
                self.unit_info_read_step = UnitParameterSteps.FREQUENCY
                self.at_last_command = 'FREQ'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.FREQUENCY:
            if value is None:
                # Retry
                self.at_last_command = 'FREQ'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.FrequencyChannel = value
                self.unit_info_read_step = UnitParameterSteps.DEV_CFG
                self.at_last_command = 'DEVCF'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.DEV_CFG:
            if value is None:
                # Retry
                self.at_last_command = 'DEVCF'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.DeviceCombination = value
                self.unit_info_read_step = UnitParameterSteps.SYS_ID
                self.at_last_command = 'SYSID'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.SYS_ID:
            if value is None:
                # Retry
                self.at_last_command = 'SYSID'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.SystemId = value
                self.unit_info_read_step = UnitParameterSteps.SERIAL_NBR
                self.at_last_command = 'SERNO'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.SERIAL_NBR:
            if value is None:
                # Retry
                self.at_last_command = 'SERNO'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.SerialNumber = value
                self.unit_info_read_step = UnitParameterSteps.TX_LOW_POWER
                self.at_last_command = 'TXPLO'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.TX_LOW_POWER:
            if value is None:
                # Retry
                self.at_last_command = 'TXPLO'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.TxLowPower = value
                self.unit_info_read_step = UnitParameterSteps.TX_HIGH_POWER
                self.at_last_command = 'TXPHI'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.TX_HIGH_POWER:
            if value is None:
                # Retry
                self.at_last_command = 'TXPHI'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.TxHighPower = value
                self.unit_info_read_step = UnitParameterSteps.BOOT_USE_PPU
                self.at_last_command = 'BPPU'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.BOOT_USE_PPU:
            if value is None:
                # Retry
                self.at_last_command = 'BPPU'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.BootUsePPU = value
                self.unit_info_read_step = UnitParameterSteps.PP_MODE_ENABLE
                self.at_last_command = 'PPEN'
                self.send_at_command('AT' + self.at_last_command + '?')
        elif self.unit_info_read_step is UnitParameterSteps.PP_MODE_ENABLE:
            if value is None:
                # Retry
                self.at_last_command = 'PPEN'
                self.send_at_command('AT' + self.at_last_command + '?')
            else:
                self.parameters.PpModeEnable = value
                self.parameters.UnitName = self.unit_name
                self.unit_info_read_in_progress = False
                self.unit_info_read_step = UnitParameterSteps.DONE
                self.at_last_command = None
                self.at_last_operation_type = None

    def unit_info_write_engine(self):
        self.at_last_operation_type = ATCommandType.WRITE_ONLY

        if self.unit_info_write_step is UnitParameterSteps.INIT:
            self.unit_info_write_step = UnitParameterSteps.ADDRESS
        elif self.unit_info_write_step is UnitParameterSteps.ADDRESS:
            self.send_at_command('ATUA=' + str(self.parameters.Address))
            self.unit_info_write_step = UnitParameterSteps.ZONE
        elif self.unit_info_write_step is UnitParameterSteps.ZONE:
            self.send_at_command('ATZONE=' + str(self.parameters.Zone))
            self.unit_info_write_step = UnitParameterSteps.SYNC_FLAG
        elif self.unit_info_write_step is UnitParameterSteps.SYNC_FLAG:
            self.send_at_command('ATSYNC=' + str(self.parameters.SyncMaster))
            self.unit_info_write_step = UnitParameterSteps.FREQUENCY
        elif self.unit_info_write_step is UnitParameterSteps.FREQUENCY:
            self.send_at_command('ATFREQ=' + str(self.parameters.FrequencyChannel))
            self.unit_info_write_step = UnitParameterSteps.DEV_CFG
        elif self.unit_info_write_step is UnitParameterSteps.DEV_CFG:
            self.send_at_command('ATDEVCF=' + str(self.parameters.DeviceCombination))
            self.unit_info_write_step = UnitParameterSteps.SYS_ID
        elif self.unit_info_write_step is UnitParameterSteps.SYS_ID:
            self.send_at_command('ATSYSID=' + str(self.parameters.SystemId))
            self.unit_info_write_step = UnitParameterSteps.SERIAL_NBR
        elif self.unit_info_write_step is UnitParameterSteps.SERIAL_NBR:
            self.send_at_command('ATSERNO=' + str(self.parameters.SerialNumber))
            self.unit_info_write_step = UnitParameterSteps.TX_LOW_POWER
        elif self.unit_info_write_step is UnitParameterSteps.TX_LOW_POWER:
            self.send_at_command('ATTXPLO=' + str(self.parameters.TxLowPower))
            self.unit_info_write_step = UnitParameterSteps.TX_HIGH_POWER
        elif self.unit_info_write_step is UnitParameterSteps.TX_HIGH_POWER:
            self.send_at_command('ATTXPHI=' + str(self.parameters.TxHighPower))
            self.unit_info_write_step = UnitParameterSteps.BOOT_USE_PPU
        elif self.unit_info_write_step is UnitParameterSteps.BOOT_USE_PPU:
            self.send_at_command('ATBPPU=' + str(self.parameters.BootUsePPU))
            self.unit_info_write_step = UnitParameterSteps.PP_MODE_ENABLE
        elif self.unit_info_write_step is UnitParameterSteps.PP_MODE_ENABLE:
            self.send_at_command('ATPPEN=' + str(self.parameters.PpModeEnable))
            self.unit_info_write_in_progress = False
            self.unit_info_write_step = UnitParameterSteps.DONE
            self.at_last_operation_type = None

        time.sleep(0.5)

    def serial_getc(self, size, timeout=1):
        received = self.serial_object.read(size)
        return received or None

    def serial_putc(self, data, timeout=1):
        return self.serial_object.write(data)

    def firmware_update(self, firmware_file, rescue_mode):
        self.firmware_update_unit_rescue = rescue_mode
        self.firmware_file = firmware_file
        self.firmware_update_state = FirmwareUpdateState.START

    def console_yview(self, *args):
        if self.console_scrollbar.get()[1] < 0.95:
            self.console_freeze = True
        else:
            self.console_freeze = False
        self.console.yview(*args)

    def ppu_unit_selection(self, pos):
        self.AT_cmd_destination_id.delete(0, END)
        self.AT_cmd_destination_id.insert(END, encode_serial_number(self.ppu_device_list.selection_get()))

    def ppu_enable(self):
        if self.ppu_enable_button.config('relief')[-1] == 'sunken':
            self.ppu_enable_button.config(relief="raised", text="ENABLE")
            self.event_queue.append(NodeEvents.PPU_DISABLE)
        else:
            self.ppu_enable_button.config(relief="sunken", text="DISABLE")
            self.event_queue.append(NodeEvents.PPU_ENABLE)

    def ppu_set_freq(self):
        self.event_queue.append(NodeEvents.PPU_SET_FREQUENCY)

    def ppu_set_system_id(self):
        self.event_queue.append(NodeEvents.PPU_SET_SYSTEM_ID)

    def broadcast_pp_request(self):
        self.event_queue.append(NodeEvents.BROADCAST_PP_REQUEST)

    def ppu_refresh_list(self):
        self.event_queue.append(NodeEvents.PPU_REFRESH_LIST)

    def decode_and_process_line(self, line):
        # log fields
        DIRECTION_FIELD = 3
        DATA_FIELD = 8
        decoded_line = None
        direction = None

        try:
            m = mesh_data_pattern.search(line)
            if m:
                direction = m.group(DIRECTION_FIELD)
                data_in = m.group(DATA_FIELD)
                if data_in:
                    decoded_line = PY_MC_PUP.packet_decode(packet=data_in)
                    if decoded_line is not None:
                        if 'FrameType' in decoded_line:
                            frame_type = FrameType(decoded_line['FrameType'])
                            if frame_type is FrameType.HEARTBEAT and direction is 't':
                                replace_txt(self.children_nbr_box, decoded_line['NoOfChildrenIdx'])
                                replace_txt(self.long_frame_idx_box, decoded_line['LongFrameIdx'])
                                replace_txt(self.rank_box, decoded_line['Rank'])
                                replace_txt(self.state_box, decoded_line['State'])

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

        except Exception as ex:
            self.console.insert(END, "Failed to decode message\r\n")
            #messagebox.showinfo("decode_line", str(ex) + '\ndata =' + data_in)

        return direction, decoded_line

    def ppu_mode_process_line(self, line):
        if line.startswith('PPLST:'):
            values = [item.strip() for item in line.strip('\n').strip('\r').split(":")]
            if values[1].startswith('0x'):
                self.ppu_device_list.delete(0, END)
                devices = [decode_serial_number(x) for x in values[1].split(',') if ishex(x)]
                for device in devices:
                    self.ppu_device_list.insert(END, device)
            elif not len(values[1]):
                self.ppu_device_list.delete(0, END)

    def request_queued_message_from_ncu(self, values):
        ncu_signals = values.split(",")
        if ncu_signals[0] is not None and ncu_signals[0] != 'ERROR':
            if self.msn_count == 0:
                if ncu_signals[0] != '0':
                    #there is a fire signal queued
                    self.send_at_command('ATQFE?')
                    self.at_last_command = 'QFE'
                    self.at_last_operation_type = ATCommandType.READ_ONLY
                    self.msn_count = 0
                else:
                    self.msn_count +=1
            elif self.msn_count == 1:
                if ncu_signals[0] != '0':
                    # there is an alarm signal queued
                    self.send_at_command('ATQAM?')
                    self.at_last_command = 'QAM'
                    self.at_last_operation_type = ATCommandType.READ_ONLY
                    self.msn_count = 0
                else:
                    self.msn_count += 1
            elif self.msn_count == 2:
                if ncu_signals[0] != '0':
                    # there is an alarm signal queued
                    self.send_at_command('ATQFT?')
                    self.at_last_command = 'QFT'
                    self.at_last_operation_type = ATCommandType.READ_ONLY
                    self.msn_count = 0
                else:
                    self.msn_count += 1
            elif self.msn_count == 3:
                if ncu_signals[0] != '0':
                    # there is a misc signal queued
                    self.send_at_command('ATQMC?')
                    self.at_last_command = 'QMC'
                    self.at_last_operation_type = ATCommandType.READ_ONLY
                    self.msn_count = 0
                else:
                    self.msn_count += 1
            else:
                self.ncu_tx_buffer_count = ncu_signals[0]
                self.msn_count = 0
        else:
            self.msn_count = 0

    def send_output_signal(self, output_profile, outputs_activated):
        op_command = 'ATOUT+Z4095U4095,' + str(output_profile.value) + ',' + str(outputs_activated.value)
        self.send_at_command(op_command)


    @staticmethod
    def replace_txt(box, txt):
        box.configure(state=NORMAL)
        box.delete('1.0', END)
        box.insert(INSERT, txt)
        box.configure(state=DISABLED)
