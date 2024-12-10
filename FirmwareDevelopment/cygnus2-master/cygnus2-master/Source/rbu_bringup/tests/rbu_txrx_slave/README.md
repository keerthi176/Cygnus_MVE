# rbu_txrx_slave

TX_RXSlave
----------

Serial port controlled transmit or receive slave (more verbose version of Matlab_Slave).

Implementation notes:
1. Messages from RBU to PC have \r\n, both CR+LF, as this was found to prevent display corruption seen when only \n used (only \r not tried).
2. Messages from PC to RBU have only \r, CR, else the platform receives no further messages until reset.
