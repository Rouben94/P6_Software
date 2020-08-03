import os
import serial
import threading 
import string
from time import sleep
from datetime import datetime

import pandas as pd
import csv

dirpath = os.getcwd()

serial_port = serial.Serial(port="COM4", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
bm_result_list = []

stop_threads = False
csv_title = ("ot_bm_result " + datetime.today().strftime('%H.%M %d-%m-%Y'))

def bm_start(time):
    serial_command = "benchmark_start {}\n"
    serial_port.write(bytes(serial_command.format(time).encode('Ascii')))

def read():
    global stop_threads
    while(1):
        # Wait until there is data waiting in the serial buffer
        if(serial_port.in_waiting > 0):
            serialString = serial_port.readline().decode('Ascii')
            if "<report>" in serialString:
                bm_result = serialString.replace('\x00', '').split()
                bm_result.remove("<report>")
                if ">" in bm_result: bm_result.remove(">")
                if "<" in bm_result: bm_result.remove("<")
                bm_result[0] = bm_result[5] + '_' + bm_result[0]
                bm_result_list.append(bm_result) 
                print(bm_result)

            elif "<REPORT_END>" in serialString:
                sleep(2)
                if stop_threads:
                    df = pd.DataFrame(bm_result_list, columns = ['Message ID', 'Timestamp (us)','Ack Timestamp (us)', 'Hops','RSSI','Source Address','Destination Address','Group Address','Data Size'])
                    path = dirpath+'\\result_files\\'+csv_title+'.csv'
                    df.to_csv(path,index=False,sep=';',encoding='utf-8')
                    break
                bm_start(30000)

""" def write():
    global stop_threads
    input("Start with enter: ")
    while(1):
        bm_start(60000)
        sleep(90)
        if stop_threads:
            df = pd.DataFrame(bm_result_list, columns = ['Message ID', 'Timestamp (us)','Ack Timestamp (us)', 'Hops','RSSI','Source Address','Destination Address','Group Address','Data Size'])
            path = dirpath+'\\result_files\\'+csv_title+'.csv'
            df.to_csv(path,index=False,sep=';',encoding='utf-8')
            break """

if __name__ == "__main__":
    t_read = threading.Thread(target=read)
    """ t_write = threading.Thread(target=write) """
    
    if serial_port.isOpen(): serial_port.close()
    serial_port.open()

    t_read.start()
    """ t_write.start() """

    bm_time = input("How long would you like to test? (min): ")
    bm_start(30000)
    sleep(int(bm_time)*60)
    stop_threads = True