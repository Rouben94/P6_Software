from django.db import models

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