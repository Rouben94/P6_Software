# Generated by Django 3.0.8 on 2020-07-23 07:26

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('p2p', '0007_auto_20200723_0925'),
    ]

    operations = [
        migrations.AlterField(
            model_name='channel',
            name='ch',
            field=models.CharField(max_length=255),
        ),
    ]
