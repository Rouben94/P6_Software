from django.urls import path
from . import views

urlpatterns = [
    path('', views.device_list, name='device_list'),
    path('scan', views.device_list, name='device_list'),
    path('chart', views.chart, name='chart'),
]