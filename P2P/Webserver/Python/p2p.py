import serial
import threading 
import string
from time import sleep
from datetime import datetime

serial_port = serial.Serial(port="COM3", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

def set_params(StartCH, StopCH, Mode, Size, CCMA_CA, Tx_Power):
    serial_command = "setParams {} {} {} {} {} {}\n"
    serial_port.write(bytes(serial_command.format(StartCH, StopCH, Mode, Size, CCMA_CA, Tx_Power).encode('Ascii')))

def read():
    while(1):
        # Wait until there is data waiting in the serial buffer
        if(serial_port.in_waiting > 0):
            serialString = serial_port.readline().decode('Ascii')
            if "<NODE_REPORT_BEGIN>" in serialString:
                report_list = serialString.split()
                report_list.pop(0)
                print(report_list)

def write():
    while(1):
        para_list = input("Set Parameters: ").split()
        set_params(para_list[0], para_list[1], para_list[2], para_list[3], para_list[4], para_list[5])

if __name__ == "__main__":
    t_read = threading.Thread(target=read)
    t_write = threading.Thread(target=write)
    
    if serial_port.isOpen(): serial_port.close()
    serial_port.open()

    t_read.start()
    t_write.start()