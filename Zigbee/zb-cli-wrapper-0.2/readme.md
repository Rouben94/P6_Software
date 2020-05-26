# Zigbee CLI wrapper

Zigbee CLI wrapper (zb_cli_wrapper) is a Python package for the nRF5 SDK for Zigbee that includes a custom-made Python wrapper for simplifying communication with the Zigbee CLI and improving the control of the Zigbee network.

## Prerequisites

Make sure you have the following hardware and software before using the wrapper.

### Hardware

The wrapper is to be used with the [Zigbee CLI Agent example](https://www.nordicsemi.com/DocLib/Content/SDK_Doc/Thread_SDK/v2-0-0/zigbee_example_cli_agent),
and it requires the same hardware as this example. Make sure the hardware supports Zigbee.

### Software
You need the following software:

- [Python 3.6](https://www.python.org/downloads/) or newer
- [pip](https://pip.pypa.io/en/stable/installing.html)
- setuptools (latest version)
    - Install them with the following command:
        ```
        `pip install -U setuptools`
        ```
 
## Installing the wrapper


To install the package, use pip:
```python
pip install zb-cli-wrapper
```

You can also [download package from PyPI](https://pypi.org/project/zb-cli-wrapper) and run the following command in the package source folder to install it:
```
python setup.py install
```

This will also install all required packages.

## Configuring the wrapper

To use wrapper, you must configure both hardware and software by completing the following steps:
- flashing the board
- creating connection with CLI

### Flashing the board

Flash the [Zigbee CLI Agent example](https://www.nordicsemi.com/DocLib/Content/SDK_Doc/Thread_SDK/v2-0-0/zigbee_example_cli_agent) on the board you are using, as described on the example page.

For flashing any nRF5 board, you can use the [nRF Connect for Desktop](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Connect-for-desktop) application, available for Windows, Linux and macOS. The hex you need can be found in `/hex` directory.

You can also flash your board using [nrfutil](https://github.com/NordicSemiconductor/pc-nrfutil) or using python module [pynrfjprog](https://github.com/NordicSemiconductor/pynrfjprog).

### Creating connection with CLI

For every CLI device that you want to communicate with by using zb_cli_wrapper, use an instance of the `ZbCliDevice` class, located in the `zb_cli_dev.py` file. 
Object of this class is used to write and read from the board that runs the Zigbee CLI Agent example.

The `ZbCliDevice` object offers methods imported from files located at `zb_cli_wrapper/src/utils/cmd_wrappers/zigbee/`: `bdb.py`, `zcl.py`, `zdo.py`, `log.py`.
These methods can be accessed as `<object>.bdb.method`, `<object>.zcl.method`, `<object>.zdo.method`, respectively.
See the example in the following sections.

To create connection with CLI:
1. Import the `zb_cli_wrapper` package once it is installed:
```
from zb_cli_wrapper.zb_cli_dev import ZbCliDevice
```
2. Create a `ZbCliDevice` object:
    1. Specify how the object will find the com port to which your nRF board is connected by passing one of the following information as an argument for constructor:
        - If your onboard SEGGER J-Link is connected to your PC, use SEGGER number: `'segger': '680000000'`.
        - If your onboard nRF is connected directly to PC, use the serial number: `'cdc_serial: 'EF0000000000'`.
        - If you want to specify the com port manually, use the com port number: `'com_port': 'COM20'`.
    2. Run the following command (replace the SEGGER number depending on your choice in step 1):
        ```
        cli_instance = ZbCliDevice({'segger': '680000000'})
        ```
     
**Note:** Make sure you use correct numbers. Regardless of whether you use a development kit with onboard J-Link, such as [nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK),
or a nRF USB Dongle, such as [nRF52840 Dongle](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-Dongle), you can read the SEGGER number or the serial number, respectively, from the sticker on the board.

When the object is created, it automatically tries to connect to the com_port to which the CLI is connected.

## Using the wrapper

To start using the wrapper, complete the following steps:
- connecting to network (or creating a network)
- discovering the device present in the network and getting its addresses and endpoints
- reading the attribute value of the device
- controlling the device using CLI

At the end, you can close CLI and the connection with the board.

**Note:** Light bulb is used as the example device in this section.

### Connecting to network

To connect to network (or create a network):
1. Connect to the existing Zigbee network or create a new one by setting one of the following CLI roles:
- To set role as router (join network): `cli_instance.bdb.role = 'zr'`
- To set role as coordinator (create network): `cli_instance.bdb.role = 'zc'`
2. Set channel or channels that CLI will use. For example, to set channels 20 and 24:
```python
cli_instance.bdb.channel = [20, 24]
```
**Note:** If you don't know the channel number, set all Zigbee channels (in range 11-26).

Join or create a network call:
```
cli_instance.bdb.start()
```

As soon as board's `NETWORK_LED` is turned on, CLI is commissioned and ready to control Zigbee devices. To wait for CLI to join network, call:
```python
cli_instance.wait_until_connected(timeout=10)
```
This call blocks until CLI is connected or times out, returns `None` if CLI is not connected, and self if connected.

When connected, you can send commands and read responses.

### Discovering the device addresses
 
To control a Zigbee device, you must know its long address (the short address can change).
To get to know the short address, use the discovery process. Then, resolve the short address into a long one.

The device can be specified by the input and output clusters. For example, for the dimmable light bulb:
- input clusters:
    - On/Off cluster (Cluster ID: 0x0006)
    - Level cluster (Cluster ID: 0x0008)
- output clusters:
    - no output clusters

**Note:** For all Zigbee-related information, including cluster ID and attribute, see [ZCL Specification](https://www.zigbee.org/zigbee-for-developers/zigbee-3-0/).

#### Getting the device short address

Before discover a device, import the basic ZCL-related constants to make the discovery process easier.
The constants are defined in the `constants.py` file located at `/zb_cli_wrapper/src/utils/cmd_wrappers/zigbee/constants.py`.
To import constants to the script, run the following command:
```python
from zb_cli_wrapper.src.utils.cmd_wrappers.zigbee import constants
```

To discover a Zigbee device like a light bulb, use the match descriptor request.
For example, use the match_desc function from zdo:
```python
response = cli_instance.zdo.match_desc([constants.ON_OFF_CLUSTER, constants.LVL_CTRL_CLUSTER], [])
>>> response
[('C2BA', '10')]
```

This function returns a list of tuples as response. A single tuple stores information about
a single device as `(device_short_addr, device_endpoint)`.

It may take up about minute to find all matching devices. The list can be long.

#### Resolving a short address into a long address

To resolve a short address (stored in `response[0][0]`) into a long address, run the following command:
```python
light_bulb_eui64 = cli_instance.zdo.ieee_addr(response[0][0])
>>> hex(light_bulb_eui64)
'0xb010ead45b1b20c'
```

### Reading from the device

To read the device cluster attributes, use `zcl.readattr`:
```
response_attr = cli.zcl.readattr(eui64, attr, ep)
```
The function returns the read data as an instance of the `Attribute` class. It also takes an instance of the same class as argument. For every cluster attribute to be read, create one instance of the `Attribute` class.

To import a class, add at the beginning of your script:
```
from zb_cli_wrapper.utils.zigbee_classes.clusters.attribute import Attribute
```
To read the brightness level of a light bulb or another of its attributes, create an `Attribute` object:
```
bulb_lvl_attr = Attribute(cluster=constants.LVL_CTRL_CLUSTER, id=constants.LVL_CTRL_CURR_LVL_ATTR, type=constants.UINT8_TYPE, name="Bulb level")
```
Here, `id=constants.LVL_CTRL_CURR_LVL_ATTR` corresponds to the `current level` attribute (which in a light bulb usually corresponds to the brightness of the bulb) and `type` determines the type of the attribute (defined in [ZCL Specification](https://www.zigbee.org/zigbee-for-developers/zigbee-3-0/)).

To read the `current level` attribute defined in `bulb_lvl_attr`, run the following command:
```python
response_attr = cli_instance.zcl.readattr(light_bulb_eui64, bulb_lvl_attr, ep=response[0][1])
>>> response_attr
Attribute Bulb level: 255
```

### Controlling the device

To change the brightness of the bulb, you must send the `Move to level` command defined in the [ZCL Specification](https://www.zigbee.org/zigbee-for-developers/zigbee-3-0/)).

To send ZCL commands, use `zcl.generic`:
```python
cli_instance.zcl.generic(eui64, ep, cluster, profile, cmd_id, payload=None)
```
In this command:
- `profile` determines the Zigbee application profile. As default, the standard Home Automation Profile (ID: 0x0104) is used.
- `cmd_id` is the ID of the command to be sent, as defined in the [ZCL Specification](https://www.zigbee.org/zigbee-for-developers/zigbee-3-0/)).
- `payload` is an additional argument if the specified command requires additional values.

**Note**: `Payload` is given as a list of tuples, where a tuple is: (value, type_of_value).

To set the light bulb brightness to 50%, run the following command:
```python
cli_instance.zcl.generic(eui64=light_bulb_eui64, ep=response[0][1], cluster=constants.LVL_CTRL_CLUSTER, profile=constants.DEFAULT_ZIGBEE_PROFILE_ID, cmd_id=constants.LVL_CTRL_MV_TO_LVL_CMD, payload=[(0x7F, constants.UINT8_TYPE), (1, constants.UINT16_TYPE)])
```

Here, `payload` is given as a new value of the `current level` attribute: `7F` (corresponds to 127 in decimal, which is 50% of 255, that is the maximum value of `current level` attribute) and transition time equals `1` (corresponds to 1 in decimal, which is equal to 0.1 seconds of transition time).

By reading the cluster attribute again, you can confirm the change of the attribute value.

### Closing CLI and the connection with the board

Use `close_cli()` method to softly close CLI and the connection with the board:
```python
cli_instance.close_cli()
```