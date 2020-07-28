from django.db import models

# Create your models here.

""" class Node(models.Model):
    mac = models.CharField(max_length=100)
    ch_array = ArrayField(
        models.CharField(max_length=100),
        models.CharField(max_length=100),
        models.CharField(max_length=100),
        models.CharField(max_length=100),
        models.CharField(max_length=100),
        models.CharField(max_length=100)
    )

    def add_channel(self, CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI):
        self.ch_array.append([CH, TxPKT_CNT, CRCOK_CNT, CRCERR_CNT, Avg_SIG_RSSI, AVG_NOISE_RSSI]) """

class Node(models.Model):
    mac = models.CharField(max_length=255, unique=False)

    def __str__(self):
        return self.mac


class Channel(models.Model):
    ch = models.IntegerField(unique=False)
    signal_to_noise_ratio = models.CharField(max_length=255)
    packetloss = models.CharField(max_length=255)
    node = models.ForeignKey(Node, on_delete=models.CASCADE)

    def __str__(self):
        return self.ch

    class Meta:
        ordering = ['ch']