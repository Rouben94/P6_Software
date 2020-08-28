# P6_Software/P2P


P2P Software is a nRF Connect SDK Based Project. The purpose of this code is to meassure which Channels and Modes (BLE_1Mbit, BLE_2Mbit, etc.) are the best to establish a communicaiton. Therefore Testmessages are send between a master node and one or many slave nodes. The Testmessage is configurable in Mode, Start Channel and Stop Channel. The meassurment results are SNR (dB) and Packet Loss (%) per channel and mode. 

# Building

The build and run the project the following Prerequisites are necessery:

- A Board compatible to nRF Connect SDK (nRF52840, nRF5240)
- Installed nRF Connect SDK (see Instructions: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_assistant.html#gs-assistant)
- Set Enviroment Variable "ZephyrBase" pointing to ncs/zephyr

# Flashing Prbuilt Binaries

To flash the prebuilt binaries under the /prebuilt_binaries continue as follows:

- 1. Install nrf Command LIne Tools available here: https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools
- 2. Install nrf Uitl available here: https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Util
- 3. flah the master or slave on a PCA10056 board with the following command: nrfjprog -f nrf52 --program <PATH_TO_prebuilt_binaries>/*.hex --sectorerase
- 4. flah the slave on a PCA10059 board with the following command: nrfutil dfu usb-serial -pkg Slave_PCA10059.zip -p <SERIAL_PORT>
- 5. continue by opening a serial command line over UART with 115200 Baud, 8 bits, no FLowctrl for the Master ... you should see the statemachine progressing.



