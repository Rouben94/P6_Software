from django.db import models
from django.conf import settings

class Device(models.Model):
    name = models.CharField(max_length=100)
    mac_addr = models.DecimalField(max_digits=16, decimal_places=0)
    channel = models.IntegerField()
    rssi = models.IntegerField()