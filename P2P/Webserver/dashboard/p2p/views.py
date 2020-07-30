from django.shortcuts import render

def device_list(request):
    return render(request, 'p2p/device_list.html', {})

def chart(request):
    return render(request, 'p2p/chart.html', {})