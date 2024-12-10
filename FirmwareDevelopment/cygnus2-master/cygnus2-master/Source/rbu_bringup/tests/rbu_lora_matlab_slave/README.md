# rbu_lora_matlab_slave
Matlab slave test for RBU platform

Matlab script "Serial_Master_COM456.m" demonstrates Matlab control over 3 RBU slaves.

Simply configures one slave as master, sending transmit commands with fixed payloads to 2 transmit slaves.

The response received on RxDone/TxDone is displayed, where Rx sends ‘D <payload> <length> <RSSI> <SNR>’ response, and Tx sends ‘D <length> <payload>’ response.

Note payload length is fixed for SF6.

Implementation notes:
1. Messages from RBU to PC have \r\n, both CR+LF, as this was found to prevent display corruption seen when only \n used (only \r not tried).
2. Messages from PC to RBU have only \r, CR, else the platform receives no further messages until reset.
