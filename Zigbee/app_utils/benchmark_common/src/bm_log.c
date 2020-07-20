#include "bm_log.h"
#include "bm_cli.h"
#include "bm_config.h"
#ifdef NRF_SDK_Zigbee
#include "bm_flash_save.h"
#elif defined ZEPHYR_BLE_MESH
#include <zephyr.h>
#include <drivers/flash.h>
#include <storage/flash_map.h>
#include <device.h>
#include <stdio.h>
/* Flash Area "image_1" is used by the MCU Bootloader for DFU Firmware Upgrade. its save to use it when there is no DFU in Process... 
see: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/ug_bootloader.html#flash-partitions-used-by-mcuboot */
#define FLASH_OFFSET FLASH_AREA_OFFSET(image_1) // Regarding DTS file size is define in nRF52 to 421’888 Bytes
#define FLASH_PAGE_SIZE   4096 // Since the Page size is 4096 bytes we need a maximum of 3000*40bytes = 120'000 ~ 30 PAGES
#define FLASH_PAGES   30 // Since the Page size is 4096 bytes we need a maximum of 3000*40bytes = 120'000 ~ 30 PAGES
struct device *flash_dev;
#endif

#include <string.h>

// Save the Loged Data to
bm_message_info message_info[NUMBER_OF_BENCHMARK_REPORT_MESSAGES] = {0};

uint32_t bm_message_cnt; // Count the Log Entry

void bm_log_clear_ram() {
  bm_message_cnt = 0;
  memset(message_info, 0, sizeof(message_info));
}

void bm_log_append_ram(bm_message_info message) {
  if (bm_message_cnt > sizeof(message_info) - 1) {
    bm_cli_log("Log Buffer Overflow... Ignoring Data\n");
    return;
  }
  message_info[bm_message_cnt] = message;
  bm_message_cnt++;
}

void bm_log_save_to_flash() {
  #ifdef NRF_SDK_Zigbee
  uint16_t bm_message_cnt_flash = 0;
  while (bm_message_cnt_flash < bm_message_cnt) {
    flash_write(*((Measurement *)&message_info[bm_message_cnt_flash]));
    bm_cli_log("Message write to Flash Message Number: %d\n", bm_message_cnt_flash);
    bm_message_cnt_flash++;
  }
  bm_cli_log("Write to Flash finished. %d data packets saved\n", bm_message_cnt_flash);
  #elif defined ZEPHYR_BLE_MESH
  flash_write_protection_set(flash_dev, false);
  flash_write(flash_dev, FLASH_OFFSET, message_info, sizeof(message_info));
  flash_write_protection_set(flash_dev, true); // enable is recommended
  #endif
}

#ifdef NRF_SDK_Zigbee
/* Callback function to read Benchmark Message Info data from Flash. */
void bm_log_load_from_flash_cb(Measurement *data) {
  message_info[bm_message_cnt] = *((bm_message_info *)data);
  bm_message_cnt++;
}
#endif

uint32_t bm_log_load_from_flash() {
  bm_message_cnt = 0;
  #ifdef NRF_SDK_Zigbee
  flash_read();
  #elif defined ZEPHYR_BLE_MESH
  if (flash_read(flash_dev, FLASH_OFFSET, message_info, sizeof(message_info)) != 0) {
			bm_cli_log("Flash read failed!\n");
			return 0;
		}
  bm_message_cnt = sizeof(message_info);
  #endif
  bm_cli_log("Read data from Flash done\n");
  return bm_message_cnt;
}

void bm_log_clear_flash() {
  #ifdef NRF_SDK_Zigbee
  flash_delete();
  bm_cli_log("Flash Data deleted\n");
  #elif defined ZEPHYR_BLE_MESH
  flash_write_protection_set(flash_dev, false);
	if (flash_erase(flash_dev, FLASH_OFFSET, FLASH_PAGE_SIZE * FLASH_PAGES) != 0) {
		bm_cli_log("Flash erase failed!\n");
	} else {
		bm_cli_log("Flash erase succeeded!\n");
	}
  flash_write_protection_set(flash_dev, true); // enable is recommended
  #endif
}

void bm_log_init() {
  #ifdef NRF_SDK_Zigbee
  flash_save_init(bm_log_load_from_flash_cb);
  #elif defined ZEPHYR_BLE_MESH
  flash_dev = device_get_binding(DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
	if (!flash_dev) 
  {
		bm_cli_log("Nordic nRF5 flash driver was not found!\n");
		return;
	}
  #endif
}