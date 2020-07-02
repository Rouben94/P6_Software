cmake_minimum_required(VERSION 3.13.1)

#Board selection
set(BOARD nrf52840dk_nrf52840)
#set(BOARD nrf5340pdk_nrf5340_cpunet)
#set(BOARD nrf52840dongle_nrf52840)

#For zephyr build
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(NONE)

target_sources(app PRIVATE src/main.cpp src/simple_nrf_radio.cpp)
target_include_directories(app PRIVATE include)