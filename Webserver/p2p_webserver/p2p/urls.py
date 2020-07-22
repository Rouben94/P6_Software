from django.urls import path
from . import views

urlpatterns = [
    path('', views.node_list, name='node_list'),
    path('node', views.node_list, name='node_list'),
    path('chart', views.chart, name='chart'),
    path('connect', views.connect, name='connect'),
    path('info', views.info, name='info'),
]