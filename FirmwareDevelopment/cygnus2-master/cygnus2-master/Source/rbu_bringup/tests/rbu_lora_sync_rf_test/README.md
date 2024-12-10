# rbu_lora_sync_rf_test
Basic synchronised RF test for RBU platform

Embedded images for receiver, main transmitter, and interferer transmitter.

Transmitters are synchronised to packet sent from receiver prior to each transmit packet.
Receiver waits 0.5s after receiving a packet before performing synchronisation again.
Receiver reports moving average RSSI and SNR for main packets received from main transmitter.

Load following:

COM4 = Sync_RX
COM5 = Sync_TX1_Main
COM6 = Sync_TX2_Interferer

Display output from COM ports in terminal (e.g. PuTTY) windows after resetting boards.

Matlab script "Serial_Display_COM456.m" can be used to display output as alternative to terminal windows.

Note COM port numbers and settings may differ, refer to device manager for these and set accordingly in Matlab script or terminal settings.

Implementation notes:
1. Messages from RBU to PC have \r\n, both CR+LF, as this was found to prevent display corruption seen when only \n used (only \r not tried).
2. Messages from PC to RBU have only \r, CR, else the platform receives no further messages until reset.
3. Some transmitter "SYNC Error - Reset all boards..." messages have to be waived when performing PER measurements with different relative power levels.
