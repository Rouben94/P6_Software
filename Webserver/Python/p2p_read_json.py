import serial
import threading 
from time import sleep
import re 
import json

serial_port = serial.Serial(port="COM9", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

def bm_start(time):
    serial_command = "benchmark_start {}\n"
    serial_port.write(bytes(serial_command.format(time).encode('Ascii')))

def scan():
    serial_command = "scan\n"
    serial_port.write(bytes(serial_command.encode('Ascii')))

def read():
    while(1):
        # Wait until there is data waiting in the serial buffer
        if(serial_port.in_waiting > 0):
            serialString = serial_port.readline().decode('Ascii')
            print(serialString)

def write():
    while(1):
        input()
        #bm_start(time)
        scan()
        sleep(0.2)

if __name__ == "__main__":
    t_read = threading.Thread(target=read)
    t_write = threading.Thread(target=write)

    if serial_port.isOpen(): serial_port.close()
    serial_port.open()

    t_read.start()
    t_write.start()