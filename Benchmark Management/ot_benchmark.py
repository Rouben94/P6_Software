import os
import serial
import threading 
import string
from time import sleep
from datetime import datetime

import pandas as pd
import csv

dirpath = os.getcwd()

serial_port = serial.Serial(port="COM84", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
bm_result_list = []

stop_threads = False
csv_title = ("ot_bm_result " + datetime.today().strftime('%H.%M %d-%m-%Y'))

def bm_start(time, messages, payload, mode, clients):
    serial_command = "benchmark_start {} {} {} {} {}\n"
    serial_port.write(bytes(serial_command.format(time, messages, payload, mode, clients).encode('Ascii')))

def read():
    global stop_threads
    while(1):
        # Wait until there is data waiting in the serial buffer
        if(serial_port.in_waiting > 0):
            serialString = serial_port.readline().decode('Ascii')
            if "<report>" in serialString:
                bm_result = serialString.replace('\x00', '').split()
                if "<report>" in bm_result: bm_result.remove("<report>")
                if ">" in bm_result: bm_result.remove(">")
                if "<" in bm_result: bm_result.remove("<")
                bm_result[0] = bm_result[5] + '_' + bm_result[0]
                if len(bm_result) == 10: bm_result_list.append(bm_result) 
                print(bm_result)

        if stop_threads:
            df = pd.DataFrame(bm_result_list, columns = ['Message ID', 'Timestamp (us)','Ack Timestamp (us)', 'Hops','RSSI','Source Address','Destination Address','Group Address','Data Size', 'Node_ID'])
            path = dirpath+'\\result_files\\'+csv_title+'.csv'
            df.to_csv(path,index=False,sep=';',encoding='utf-8')
            break

if __name__ == "__main__":
    t_read = threading.Thread(target=read)
    
    if serial_port.isOpen(): serial_port.close()
    serial_port.open()

    t_read.start()

    bm_parameters = input("Please insert Parameters <Benchmark time in s> <Number of messages> <Payload in Byte> <mode> <Number of Clients>").split()
    bm_start(bm_parameters[0], bm_parameters[1], bm_parameters[2], bm_parameters[3], bm_parameters[4])
    sleep(int(bm_parameters[0])+70)
    while(True):
        if input("Did you got all results?") == 'y':
            stop_threads = True
            break