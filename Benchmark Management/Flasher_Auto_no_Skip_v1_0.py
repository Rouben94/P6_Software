import os
import time
import serial.tools.list_ports
import pandas as pd
import subprocess

#pip3 install pandas pyserial

COM_PORT_Dongle = ''
COM_PORT_Dongle_before = ''

dirpath = os.getcwd() + '\\flash_files'

#Open COnfig File
df = pd.read_excel('Config.xlsx', sheet_name='Config')

# iterate through each row entry (Slave)
for ind in df.index:
    if str(df['Number'][ind]) == 'nan':
        break
    print('Insert Dongle number ' + str(df['Number'][ind]))
    while COM_PORT_Dongle == COM_PORT_Dongle_before:
        #Get the COM Ports
        ports = list(serial.tools.list_ports.comports())
        for p in ports:
            if 'nRF52 SDFU USB' in p.description:
                #print('Assume nRF52 DFU Dongle on: ' + p.device)
                COM_PORT_Dongle = p.device
        time.sleep(1)
    print('Dongle number ' + str(df['Number'][ind]) + ' detected.. flashing')
    COM_PORT_Dongle_before = COM_PORT_Dongle
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

            
