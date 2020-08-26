# Zigbee

The Zigbee Benchmark Firmware is divided into 3 parts: bechmark_master, benchmark_server and benchmark_client.
Each part has its own Segger Embedded Studio (SES) solution.


The Zigbee Benchmark Firmware is developed with SES. SES has to be preinstalled on your local drive.
Please do the following steps to configure SES to build the existing projects.

- Clone this Repository (main directory P6_Software) including SharedLib folder.
- Install nRF5_SDK_for_Thread_and_Zigbee on your local drive.
- Set global path macro in SES to the SDK: nRF5_SDK_for_Thread_and_Zigbee_v...
	- Open SES and select Tools from the menu bar, then open options -> Building -> Build -> Global macros. 
	- Set the following macro to the path of your SDK: SDK=C:/user/Workspace/nRF5_SDK_for_Thread_and_Zigbee_v..
- Build and run solution.