from django.http import HttpResponseRedirect
from django.shortcuts import render
from .forms import ParamForm, PortForm
import threading
import serial
from time import sleep
from django.contrib import messages
from .models import Node

serial_port = serial.Serial()
stop_threads = False



def write():
    while(1):
        print("test_read")
        sleep(1.1)

def set_params(StartCH, StopCH, Mode, Size, CCMA_CA, Tx_Power):
    serial_command = "setParams {} {} {} {} {} {}\n"
    serial_port.write(bytes(serial_command.format(StartCH, StopCH, Mode, Size, CCMA_CA, Tx_Power).encode('Ascii')))

def node_list(request):
    if request.method == 'POST':
        form = ParamForm(request.POST)
        if form.is_valid():
            try:
                set_params(form.cleaned_data['start_channel'], 
                           form.cleaned_data['stop_channel'],
                           form.cleaned_data['mode'],
                           form.cleaned_data['size'],
                           form.cleaned_data['ccma_ca'],
                           form.cleaned_data['tx_power'])
            except:
                messages.success(request, 'Please connect to Master')
            return render(request, 'p2p/node_list.html', {'form': form})
    else:
        form = ParamForm()
    return render(request, 'p2p/node_list.html', {'form': form})

def chart(request):
    return render(request, 'p2p/chart.html', {})

def connect(request):
    if request.method == 'POST':
        form = PortForm(request.POST)
        if form.is_valid():
            global stop_threads
            global serial_port
            if form.cleaned_data['port'] == "disconnect":
                stop_threads = True
            else:
                try:
                    serial_port = serial.Serial(port=form.cleaned_data['port'], baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
                    t_read = threading.Thread(target=read)
                    stop_threads = False
                    t_read.start()
                except:
                    messages.success(request, 'Could not open port')
            return render(request, 'p2p/connect.html', {'form': form})
    else:
        form = PortForm()
    return render(request, 'p2p/connect.html' ,{'form': form})

def info(request):
    return render(request, 'p2p/info.html' ,{})

def read():
    if serial_port.isOpen(): serial_port.close()
    serial_port.open()

    while(1):
        # Wait until there is data waiting in the serial buffer
        if(serial_port.in_waiting > 0):
            serialString = serial_port.readline().decode('Ascii')
            if "<NODE_REPORT_BEGIN>" in serialString:
                print("Node report begins")
            if "<NODE_REPORT>" in serialString:
                report_list = serialString.split()
                report_list.pop(0)

                print(report_list)
        if stop_threads: 
            serial_port.close()
            break

