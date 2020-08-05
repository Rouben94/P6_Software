import os
import time
import serial.tools.list_ports
import pandas as pd
import subprocess



COM_PORT_Master = ''

dirpath = os.getcwd()

#Open COnfig File
df = pd.read_excel('Config.xlsx', sheet_name='Config')

# creating a list of dataframe columns 
columns = list(df) 
        

#Get the COM Ports
ports = list(serial.tools.list_ports.comports())
for p in ports:
    if 'JLink CDC UART Port' in p.description:
        print('Assume Master on: ' + p.device)
        COM_PORT_Master = p.device
        ser = serial.Serial(p.device, 115200)
if (COM_PORT_Master == ''):
    print('No Master Detected... exit')
    import sys
    sys.exit()

# iterate through each row entry (Slave)
for ind in df.index: 

    #Configure the Device
    if str(df['Number'][ind]) == 'nan':
        break
    print("Set node settings for Node " + str(df['Dev ID'][ind]))
    #time.sleep(3)
    print("setNodeSettings " + str(int(str(df['Dev ID'][ind]), 16)) + " " + str(df['Group ID'][ind]) + " " + str(df['Node Id'][ind]) + " " + str(df['Ack'][ind]) + " " + str(df['Add Payload Len'][ind]) + " " + str(int(str(df['DST_MAC_1'][ind]), 16)) + " " + str(int(str(df['DST_MAC_2'][ind]), 16)) + " " + str(int(str(df['DST_MAC_3'][ind]), 16)))
    serialcmd = "setNodeSettings " + str(int(str(df['Dev ID'][ind]), 16)) + " " + str(df['Group ID'][ind]) + " " + str(df['Node Id'][ind]) + " " + str(df['Ack'][ind]) + " " + str(df['Add Payload Len'][ind]) + " " + str(int(str(df['DST_MAC_1'][ind]), 16)) + " " + str(int(str(df['DST_MAC_2'][ind]), 16)) + " " + str(int(str(df['DST_MAC_3'][ind]), 16)) + "\r"
    ser.write(serialcmd.encode("ascii"))
    ser.flushInput()
    while True:        
        serdataline = ser.readline()
        #if there is something process it
        if len(serdataline) >= 1:
            serdataline_str = serdataline.decode("utf-8")
            print(serdataline_str)            
            if 'Ready for Control Message' in serdataline_str:
                print("Ready for next Node")
                time.sleep(2)
                break
    

ser.close()
            
