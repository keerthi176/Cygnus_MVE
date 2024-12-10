# rig specific terminal configurations
# intended to be called in terminal_start.py

# define constants
TERATERM_PATH = 'C:\\Program Files (x86)\\teraterm'
BAUDRATE = '2000000'

terminalConfigurations = \
[
 ['NCU',  '/C=9',  '/X=100', '/Y=10'],
 ['RBU3', '/C=14',  '/X=1450', '/Y=10'],
 ['RBU2', '/C=17', '/X=1000', '/Y=10'],
 ['RBU1', '/C=18', '/X=550', '/Y=10']
]
