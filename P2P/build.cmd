set ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
set GNUARMEMB_TOOLCHAIN_PATH=C:\gnuarmemb
set ZEPHYR_BASE=D:\Raffael\GitHub\P6_Software_local\ncs\zephyr

cd ZEPHYR_BASE

west build -b nrf5340_dk_nrf5340_cpunet

west flash