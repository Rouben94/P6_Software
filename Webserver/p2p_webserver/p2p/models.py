from django.db import models

# Create your models here.

class Node(models.Model):
    def __init__(self, MAC, CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI):
        self.mac = MAC
        self.ch_array = [] 
        self.ch_array.append([CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI])

    def add_channel(self, CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI):
        self.ch_array.append([CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI])