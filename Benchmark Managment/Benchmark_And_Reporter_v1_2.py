import os
import time
import datetime
import serial.tools.list_ports
import pandas as pd
import subprocess
from openpyxl import load_workbook
import numpy as np

COM_PORT_Master = ''

dirpath = os.getcwd() + '\\result_files'

report = []

#helper function to read all bytes
def read_all_lines(port):
    port.reset_input_buffer()
    """Read all characters on the serial port and return them."""
    if not port.timeout:
        raise TypeError('Port needs to have a timeout set!')

    read_buffer = b''
    
    while True:
        # Read in chunks. Each chunk will wait as long as specified by
        # timeout. Increase chunk_size to fail quicker
        byte_chunk = port.readline()
        read_buffer += byte_chunk
        print(byte_chunk.decode('utf-8'), end = '')
        if not len(byte_chunk) >= 1 or 'Ready for Control Message' in byte_chunk.decode('utf-8'):
            break

    return read_buffer

# Python code to sort the tuples using second element  
# of sublist Inplace way to sort using sort() 
def Sort(sub_li): 
  
    # reverse = None (Sorts in Ascending order) 
    # key is set to sort using first element of  
    # sublist lambda has been used 
    sub_li.sort(key = lambda x: x[0]) 
    return sub_li 




#Open COnfig File
df = pd.read_excel('Config.xlsx', sheet_name='Config')

#Get the COM Ports
ports = list(serial.tools.list_ports.comports())
for p in ports:
    if 'JLink CDC UART Port' in p.description:
        print('Assume Master on: ' + p.device)
        COM_PORT_Master = p.device
        ser = serial.Serial(p.device, 115200,timeout=4)
        ser.set_buffer_size(rx_size = 12800, tx_size = 12800)
if (COM_PORT_Master == ''):
    print('No Master Detected... exit')
    import sys
    sys.exit()

x = input('If you like to Start a new Benchmark Press enter \nIf you like to get the reports press s followed by Enter to skip Benchmarking.')
if not x == 's':

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
            print(serdataline_str, end = '')            
            if 'Ready for Control Message' in serdataline_str:
                print("Ready for Reporting.. start in 2 seconds")
                break
    time.sleep(2)


had_nodes_ind = []

ind = 0

# iterate through each row entry (Slave)
while len(had_nodes_ind) < len(df.index):
#for ind in df.index:
    if str(df['Number'][ind]) == 'nan':
        break
    if str(df['Number'][ind]) not in had_nodes_ind :
        #Get Device Report
        x = input('Please go near to Dongle number ' + str(df['Number'][ind]) + ' ... Press enter to continue. \nPress s followed by Enter to skip. \nPress q followed by Enter')
        if x == 's':
            ind = ind + 1
            if (ind >= len(df.index)):
                ind = 0
            continue
        if x == 'q':
            break
        print("Get Report of Node " + str(df['Dev ID'][ind]) +" in 3 seconds")
        time.sleep(3)
        ser.reset_input_buffer()
        print("getNodeReport " + str(int(str(df['Dev ID'][ind]), 16)))
        serialcmd = "getNodeReport " + str(int(str(df['Dev ID'][ind]), 16)) + "\r"
        ser.write(serialcmd.encode("ascii"))
        ser.reset_input_buffer()
        #read data from serial port
        ReportsCnt = 0
        serdata = read_all_lines(ser).decode("utf-8")
        for serdataline in serdata.split('\n'):
            if '<report>' in serdataline:
                entry = serdataline.split()
                remove_tag_entry = entry.pop(0)
                #Manipulate Mesage ID
                entry[0] = str(entry[5]) + "_" + str(entry[0])
                report.append(entry)            
                ReportsCnt = ReportsCnt + 1
        print("Got " + str(ReportsCnt) + " Reports")
        if ReportsCnt > 0:
            had_nodes_ind.append(str(df['Number'][ind]))
            ind = ind + 1
            if (ind >= len(df.index)):
                ind = 0
ser.close()

print(report)

if len(report) > 0:
    # Create the pandas DataFrame
    Sort(report) #Sort by message ID
    df = pd.DataFrame(report, columns = ['Message ID', 'Timestamp (us)','Ack Timestamp (us)', 'Hops','RSSI','Source Address','Destination Address','Group Address','Data Size'])
    #Prepare Writing to File
    path = dirpath + "\\Results_" + str(datetime.datetime.now().strftime("%I_%M%p_on_%B_%d_%Y")) + ".csv"
    #writer = pd.ExcelWriter(path, engine = 'openpyxl')
    df.to_csv(path,index=False,sep=';',encoding='utf-8')

    #writer.save()
    #writer.close()
            
