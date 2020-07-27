import os
import time
import serial.tools.list_ports
import pandas as pd
import subprocess

COM_PORT_Dongle = ''

dirpath = os.getcwd()

#Open COnfig File
df = pd.read_excel('Config.xlsx', sheet_name='Config')

#Get the COM Ports
ports = list(serial.tools.list_ports.comports())
for p in ports:
    if 'JLink CDC UART Port' in p.description:
        print('Assume Master on: ' + p.device)
        ser = serial.Serial(p.device, 115200)
if (COM_PORT_Dongle == ''):
    print('No Master Detected... exit')
    import sys
    sys.exit()

# iterate through each row entry (Slave)
for ind in df.index: 

    #Configure the Device
    print("Set node settings for Node " + str(df['Dev ID'][ind]) +" in 3 seconds")
    time.sleep(3)
    print("setNodeSettings " + str(int(str(df['Dev ID'][ind]), 16)) + " " + str(df['Group ID'][ind]))
    serialcmd = "setNodeSettings " + str(int(str(df['Dev ID'][ind]), 16)) + " " + str(df['Group ID'][ind]) + "\r"
    ser.write(serialcmd.encode("ascii"))
    ser.flushInput()

ser.close()
            
