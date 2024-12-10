# rig specific terminal configurations
# intended to be called in terminal_start.py

# define constants
TERATERM_PATH = 'C:\\Program Files (x86)\\teraterm'
BAUDRATE = '2000000'

terminalConfigurations = \
[
 ['NCU',  '/C=5',  '/X=100', '/Y=450'],
 ['RBU1', '/C=6',  '/X=800', '/Y=450'],
 ['RBU2', '/C=12', '/X=800', '/Y=100'],
 ['RBU3', '/C=16', '/X=100', '/Y=100']
]
