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

#helper function to read all bytes
def read_all(port, chunk_size=10):
    """Read all characters on the serial port and return them."""
    if not port.timeout:
        raise TypeError('Port needs to have a timeout set!')

    read_buffer = b''

    while True:
        # Read in chunks. Each chunk will wait as long as specified by
        # timeout. Increase chunk_size to fail quicker
        byte_chunk = port.read(size=chunk_size)
        read_buffer += byte_chunk
        if not len(byte_chunk) == chunk_size:
            break

    return read_buffer


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
        ser = serial.Serial(p.device, 115200,timeout=4)
if (COM_PORT_Master == ''):
    print('No Master Detected... exit')
    import sys
    sys.exit()

bm_time_s = input('Enter Benchmark Time (s):')
bm_events = input('Enter Benchmark Events (1-1000):')

print("startBM " + str(bm_time_s) + " " + str(bm_events))
serialcmd = "startBM " + str(bm_time_s) + " " + str(bm_events) + "\r"
ser.write(serialcmd.encode("ascii"))
ser.flushInput()

while True:        
    serdataline = ser.readline()
    #if there is something process it
    if len(serdataline) >= 1:
        serdataline_str = serdataline.decode("utf-8")
        print(serdataline_str)            
        if 'Ready for Control Message' in serdataline_str:
            print("Ready for Reporting.. start in 2 seconds")
            break
time.sleep(2)

# iterate through each row entry (Slave)
for ind in df.index: 

    #Get Device Report
    input('Please go near to Dongle number ' + str(df['Number'][ind]) + ' ... Press enter to continue')
    print("Get Report of Node " + str(df['Dev ID'][ind]) +" in 3 seconds")
    time.sleep(3)
    ser.reset_input_buffer()
    print("getNodeReport " + str(int(str(df['Dev ID'][ind]), 16)))
    serialcmd = "getNodeReport " + str(int(str(df['Dev ID'][ind]), 16)) + "\r"
    ser.write(serialcmd.encode("ascii"))
    ser.reset_input_buffer()
    #read data from serial port
    ReportsCnt = 0
    serdata = ''
    while True:        
        #serdataline = ser.readline()
        serdata = serdata + ser.read(size=ser.in_waiting).decode("utf-8")
        if 'Ready for Control Message' in serdata:
            serdatalines = serdata.split('\n')
        for serdataline in serdatalines:
            #if there is something process it
            if len(serdataline) >= 1:
                serdataline_str = serdataline.decode("utf-8")
                
                print(serdataline_str)
                if '<report>' in serdataline_str:
                    entry = serdataline_str.split()
                    remove_tag_entry = entry.pop(0)
                    report.append(entry)
                    ReportsCnt = ReportsCnt + 1
                elif 'Ready for Control Message' in serdataline_str:
                    print("Got " + str(ReportsCnt) + " Reports")
                    breakpoint

ser.close()

print(report)

# Create the pandas DataFrame 
df = pd.DataFrame(report, columns = ['Message ID', 'Timestamp (us)','Ack Timestamp (us)', 'Hops','RSSI','Source Address','Destination Address','Group Address'])    
df.to_excel(writer, sheet_name = str(datetime.datetime.now().strftime("%I_%M%p on %B %d, %Y")))

writer.save()
writer.close()
            
