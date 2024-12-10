"""

"""
import os
import sys
import traceback
import string
from tkinter import Frame, Scrollbar, Canvas, Toplevel, Label
from tkinter.constants import *
from enum import Enum
from serial.tools import list_ports

# This list contains a reference to all the RBU/NCU/PPU node objects
nodes = []

node_parameters = [
                    'SyncMaster',
                    'Address',
                    'Zone',
                    'FrequencyChannel',
                    'DeviceCombination',
                    'SystemId',
                    'SerialNumber',
                    'TxLowPower',
                    'TxHighPower',
                    'PpModeEnable',
                    'BootUsePPU'
]


class ATCommandType(Enum):
    READ_ONLY = 0
    WRITE_ONLY = 1
    READ_WRITE = 2
    SPECIAL = 3
    ATCMD = 4
    ATOUT = 5

class ReportAssociatedNodeState(Enum):
    INIT = 0
    START = 1
    GET_NODES = 2
    PROCESS = 3
    DONE = 4
    ERROR = 5


class UnitParameterActions(Enum):
    INIT = 0
    READ = 1
    WRITE = 2


class UnitParameterSteps(Enum):
    INIT = 0
    ADDRESS = 1
    ZONE = 2
    SYNC_FLAG = 3
    FREQUENCY = 4
    DEV_CFG = 5
    SYS_ID = 6
    SERIAL_NBR = 7
    TX_LOW_POWER = 8
    TX_HIGH_POWER = 9
    BOOT_USE_PPU = 10
    PP_MODE_ENABLE = 11
    DONE = 12


class FirmwareUpdateState(Enum):
    IDLE = 0
    START = 1
    WAIT_FOR_BOOT_MODE = 2
    PROGRAMMING = 3
    WAIT_FOR_BOOTLOADER_END = 4
    WAIT_FOR_APP = 5
    GET_ACTIVE_BANK = 6
    WAIT_FOR_NEW_APP = 7
    WAIT_FOR_NEW_APP_INFO = 8
    LEAVE = 9
    COMPLETED = 10
    FAILED = 11
    SUCCESS = 12


class NodeEvents(Enum):
    PPU_ENABLE = 0
    PPU_DISABLE = 1
    PPU_REFRESH_LIST = 2
    PPU_SET_FREQUENCY = 3
    PPU_SET_SYSTEM_ID = 4
    BROADCAST_PP_REQUEST = 5


AT_COMMAND = [
    ['System State',            'STATE',   ATCommandType.READ_WRITE],
    ['Unit Address',            'UA',      ATCommandType.READ_WRITE],
    ['Unit Zone',               'ZONE',    ATCommandType.READ_WRITE],
    ['Sync Flag',               'SYNC',    ATCommandType.READ_WRITE],
    ['Frequency',               'FREQ',    ATCommandType.READ_WRITE],
    ['Device Combination',      'DEVCF',   ATCommandType.READ_WRITE],
    ['System ID',               'SYSID',   ATCommandType.READ_WRITE],
    ['Serial Number',           'SERNO',   ATCommandType.READ_WRITE],
    ['Use PPU UART Port',       'BPPU',    ATCommandType.READ_WRITE],
    ['Test Mode',               'MODE',    ATCommandType.READ_WRITE],
    ['Firmware Information',    'FI',      ATCommandType.READ_ONLY],
    ['Reboot the Unit',         'R',       ATCommandType.SPECIAL],
    ['Enter Bootloader',        'BOOT',    ATCommandType.SPECIAL],
    ['Fw Image w/ E2P Update',  'FA',      ATCommandType.READ_WRITE],
    ['Fw Image w/o E2P Update', 'FAN',     ATCommandType.READ_WRITE],
    ['Built In Test',           'BIT',     ATCommandType.SPECIAL],
    ['Low transmission pwr',    'TXPLO',   ATCommandType.READ_WRITE],
    ['High transmission pwr',   'TXPHI',   ATCommandType.READ_WRITE],
    ['Report associated Nodes', 'NODES',   ATCommandType.SPECIAL],
    ['ATCMD Command',           'CMD',     ATCommandType.ATCMD],
    ['OUTPUT Command',          'OUT',     ATCommandType.ATOUT],
    ['PPU Device List',         'PPLST',   ATCommandType.READ_ONLY],
    ['RBU PP Mode Enable',      'PPEN',    ATCommandType.READ_WRITE],
    ['Broadcast PP Request',    'PPBST', ATCommandType.SPECIAL],
]

mesh_states_list = ['IDLE', 'CONFIG_SYNC', 'CONFIG_FORM', 'ACTIVE', 'TEST_MODE']

# Extract usable data from the At command definition
at_command_desc_list = next(zip(*AT_COMMAND))
at_command_list = list(zip(*AT_COMMAND))[1]
at_command_types = list(zip(*AT_COMMAND))[2]

CIE_QUEUE = [
    ['MISC'],
    ['FIRE'],
    ['FIRST AID'],
    ['FAULT'],
    ['MISC LOST'],
    ['FIRE LOST'],
    ['FIRST AID LOST'],
    ['FAULT LOST'],
    ['TxBUFFER LOST']
]

cie_queue_list = next(zip(*CIE_QUEUE))

frequencyChannels = [865150000,
                     865450000,
                     865750000,
                     866050000,
                     866350000,
                     866650000,
                     866950000,
                     867250000,
                     867550000,
                     867850000
                     ]


def resource_path(relative_path):
    """ Get absolute path to resource, works for dev and for PyInstaller """
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)


def replace_txt(box, txt):
    if box is not None:
        box.configure(state=NORMAL)
        box.delete('1.0', END)
        box.insert(INSERT, txt)
        box.configure(state=DISABLED)


def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    result = []

    ports = list(list_ports.comports())

    for port in ports:
        result.append(port.device)

    return result


def exception_callback(*args):
    err = traceback.format_exception(*args)
    print(err)
    traceback.print_exc()


class VerticalScrolledFrame(Frame):
    """A pure Tkinter scrollable frame that actually works!
    * Use the 'interior' attribute to place widgets inside the scrollable frame
    * Construct and pack/place/grid normally
    * This frame only allows vertical scrolling
    """
    def __init__(self, parent,  width=None, height=None, *args, **kw):
        Frame.__init__(self, parent, *args, width=width, height=height, **kw)

        # create a canvas object and a vertical scrollbar for scrolling it
        vscrollbar = Scrollbar(self, orient=VERTICAL)
        vscrollbar.pack(fill=Y, side=RIGHT, expand=FALSE)
        canvas = Canvas(self, bd=0, highlightthickness=0,
                        yscrollcommand=vscrollbar.set,width=width, height=height)
        canvas.pack(side=LEFT, fill=BOTH, expand=TRUE)
        vscrollbar.config(command=canvas.yview)

        # reset the view
        canvas.xview_moveto(0)
        canvas.yview_moveto(0)

        # create a frame inside the canvas which will be scrolled with it
        self.interior = interior = Frame(canvas)
        interior_id = canvas.create_window(0, 0, window=interior,
                                           anchor=NW)

        # track changes to the canvas and frame width and sync them,
        # also updating the scrollbar
        def _configure_interior(event):
            # update the scrollbars to match the size of the inner frame
            size = (interior.winfo_reqwidth(), interior.winfo_reqheight())
            canvas.config(scrollregion="0 0 %s %s" % size)
            if interior.winfo_reqwidth() != canvas.winfo_width():
                # update the canvas's width to fit the inner frame
                canvas.config(width=interior.winfo_reqwidth())
        interior.bind('<Configure>', _configure_interior)

        def _configure_canvas(event):
            if interior.winfo_reqwidth() != canvas.winfo_width():
                # update the inner frame's width to fill the canvas
                canvas.itemconfigure(interior_id, width=canvas.winfo_width())
        canvas.bind('<Configure>', _configure_canvas)


def ishex(data):
    if data.startswith('0x'):
        return all(c in string.hexdigits for c in data.lstrip('0x'))
    else:
        return False


def decode_serial_number(s_serno):
    serno = int(s_serno, 0)
    year = (serno >> 25) & 0x7f
    month = (serno >> 21) & 0xf
    batch = (serno >> 16) & 0x1f
    serial_number = serno & 0xffff

    return "%02d%02d-%02d-%05d" %(year, month, batch, serial_number)


def encode_serial_number(s_serno):
    year_month, batch, serial_number = s_serno.split('-')
    year = int(year_month[:2])
    month = int(year_month[2:])
    return (year << 25) + (month << 21) + (int(batch) << 16) + int(serial_number)


class Tooltip:
    def __init__(self, widget,
                 *,
                 bg='#FFFFEA',
                 pad=(5, 3, 5, 3),
                 text='widget info',
                 waittime=500,
                 wraplength=250):

        self.waittime = waittime  # in miliseconds, originally 500
        self.wraplength = wraplength  # in pixels, originally 180
        self.widget = widget
        self.text = text
        self.widget.bind("<Enter>", self.onEnter)
        self.widget.bind("<Leave>", self.onLeave)
        self.widget.bind("<ButtonPress>", self.onLeave)
        self.bg = bg
        self.pad = pad
        self.id = None
        self.tw = None

    def onEnter(self, event=None):
        self.schedule()

    def onLeave(self, event=None):
        self.unschedule()
        self.hide()

    def schedule(self):
        self.unschedule()
        self.id = self.widget.after(self.waittime, self.show)

    def unschedule(self):
        id_ = self.id
        self.id = None
        if id_:
            self.widget.after_cancel(id_)

    def show(self):
        def tip_pos_calculator(widget, label,
                               *,
                               tip_delta=(10, 5), pad=(5, 3, 5, 3)):

            w = widget

            s_width, s_height = w.winfo_screenwidth(), w.winfo_screenheight()

            width, height = (pad[0] + label.winfo_reqwidth() + pad[2],
                             pad[1] + label.winfo_reqheight() + pad[3])

            mouse_x, mouse_y = w.winfo_pointerxy()

            x1, y1 = mouse_x + tip_delta[0], mouse_y + tip_delta[1]
            x2, y2 = x1 + width, y1 + height

            x_delta = x2 - s_width
            if x_delta < 0:
                x_delta = 0
            y_delta = y2 - s_height
            if y_delta < 0:
                y_delta = 0

            offscreen = (x_delta, y_delta) != (0, 0)

            if offscreen:

                if x_delta:
                    x1 = mouse_x - tip_delta[0] - width

                if y_delta:
                    y1 = mouse_y - tip_delta[1] - height

            offscreen_again = y1 < 0  # out on the top

            if offscreen_again:
                # No further checks will be done.

                # TIP:
                # A further mod might automagically augment the
                # wraplength when the tooltip is too high to be
                # kept inside the screen.
                y1 = 0

            return x1, y1

        bg = self.bg
        pad = self.pad
        widget = self.widget

        # creates a toplevel window
        self.tw = Toplevel(widget)

        # Leaves only the label and removes the app window
        self.tw.wm_overrideredirect(True)

        win = Frame(self.tw,
                    background=bg,
                    relief=SOLID,
                    borderwidth=1)
        label = Label(win,
                          text=self.text,
                          justify=LEFT,
                          background=bg,
                          relief=SOLID,
                          borderwidth=0,
                          wraplength=self.wraplength)

        label.grid(padx=(pad[0], pad[2]),
                   pady=(pad[1], pad[3]),
                   sticky=NSEW)
        win.grid()

        x, y = tip_pos_calculator(widget, label)

        self.tw.wm_geometry("+%d+%d" % (x, y))

    def hide(self):
        tw = self.tw
        if tw:
            tw.destroy()
        self.tw = None


class NodeParameters:
    def __init__(self, **kwargs):
        self.Address = None
        self.Zone = None
        self.SyncMaster = None
        self.FrequencyChannel = None
        self.DeviceCombination = None
        self.SystemId = None
        self.SerialNumber = None
        self.TxLowPower = None
        self.TxHighPower = None
        self.PpModeEnable = None
        self.BootUsePPU = None

        if kwargs is not None:
            # Set the class member present in the keyword arguments
            for key, value in kwargs.items():
                setattr(self, key, value)

    def __eq__(self, other):
        """Override the default Equals behavior"""
        if isinstance(other, self.__class__):
            for var in vars(self):
                if getattr(self, var) != getattr(other, var):
                    return False
            return True
        else:
            return False

    def __ne__(self, other):
        """Override the default Unequal behavior"""
        if isinstance(other, self.__class__):
            for var in vars(self):
                if getattr(self, var) != getattr(other, var):
                    return True
            return False
        else:
            return True

    def reset(self):
        """ Set all the class members to None """
        for var in vars(self):
            setattr(self, var, None)

    def set(self, **kwargs):
        """ Set the class member present in the keyword arguments"""
        if kwargs is not None:
            for key, value in kwargs.items():
                setattr(self, key, value)