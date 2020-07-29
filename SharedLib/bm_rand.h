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

/* AUTHOR   :   Raffael Anklin        */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_RAND_H
#define BM_RAND_H

#include "bm_config.h"

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#endif

extern uint32_t bm_rand_32;           // Randomly generated 4 bytes
extern uint32_t bm_rand_msg_ts[1000];     // Randomly generated Message Timestamps

/* Initialize Random Data */
void bm_rand_init();

/* Function to get Random values */
void bm_rand_get(void *dst, int len);

/* Function to implement bubble sort */
void bm_rand32_bubbleSort(uint32_t arr[], int len);

void bm_rand_init_message_ts();


#ifdef __cplusplus
}
#endif
#endif