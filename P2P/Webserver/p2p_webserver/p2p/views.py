from django.http import HttpResponseRedirect
from django.shortcuts import render
from .forms import ParamForm, PortForm
import threading
import serial
from time import sleep
from django.contrib import messages
from .models import Node, Channel
from plotly.offline import plot
import plotly.graph_objects as go

serial_port = serial.Serial()
stop_threads = False
number_of_nodes = 0


def write():
    while(1):
        print("test_read")
        sleep(1.1)


def set_params(StartCH, StopCH, Mode, Size, CSMA_CA, Tx_Power):
    serial_command = "setParams {} {} {} {} {} {}\n"
    serial_port.write(bytes(serial_command.format(
        StartCH, StopCH, Mode, Size, CSMA_CA, Tx_Power).encode('Ascii')))


def node_list(request):
    nodes = Node.objects.all()
    if request.method == 'POST':
        form = ParamForm(request.POST)
        if form.is_valid():
            try:
                set_params(form.cleaned_data['start_channel'],
                           form.cleaned_data['stop_channel'],
                           form.cleaned_data['mode'],
                           form.cleaned_data['size'],
                           form.cleaned_data['csma_ca'],
                           form.cleaned_data['tx_power'])
            except:
                messages.success(request, 'Please connect to Master')
            return render(request, 'p2p/node_list.html', {'form': form, 'nodes': nodes})
    else:
        form = ParamForm()
    return render(request, 'p2p/node_list.html', {'form': form, 'nodes': nodes})


def chart(request):
    nodes = Node.objects.all()
    bar_div_array = []
    for node in nodes:
        x_data = []
        y_data = []
        y_desc = []
        for channel in node.channel_set.all():
            x_data.append('Channel ' + str(channel.ch))
            y_data.append(float(channel.packetloss))
            y_desc.append(channel.packetloss+' %')

        bar_array = go.Figure(
                data=[
                    go.Bar(
                        name="Channel Packetloss",
                        x=x_data,
                        y=y_data,
                        text=y_desc,
                        textposition='auto',
                    ),
                ],
                layout=go.Layout(
                    title="Packetloss over Channel",
                    yaxis_title="Packetloss [%]",
                    width=1300,
                )
            )
        bar_array.update_traces(opacity=0.75)
        bar_div_array.append(plot(bar_array, output_type='div', include_plotlyjs=False))
    
    bar_list = zip(bar_div_array, nodes)
    return render(request, 'p2p/chart.html', {'bar_list': bar_list})



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
                    serial_port = serial.Serial(
                        port=form.cleaned_data['port'], baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
                    t_read = threading.Thread(target=read)
                    stop_threads = False
                    t_read.start()
                except:
                    serial_port.close()
                    messages.success(request, 'Could not open port')
            return render(request, 'p2p/connect.html', {'form': form})
    else:
        form = PortForm()
    return render(request, 'p2p/connect.html', {'form': form})


def info(request):
    return render(request, 'p2p/info.html', {})


def read():
    if serial_port.isOpen():
        serial_port.close()
    serial_port.open()
    Node.objects.all().delete()
    Channel.objects.all().delete()

    while(1):
        # Wait until there is data waiting in the serial buffer
        if(serial_port.in_waiting > 0):
            serialString = serial_port.readline().decode('Ascii')
            global number_of_nodes
            if "<NODE_REPORT_BEGIN>" in serialString:
                print("Node report begins")
                number_of_nodes = 0
                Node.objects.all().delete()
                Channel.objects.all().delete()
            if "<NEW_NODE>" in serialString:
                number_of_nodes += 1
                Node.objects.create(pk=number_of_nodes)
            if "<NODE_REPORT>" in serialString:
                report_list = serialString.split()
                report_list.pop(0)
                Node.objects.filter(pk=number_of_nodes).update(
                    mac=report_list[0])
                Channel.objects.create(ch=int(report_list[1]), signal_to_noise_ratio=str(int(report_list[6])-int(report_list[5])), packetloss=str(
                    round((1-(int(report_list[3])/int(report_list[2])))*100, 1)), node=Node.objects.get(pk=number_of_nodes))
                print(report_list)
            if "<NODE_REPORT_END>" in serialString:
                print("Node report ends")
        if stop_threads:
            Node.objects.all().delete()
            Channel.objects.all().delete()
            serial_port.close()
            break
