# Generated by Django 3.0.8 on 2020-07-28 14:51

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('p2p', '0012_channel_node'),
    ]

    operations = [
        migrations.AlterField(
            model_name='channel',
            name='ch',
            field=models.IntegerField(),
        ),
    ]