import os
import time
import datetime
import serial.tools.list_ports
import pandas as pd
import subprocess
from openpyxl import load_workbook
import numpy as np

COM_PORT_Master = ''

dirpath = os.getcwd()

report = []

#Prepare Writing to File
book = load_workbook(dirpath + "\\Config.xlsx")
writer = pd.ExcelWriter(dirpath + "\\Config.xlsx", engine = 'openpyxl')
writer.book = book

#Open COnfig File
df = pd.read_excel('Config.xlsx', sheet_name='Config')

#Get the COM Ports
ports = list(serial.tools.list_ports.comports())
for p in ports:
    if 'JLink CDC UART Port' in p.description:
        print('Assume Master on: ' + p.device)
        COM_PORT_Master = p.device
        ser = serial.Serial(p.device, 115200,timeout=10)
if (COM_PORT_Master == ''):
    print('No Master Detected... exit')
    import sys
    sys.exit()

# iterate through each row entry (Slave)
for ind in df.index: 

    #Get Device Report
    print("Get Report of Node " + str(df['Dev ID'][ind]) +" in 3 seconds")
    time.sleep(3)
    print("getNodeReport " + str(int(str(df['Dev ID'][ind]), 16)))
    serialcmd = "getNodeReport " + str(int(str(df['Dev ID'][ind]), 16)) + "\r"
    ser.write(serialcmd.encode("ascii"))
    ser.flushInput()
    #read data from serial port
    ReportsCnt = 0
    timeout = time.time() + 10   # 10 seconds from now
    while True:        
        serdataline = ser.readline()
        #if there is something process it
        if len(serdataline) >= 1:
            serdataline_str = serdataline.decode("utf-8")
            print(serdataline_str)
            if '<report>' in serdataline_str:
                entry = serdataline_str.split()
                remove_tag_entry = entry.pop(0)
                report.append(entry)
                ReportsCnt = ReportsCnt + 1
                timeout = time.time() + 1 # Extend Timeout
            elif 'Ready for Control Message' in serdataline_str:
                print("Got " + str(ReportsCnt) + " Reports")
                break
        elif time.time() > timeout:
            print("Got " + str(ReportsCnt) + " Reports")
            break

ser.close()

print(report)

# Create the pandas DataFrame 
df = pd.DataFrame(report, columns = ['Message ID', 'Timestamp (us)','Ack Timestamp (us)', 'Hops','RSSI','Source Address','Destination Address','Group Address'])    
df.to_excel(writer, sheet_name = str(datetime.datetime.now().strftime("%I_%M%p on %B %d, %Y")))

writer.save()
writer.close()
            
