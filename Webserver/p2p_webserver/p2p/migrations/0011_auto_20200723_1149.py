# Generated by Django 3.0.8 on 2020-07-23 09:49

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('p2p', '0010_auto_20200723_1110'),
    ]

    operations = [
        migrations.AlterModelOptions(
            name='channel',
            options={'ordering': ['ch']},
        ),
        migrations.RenameField(
            model_name='channel',
            old_name='avg_noise_rssi',
            new_name='packetloss',
        ),
        migrations.RenameField(
            model_name='channel',
            old_name='avg_sig_rssi',
            new_name='signal_to_noise_ratio',
        ),
        migrations.RemoveField(
            model_name='channel',
            name='crcerr_cnt',
        ),
        migrations.RemoveField(
            model_name='channel',
            name='crcok_cnt',
        ),
        migrations.RemoveField(
            model_name='channel',
            name='node',
        ),
        migrations.RemoveField(
            model_name='channel',
            name='txpkt_cnt',
        ),
    ]