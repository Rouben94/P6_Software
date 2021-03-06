/*
This file is part of Benchmark-Shared-Library.

Benchmark-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchmark-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchmark-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR Part ZEPHYR_BLE_MESH 	   :   Raffael Anklin        */
/* AUTHOR Part NRF_SDK_ZIGBEE 	 :     Cyrill Horath      */

#include "bm_log.h"
#include "bm_cli.h"
#include "bm_config.h"
#if defined NRF_SDK_ZIGBEE || defined NRF_SDK_THREAD
#include "bm_flash_save.h"
#elif defined NRF_SDK_MESH
#include "nrf_drv_clock.h"
#include "bm_flash_save.h"
#elif defined ZEPHYR_BLE_MESH
#include <device.h>
#include <drivers/flash.h>
#include <stdio.h>
#include <storage/flash_map.h>
#include <zephyr.h>
/* Flash Area "image_1" is used by the MCU Bootloader for DFU Firmware Upgrade. its save to use it when there is no DFU in Process... 
see: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/ug_bootloader.html#flash-partitions-used-by-mcuboot */
#define FLASH_OFFSET FLASH_AREA_OFFSET(image_1)         // Regarding DTS file size is define in nRF52 to 421�??888 Bytes
#define FLASH_OFFSET_STORAGE FLASH_AREA_OFFSET(storage) // Regarding DTS file size is define in nRF52 to 32'768 Bytes = 8 Pages
#define FLASH_PAGE_SIZE 4096                            // Since the Page size is 4096 bytes we need a maximum of 3000*40bytes = 120'000 ~ 30 PAGES
#define FLASH_PAGES 30                                  // Since the Page size is 4096 bytes we need a maximum of 3000*40bytes = 120'000 ~ 30 PAGES
#define FLASH_PAGES_STORAGE 8                           // Since the Page size is 4096 bytes we need a maximum of 3000*40bytes = 120'000 ~ 30 PAGES
struct device *flash_dev;
uint32_t test = 0;
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
  //bm_cli_log("Debug Msg ID: %u \n %u%u \n %u%u \n %u \n %d \n%x \n%x\n %x\r\n",message.message_id,(uint32_t)(message.net_time>>32),(uint32_t)message.net_time,(uint32_t)(message.ack_net_time>>32),(uint32_t)message.ack_net_time,message.number_of_hops,message.rssi,message.src_addr,message.dst_addr,message.group_addr);
  /*bm_cli_log("%u\n%u %u\n%u %u\n%u\n%d\n%x\n%x\n%x\r\n",
  message.message_id,
  (uint32_t)(message.net_time>>32),
  (uint32_t)message.net_time,
  (uint32_t)(message.ack_net_time>>32),
  (uint32_t)message.ack_net_time,
  message.number_of_hops,
  message.rssi,
  message.src_addr,
  message.dst_addr,
  message.group_addr);
  */
  message_info[bm_message_cnt] = message;
  bm_cli_log("Log Entry added: %u %u ...\n", message_info[bm_message_cnt].message_id, (uint32_t)message_info[bm_message_cnt].net_time);
  bm_message_cnt++;
}

void bm_log_clear_flash() {
#if defined NRF_SDK_ZIGBEE || defined NRF_SDK_THREAD || defined NRF_SDK_MESH
  flash_delete();
  bm_cli_log("Flash Data deleted\n");
#elif defined ZEPHYR_BLE_MESH
  flash_write_protection_set(flash_dev, false);
  for (int i = 0; i < FLASH_PAGES; i++) {
    flash_erase(flash_dev, FLASH_OFFSET + FLASH_PAGE_SIZE * i, FLASH_PAGE_SIZE);
  }
  bm_cli_log("Flash erase succeeded!\n");
  flash_write_protection_set(flash_dev, true); // enable is recommended
#endif
}

#ifdef ZEPHYR_BLE_MESH
void bm_log_clear_storage_flash() {
  flash_write_protection_set(flash_dev, false);
  for (int i = 0; i < FLASH_PAGES_STORAGE; i++) {
    flash_erase(flash_dev, FLASH_OFFSET_STORAGE + FLASH_PAGE_SIZE * i, FLASH_PAGE_SIZE);
  }
  bm_cli_log("Storage Flash erase succeeded!\n");
  flash_write_protection_set(flash_dev, true); // enable is recommended
}
#endif

void bm_log_save_to_flash() {
#if defined NRF_SDK_ZIGBEE || defined NRF_SDK_THREAD || defined NRF_SDK_MESH
  uint16_t bm_message_cnt_flash = 0;
  while (bm_message_cnt_flash < bm_message_cnt) {
    flash_write(*((bm_message_info *)&message_info[bm_message_cnt_flash]));
    //    bm_cli_log("Message write to Flash Message Number: %d\n", bm_message_cnt_flash);
    bm_message_cnt_flash++;
  }
  bm_cli_log("Write to Flash finished. %d data packets saved\n", bm_message_cnt_flash);
#elif defined ZEPHYR_BLE_MESH
  int ret;
  uint32_t i, offset;
  //bm_log_clear_flash();
  ret = flash_write_protection_set(flash_dev, false);
  for (i = 0U; i < ARRAY_SIZE(message_info); i++) {
    if (message_info[i].net_time == 0) {
      bm_cli_log("Saved %u entries to flash\n", i);
      break;
    }
    offset = FLASH_OFFSET + i * sizeof(message_info[i]);
    if (flash_write(flash_dev, offset, &message_info[i],
            sizeof(message_info[i])) != 0) {
      printf("Flash write failed!\n");
      return;
    }
  }
  flash_write_protection_set(flash_dev, true); // enable is recommended
#endif
}

#if defined NRF_SDK_ZIGBEE || defined NRF_SDK_THREAD || defined NRF_SDK_MESH
/* Callback function to read Benchmark Message Info data from Flash. */
void bm_log_load_from_flash_cb(bm_message_info *data) {
  message_info[bm_message_cnt] = *((bm_message_info *)data);
  bm_cli_log("Data: %d %d", data->message_id, data->net_time);
  bm_message_cnt++;
}
#endif

uint32_t bm_log_load_from_flash() {
  bm_message_cnt = 0;
#if defined NRF_SDK_ZIGBEE || defined NRF_SDK_THREAD || defined NRF_SDK_MESH
  flash_read();
#elif defined ZEPHYR_BLE_MESH
  uint32_t i, offset;
  for (i = 0U; i < ARRAY_SIZE(message_info); i++) {
    offset = FLASH_OFFSET + i * sizeof(message_info[i]);
    flash_read(flash_dev, offset, &message_info[i], sizeof(message_info[i]));
    if (message_info[i].net_time == UINT64_MAX) {
      bm_cli_log("Read %u entries from flash\n", i);
      memset(&message_info[i], 0, sizeof(message_info[i])); // Delete Last entry
      break;
    }
  }
  bm_cli_log("Net Time: %u\n", (uint32_t)message_info[0].net_time);
  bm_cli_log("Net Time: %u\n", (uint32_t)message_info[1].net_time);
  bm_message_cnt = i;
#endif
  bm_cli_log("Read data from Flash done\n");
  return bm_message_cnt;
}

void bm_log_init() {
#if defined NRF_SDK_ZIGBEE || defined NRF_SDK_THREAD
  flash_save_init(bm_log_load_from_flash_cb);
#elif defined NRF_SDK_MESH
    // Initialize the clock. 
    /*
    ret_code_t rc = nrf_drv_clock_init();
    APP_ERROR_CHECK(rc);

    nrf_drv_clock_lfclk_request(NULL);

    // Wait for the clock to be ready. 
    while (!nrf_clock_lf_is_running()) {;}
    */
  flash_save_init(bm_log_load_from_flash_cb);
#elif defined ZEPHYR_BLE_MESH
  flash_dev = device_get_binding(DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
  if (!flash_dev) {
    bm_cli_log("Nordic nRF5 flash driver was not found!\n");
    return;
  }
  bm_cli_log("FLASH AREA is : %x\n", FLASH_OFFSET);
#endif
}