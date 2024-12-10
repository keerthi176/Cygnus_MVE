# rbu_lora_rf_test
Basic RF test for RBU platform

Embedded images for receiver, main transmitter, and interferer transmitter.

Transmitters are not synchronised.
Main transmitter waits 200ms between each transmit.
Interferer transmitter wait between each transmit ramps between 0 and 250 seconds in 10 second increments.
Receiver reports number of packets received from each transmitter along with other statistics that relate to both.

Load following:

COM4 = RX
COM5 = TX1_Main
COM6 = TX2_Interferer

Display output from COM ports in terminal (e.g. PuTTY) windows after resetting boards.

Matlab script "Serial_Display_COM456.m" can be used to display output as alternative to terminal windows.

Note COM port numbers and settings may differ, refer to device manager for these and set accordingly in Matlab script or terminal settings.

Implementation notes:
1. Messages from RBU to PC have \r\n, both CR+LF, as this was found to prevent display corruption seen when only \n used (only \r not tried).
2. Messages from PC to RBU have only \r, CR, else the platform receives no further messages until reset.
