# Bluetooth Mesh Benchmark

This example is for benchmarking a Bluetooth Mesh Network. 

**Table of contents**
- [Preparing for building](@ref Bluetooth_Mesh_Building)

## Preparing for building @anchor Bluetooth_Mesh_Preparing_for_building

To prepare for building the following procedure needs to be followed:

1. Download the nRF5 SDK v16
2. Download the nRF5 SDK for Mesh v4.1.0 and extract it next to the nRF5 SDK v16
3. Modify the file ../nRF5_SDK_for_Mesh_Root/examples/CMakeLists.txt on line 25 with set(BLUETOOTH_BENCHMARK_SOURCE_DIR "path to Bluetooth Benchmark dir")

## Building @anchor Bluetooth_Mesh_Building

To build the following procedure needs to be followed:

1. open the ../nRF5_SDK_for_Mesh_Root in terminal or vscode terminal
2. run `cmake -G Ninja -DTOOLCHAIN=gccarmemb -DPLATFORM=nrf52840_xxAA -DCMAKE_BUILD_TYPE=Debug` (or see https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.meshsdk.v4.1.0/md_doc_getting_started_getting_started.html for other arguments)
3. Open the Segger Solution Project to Build and Flash or use vscode to work on it. Compiled binary is under build/...


