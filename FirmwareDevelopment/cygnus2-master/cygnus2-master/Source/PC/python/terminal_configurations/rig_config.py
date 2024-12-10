# rig specific terminal configurations

# define constants
TERATERM_PATH = 'C:\\Program Files (x86)\\teraterm'
BAUDRATE = '2000000'
FREQUENCY = '5'
SYSTEM_ID = '1001'

terminalConfigurations = \
[
 # [Name, com port, x pos, y pos, UA, DEVCF, TXPLO, TXPHI, SERNO]
 ['NCU', '/C=8', '/X=0', '/Y=0', '0,1', '0', '2', '5', '1805-30-00001' ],
 ['RBU2', '/C=7', '/X=450', '/Y=0', '0,2', '0', '2', '5', '1805-30-00002' ],
 ['RBU3', '/C=11', '/X=900', '/Y=0', '0,3', '0', '2', '5', '1805-30-00003' ],
 ['RBU4', '/C=12', '/X=1910', '/Y=0', '0,4', '0', '2', '5', '1805-30-00004' ],
 ['RBU5', '/C=22', '/X=2340', '/Y=0', '0,5', '0', '2', '5', '1805-30-00005' ],
 ['RBU6', '/C=20', '/X=2770', '/Y=0', '0,6', '0', '2', '5', '1805-30-00006' ],
 ['RBU7', '/C=21', '/X=3200', '/Y=0', '0,7', '0', '2', '5', '1805-30-00007' ],
 ['RBU8', '/C=23', '/X=0', '/Y=260', '0,8', '0', '2', '5', '1805-30-00008' ],
 ['RBU9', '/C=13', '/X=450', '/Y=260', '0,9', '0', '2', '5', '1805-30-00009' ],
 ['RBU10', '/C=14', '/X=900', '/Y=260', '0,10', '0', '2', '5', '1805-30-00010' ],
 ['RBU11', '/C=15', '/X=1910', '/Y=260', '0,11', '0', '2', '5', '1805-30-00011' ],
 ['RBU12', '/C=24', '/X=2340', '/Y=260', '0,12', '0', '2', '5', '1805-30-00012' ],
 ['RBU13', '/C=25', '/X=2770', '/Y=260', '0,13', '0', '2', '5', '1805-30-00013' ],
 ['RBU14', '/C=26', '/X=3200', '/Y=260', '0,14', '0', '2', '5', '1805-30-00014' ],
 ['RBU15', '/C=27', '/X=0', '/Y=520', '0,15', '0', '2', '5', '1805-30-00015' ],
 ['RBU16', '/C=10', '/X=450', '/Y=520', '0,16', '0', '2', '5', '1805-30-00016' ],
 ['RBU17', '/C=6', '/X=900', '/Y=520', '0,17', '0', '2', '5', '1805-30-00017' ],
 ]
