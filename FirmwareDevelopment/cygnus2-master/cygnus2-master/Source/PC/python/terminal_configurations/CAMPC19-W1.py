# rig specific terminal configurations
# intended to be called in terminal_start.py

# define constants
TERATERM_PATH = 'C:\\Program Files (x86)\\teraterm'
BAUDRATE = '2000000'

terminalConfigurations = \
[
 ['NCU',  '/C=11',  '/X=1450', '/Y=10'],
 ['RBU1', '/C=10', '/X=2010', '/Y=0'],
 ['RBU2', '/C=13', '/X=2480', '/Y=0'],
 ['RBU3', '/C=12',  '/X=2940', '/Y=0'],
# ['RBU4',  '/C=14',  '/X=3500', '/Y=0'],
# ['RBU5', '/C=7',  '/X=2480', '/Y=350'],
# ['RBU6', '/C=16', '/X=2940', '/Y=350'],
# ['RBU7', '/C=15', '/X=2710', '/Y=700']
]
