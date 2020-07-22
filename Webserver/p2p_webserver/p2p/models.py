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
    number = mac = models.CharField(max_length=100, unique=True)
    mac = models.CharField(max_length=100, unique=True)

    class Meta:
        ordering = ["mac"]


class Channel(models.Model):
    ch = models.CharField(max_length=255, unique=True)
    capital = models.CharField(max_length=255)
    code = models.CharField(max_length=2, unique=True, primary_key=False)
    continent = models.ForeignKey('Continent', related_name='countries')
    population = models.PositiveIntegerField()
    area = PositiveIntegerField()
    class Meta:
        ordering = ["mac"]