from django import forms

MODE_CHOICES= [
    ('0', '1 Mbit/s Nordic radio mode'),
    ('1', '2 Mbit/s Nordic radio mode'),
    ('3', '1 Mbit/s BLE'),
    ('4', '2 Mbit/s BLE'),
    ('5', 'Long range 125 kbit/s TX'),
    ('6', 'Long range 500 kbit/s TX'),
    ('15', 'IEEE 802.15.4-2006 250 kbit/s')
    ]

CCA_CA_CHOICES= [
    ('0', 'Off'),
    ('1', 'Ed Mode'),
    ('2', 'Carrier Mode'),
    ('3', 'Carrier and Ed Mode'),
    ('4', 'Carrier or Ed Mode')
    ]

TX_CHOICES= [
    ('0', '0 dB'),
    ('1', '1 dB'),
    ('2', '2 dB'),
    ('3', '3 dB'),
    ('4', '4 dB'),
    ('5', '5 dB'),
    ('6', '6 dB'),
    ('7', '7 dB'),
    ('8', '8 dB'),
    ]

PORT_CHOICES= [
    ('disconnect', 'Disconnect'),
    ('COM1', 'COM 1'),
    ('COM2', 'COM 2'),
    ('COM3', 'COM 3'),
    ('COM4', 'COM 4'),
    ('COM5', 'COM 5'),
    ('COM6', 'COM 6'),
    ('COM7', 'COM 7'),
    ('COM8', 'COM 8'),
    ('COM9', 'COM 9'),
    ('COM10', 'COM 10'),
    ('COM11', 'COM 11'),
    ('COM12', 'COM 12'),
    ('COM13', 'COM 13'),
    ('COM14', 'COM 14'),
    ('COM15', 'COM 15'),
    ('COM16', 'COM 16'),
    ('COM17', 'COM 17'),
    ('COM18', 'COM 18'),
    ('COM19', 'COM 19'),
    ('COM20', 'COM 20'),
    ('COM21', 'COM 21'),
    ('COM22', 'COM 22'),
    ('COM23', 'COM 23'),
    ('COM24', 'COM 24'),
    ('COM25', 'COM 25'),
    ('COM26', 'COM 26'),
    ('COM27', 'COM 27'),
    ('COM28', 'COM 28'),
    ('COM29', 'COM 29'),
    ('COM30', 'COM 30'),
    ('COM31', 'COM 31'),
    ('COM32', 'COM 32'),
    ('COM33', 'COM 33'),
    ('COM34', 'COM 34'),
    ('COM35', 'COM 35'),
    ('COM36', 'COM 36'),
    ('COM37', 'COM 37'),
    ('COM38', 'COM 38'),
    ('COM39', 'COM 39'),
    ('COM40', 'COM 40'),
    ('COM41', 'COM 41'),
    ('COM42', 'COM 42'),
    ('COM43', 'COM 43'),
    ('COM44', 'COM 44'),
    ('COM45', 'COM 45'),
    ('COM46', 'COM 46'),
    ('COM47', 'COM 47'),
    ('COM48', 'COM 48'),
    ('COM49', 'COM 49')
    ]

class ParamForm(forms.Form):
    start_channel = forms.IntegerField(label_suffix='' , min_value=1, max_value=40)
    stop_channel = forms.IntegerField(label_suffix='', min_value=1, max_value=40)
    mode = forms.CharField(label_suffix='' , widget=forms.Select(choices=MODE_CHOICES))
    size = forms.IntegerField(label_suffix='' , min_value=1, max_value=250) # 1 - 250
    ccma_ca = forms.CharField(label_suffix='' , widget=forms.Select(choices=CCA_CA_CHOICES))
    tx_power = forms.CharField(label_suffix='' , widget=forms.Select(choices=TX_CHOICES))

class PortForm(forms.Form):
    port = forms.CharField(label='Port: ', label_suffix='', widget=forms.Select(choices=PORT_CHOICES))
