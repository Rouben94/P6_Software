# Generated by Django 3.0.8 on 2020-07-23 07:25

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('p2p', '0006_remove_node_number'),
    ]

    operations = [
        migrations.AlterField(
            model_name='node',
            name='mac',
            field=models.CharField(max_length=100),
        ),
    ]
