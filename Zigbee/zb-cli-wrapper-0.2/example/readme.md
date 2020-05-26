
# MQTT Zigbee gateway application

The MQTT Zigbee Gateway application uses Zigbee CLI example to control Zigbee devices through the MQTT protocol.
The connection and communication with CLI is handled through the zb_cli_wrapper module.

The gateway maps the basic Zigbee clusters of devices to the MQTT topics.
- The devices are specified in the [configuration file](#Configuration file).
- The MQTT topics are addresses with a specific [topic structure pattern](#Topic structure pattern).
They are used to store resources that are then passed to the gateway application.


## Configuration file
The application reads the following configuration data from the `config.yaml` file:
- MQTT connection parameters
- List of devices to control with the device parameters
- MQTT topics to subscribe to
- CLI configuration

## Topic structure pattern

The MQTT topics have the following pattern:
``
home/device_alias/cluster_name/attribute_name
``

In this pattern, the following values are used in the gateway application:
- `device_alias` is the unique name for the device
  - The device is specified with a long EUI64 address and an endpoint. The same address and different endpoints result in a different device.
- `cluster_name` is the name of the Zigbee cluster, and `attribute_name` is the name of the Zigbee attribute:
  - `on_off` represents the On/Off cluster.
      - `state` is the OnOff attribute.
  - `lvl_ctrl` represents the Level cluster.
      - `level` is the CurrentLevel attribute.
  - `color_ctrl` represents the Color Control cluster.
      - MQTT stores color-related information in the RGB color space, and the application translates it to the corresponding Zigbee HSV value.
          For this reason, MQTT stores color values in three separate attribute names: `r`, `g`, `b` (red, green, and blue, respectively).
  - `door_lock` represents the Door Lock cluster.
      - `lock_state` is LockState attribute.

The application subscribes to the MQTT topics specified in the `config.yaml` file. For each topic value change, the application sends a ZCL command.

## Prerequisites

The application requires the following software:
- Python (version 3.6 or newer) with several Python modules.
    - To install the required Python modules, run the following command:
    ```
    pip install paho-mqtt pyyaml setuptools
    ```
    - zb-cli-wrapper module. To install the zb-cli-wrapper module, run the `setup.py` script from the zb-cli-wrapper package directory:
    ```
    python setup.py install
    ```
## Configuring the application behavior

See the following table for the default application behavior and recommended actions for configuring it.

| Step  | Application behavior      | Recommended actions |
|-------|---------------------------|---------------------|
| 1     | Tries to connect to the broker present on the local host. It closes if it fails to connect to the broker. |  Make sure the broker is running at the local host, or configure the broker address in `config.yaml`. |
| 2     | Tries to connect to the CLI device by its SEGGER number, CDC serial number, or the com port number specified in `config.yaml`. It closes if it fails to open the com port.    |   Set the correct value of your board SEGGER number or serial number, or of the com port your board is connected to.|
| 3     | Uses the Zigbee channels list in `config.yaml`.   |   Set the correct channels in range from 11 to 26. If you are not sure which channel to use, set all of them.|
| 4     | Uses the Zigbee role defined in `config.yaml`.    |   Set the role of the CLI device you need: if you have a Zigbee network, use the router role: `'zr'`; if you do not have a Zigbee network, use the coordinator role to create network: `'zc'`.    |
| 5     | Uses the pre-defined Zigbee devices.              |   Provide the correct long addresses and endpoints of devices you want to control. Check `readme.md` of the `zb_cli_wrapper` module to learn how to detect Zigbee devices and get their addresses and endpoints.<br>For every device you want to control provide topics to use. Check [this section](#topic-structure-pattern) to learn how to generate topics.  |

## Running the application

You can run the application by calling Python with the application file name as argument:
```bash
python MQTT_Zigbee_gateway.py
```