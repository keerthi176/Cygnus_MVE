from tkinter import *
import ctypes
from ctypes import cdll, windll, c_long, c_int, c_char_p, create_string_buffer

# define DLL and function
lib = ctypes.CDLL('Mesh.dll')
func = lib['decode']
func.restype = ctypes.c_int

boutstring = create_string_buffer(2048)

# handler for when user clicks the Decode button
def OnClickDecode():
    global boutstring

    msg = e1.get()

    # remove spaces
    strippedmsg = re.sub(r'\s+', '', msg)

    # encode as ascii string
    binstring = strippedmsg.encode('ascii')

    # call message decoder
    result = func(binstring, boutstring)

    # display output string
    t1.configure(state='normal')
    t1.delete("1.0","100.0")
    t1.insert(INSERT, boutstring.value.decode('ascii') + str('\n'))
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
