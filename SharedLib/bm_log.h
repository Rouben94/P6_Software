/*
This file is part of Benchamrk-Shared-Library.

Benchamrk-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchamrk-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchamrk-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR Part ZEPHYR_BLE_MESH 	   :   Raffael Anklin        */
/* AUTHOR Part NRF_SDK_ZIGBEE 	 :     Cyrill Horath      */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_LOG_H
#define BM_LOG_H

#include "bm_config.h"




/* Struct for benchmark message information */
typedef struct
{
  uint16_t message_id;
  uint64_t net_time;
  uint64_t ack_net_time;
  uint8_t number_of_hops;
  uint8_t rssi;
  uint16_t src_addr;
  uint16_t dst_addr;
  uint16_t group_addr;
  uint16_t data_size;
} __attribute__((packed)) bm_message_info;

//extern bm_message_info message_info[NUMBER_OF_BENCHMARK_REPORT_MESSAGES];

/* Array of structs to save benchmark message info to */
extern bm_message_info message_info[NUMBER_OF_BENCHMARK_REPORT_MESSAGES];

void bm_log_init();

void bm_log_clear_ram();

#ifdef ZEPHYR_BLE_MESH
void bm_log_clear_storage_flash();
#endif

void bm_log_append_ram(bm_message_info message);

void bm_log_save_to_flash();

uint32_t bm_log_load_from_flash();

void bm_log_clear_flash();

//void bm_clear_log();
//
//void bm_append_log(bm_message_info msginfo);

#ifdef __cplusplus
}
#endif
#endif