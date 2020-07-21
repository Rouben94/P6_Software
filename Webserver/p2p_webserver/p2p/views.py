from django.shortcuts import render
import serial

# Create your views here.
#serial_port = serial.Serial(port="COM3", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

def node_list(request):
    return render(request, 'p2p/node_list.html', {})

def chart(request):
    return render(request, 'p2p/chart.html', {})