# Generated by Django 3.0.8 on 2020-07-23 09:10

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('p2p', '0009_auto_20200723_0931'),
    ]

    operations = [
        migrations.AlterModelOptions(
            name='channel',
            options={'ordering': ['txpkt_cnt']},
        ),
    ]
