from tkinter import *
import ctypes
from ctypes import c_long, c_int, c_char_p, create_string_buffer
import PY_MC_PUP
from PY_MC_PUP import *
from CO_Message import *


boutstring = create_string_buffer(2048)
binstring = create_string_buffer(2048)

# handler for when user clicks the Decode button
def OnClickDecode():
    global boutstring
    global binString
    
    binString = e1.get()
    decoded_line = PY_MC_PUP.packet_decode(packet=binString)
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
        # display output string
        t1.configure(state='normal')
        t1.delete("1.0","100.0")
        t1.insert(INSERT, str(decoded_line) + str('\n'))
        t1.configure(state='disabled')


# handler for when user clicks the Clear button
def OnClickClear():
    # clear input string
    e1.delete(0, 'end')

    # clear output string
    t1.configure(state='normal')
    t1.delete("1.0","100.0")
    t1.configure(state='disabled')

# create root window
root = Tk()
root.title("Mesh Message Decoder")
root.minsize(width=400, height=140)

# create container frame
f1 = Frame(root)
f1.pack(padx=5, pady=5, fill=BOTH, expand=1)

# create entry for message input with label
l1 = Label(f1, text='Input (hex string)')
l1.pack(fill=X)
e1 = Entry(f1, font=('Courier', '10'))
e1.pack(fill=X)

# create container frame
f2 = Frame(f1)
f2.pack()

# create decode button
b1 = Button(f2, text="Decode", height=1, command=OnClickDecode)
b1.pack(side=LEFT, padx=5, pady=10, fill=X)

# create clear button
b2 = Button(f2, text="Clear", height=1, command=OnClickClear)
b2.pack(side=LEFT, padx=5, pady=10, fill=X)


# create text box for output with label
# text box is disabled to prevent user input
l2 = Label(f1, text='Decoder Output')
l2.pack(fill=X)
t1 = Text(f1, height=4, font=('Courier', '10'))
t1.pack(fill=BOTH, expand=1)
t1.configure(state='disabled')


# run main loop
root.mainloop()
