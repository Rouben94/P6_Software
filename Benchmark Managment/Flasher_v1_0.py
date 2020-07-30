import os
import time
import serial.tools.list_ports
import pandas as pd
import subprocess

#pip3 install pandas pyserial

COM_PORT_Dongle = ''

dirpath = os.getcwd()

#Open COnfig File
df = pd.read_excel('Config.xlsx', sheet_name='Config')

# iterate through each row entry (Slave)
for ind in df.index: 
    x = input('Please insert Dongle number ' + str(df['Number'][ind]) + ' ... Press enter to continue. Press s followed by Enter to skip')
    if x == 's':
        continue
    #Get the COM Ports
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if 'nRF52 SDFU USB' in p.description:
            print('Assume nRF52 DFU Dongle on: ' + p.device)
            COM_PORT_Dongle = p.device
    if (COM_PORT_Dongle == ''):
        print('No DOngle Detected... continue with next')
        continue #Do next Entry if no Dongle conected

    #Flash the Firmware
    if '.hex' in str(df['Firmware'][ind]):
        print('Create Package File')
        print("nrfutil pkg generate --hw-version 52 --sd-req 0x00 --application-version 1 --application  " + '"' + dirpath + "\\" + str(df['Firmware']))
        os.system("nrfutil pkg generate --hw-version 52 --sd-req 0x00 --application-version 1 --application  " + '"' + dirpath + "\\" + str(df['Firmware'][ind]) + '" ' + '"' + dirpath + "\\" + str(df['Firmware'][ind]) + '.zip"')
        print("nrfutil dfu usb-serial -pkg " + dirpath + "\\" + str(df['Firmware'][ind]) + " -p " + COM_PORT_Dongle)
        os.system("nrfutil dfu usb-serial -pkg " + '"' + dirpath + "\\" + str(df['Firmware'][ind]) + '.zip"' + " -p " + COM_PORT_Dongle)
    else:        
        print("nrfutil dfu usb-serial -pkg " + dirpath + "\\" + str(df['Firmware'][ind]) + " -p " + COM_PORT_Dongle)
        os.system("nrfutil dfu usb-serial -pkg " + '"' + dirpath + "\\" + str(df['Firmware'][ind]) + '"' + " -p " + COM_PORT_Dongle)
        #subprocess.run(["nrfutil", "dfu", "usb-serial", "-pkg " + dirpath + "/" + df['Firmware'][ind] ,"-p " + COM_PORT_Dongle ],capture_output=True)

            
