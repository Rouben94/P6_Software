from django.http import HttpResponseRedirect
from django.shortcuts import render
from .forms import ParamForm
import serial

# Create your views here.
#serial_port = serial.Serial(port="COM3", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

def node_list(request):
    if request.method == 'POST':
        print("SetParameter")
        return render(request, 'p2p/node_list.html', {})
    else:
        print("SetParameter")
        return render(request, 'p2p/node_list.html', {})

def chart(request):
    form = ParamForm()
    return render(request, 'p2p/chart.html', {'form': form})