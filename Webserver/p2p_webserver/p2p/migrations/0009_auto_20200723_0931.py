# Generated by Django 3.0.8 on 2020-07-23 07:31

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('p2p', '0008_auto_20200723_0926'),
    ]

    operations = [
        migrations.AlterField(
            model_name='node',
            name='mac',
            field=models.CharField(max_length=255),
        ),
    ]