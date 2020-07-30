from django.db import models
from django.conf import settings


class Node(models.Model):
    self.ch = []
    def __init__(self, MAC, CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI):
        self.ch.append(Channel(MAC, CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI)) 

    def add_channel(MAC, CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI):
        self.ch.append(Channel(MAC, CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI))

class Channel(models.Model):
    def __init__(self, CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI):
        self.txpkt_cnt      = TxPKT_CNT
        self.crcok_cnt      = CRCOK_CNT
        self.crcerr_cnt     = CRCERR_CNT
        self.avf_sig_rssi   = Avg_SIG_RSSI
        self.avg_noise_rssi = AVG_NOISE_RSSI