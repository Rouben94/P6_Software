#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.13.1)

#Board selection
set(BOARD nrf52840dk_nrf52840)
#set(BOARD nrf5340_dk_nrf5340_cpunet)
#set(BOARD nrf52840dongle_nrf52840)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project("Zigbee light bulb")

# NORDIC SDK APP START
target_sources(app PRIVATE
  src/main.c
)
# NORDIC SDK APP END

# Include all files in folder src
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
