cmake_minimum_required(VERSION 3.13.1)

set(NRF_DIR D:/Raffael/GitHub/P6_Software_local/ncs/nrf)

#Board selection
#set(BOARD nrf52840dk_nrf52840)
set(BOARD nrf5340_dk_nrf5340_cpunet)
#set(BOARD nrf52840_pca10059)

#For ncs build
include($ENV{ZEPHYR_BASE}/../nrf/cmake/boilerplate.cmake)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

#For zephyr build
#find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})


project(NONE)


target_sources(app PRIVATE src/main.cpp src/Simple_nrf_radio.cpp src/Timer_sync.c)
target_include_directories(app PRIVATE include)