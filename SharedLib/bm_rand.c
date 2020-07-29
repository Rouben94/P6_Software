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


#include "bm_rand.h"
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_timesync.h"

#include <stdlib.h>

#ifdef ZEPHYR_BLE_MESH
#include <kernel.h>
#include <zephyr.h>
#elif defined NRF_SDK_ZIGBEE
#include "nrf52840.h"
#include "zboss_api_core.h"
#endif

uint32_t bm_rand_32 = 0;       // Randomly generated 4 bytes
uint32_t bm_rand_msg_ts[1000]; // Randomly generated Message Timestamps

void bm_rand_get(void *dst, int len) {
  uint64_t start_ts = synctimer_getSyncTime();
  NRF_RNG->TASKS_STOP = false; // Start Random Number Generator
  NRF_RNG->TASKS_START = true; // Start Random Number Generator
  NRF_RNG->CONFIG = true;      // Turn on BIAS Correction

  for (uint32_t i = 0; i < len; i++) {
    bm_sleep(1);
    //    uint32_t cnt = 0;
    //    while (cnt < 1000)
    //    {
    //      cnt++;
    //      __NOP(); //Wait for Random Data -> ~120us Run time per byte with bias correction. Uniform distribution of 0 and 1 is guaranteed. Time to generate a byte cannot be guaranteed.
    //    }
    *((uint8_t *)dst + i) = NRF_RNG->VALUE;
    //((uint8_t *)dst)[i] = (uint8_t)NRF_RNG->VALUE;
    //bm_cli_log("Rand Value: %u\n",((uint8_t *)dst)[i]);
    NRF_RNG->TASKS_START = true; // Start Random Number Generator -> The Low Level Driver always stops the RNG (shorts set?)
  }
  NRF_RNG->TASKS_STOP = true; // Stop Random Number Generator
  bm_cli_log("Rand Generated in %u ms\n", (uint32_t)(synctimer_getSyncTime() - start_ts) / 1000);
}

void bm_swap(uint32_t *xp, uint32_t *yp) {
  uint32_t temp = *xp;
  *xp = *yp;
  *yp = temp;
}

// A function to implement bubble sort (small to big)
void bm_rand32_bubbleSort(uint32_t arr[], int len) {
  uint64_t start_ts = synctimer_getSyncTime();
  uint32_t i, j;
  for (i = 0; i < len - 1; i++) {
    // Last i elements are already in place
    for (j = 0; j < len - i - 1; j++) {
      //bm_cli_log("%u\n", j);
      if (arr[j] > arr[j + 1]) {
        bm_swap(&arr[j], &arr[j + 1]);
      }
    }
  }
  bm_cli_log("Sorted in %u ms\n", (uint32_t)(synctimer_getSyncTime() - start_ts) / 1000);
}

void bm_rand_init() {
  bm_rand_get(&bm_rand_32, sizeof(bm_rand_32));
  bm_cli_log("32bit Random value initalized with: %u\n", bm_rand_32);
  return;
}

void bm_rand_init_message_ts() {
  bm_rand_get(&bm_rand_msg_ts, bm_params.benchmark_packet_cnt * sizeof(uint32_t)); // Genrate Random Values
  bm_rand32_bubbleSort(bm_rand_msg_ts, bm_params.benchmark_packet_cnt);            // Sort Random Array
  // Convert to Timesstamps relativ to benchmark Time
  for (int i = 0; i < bm_params.benchmark_packet_cnt; i++) {
    bm_rand_msg_ts[i] = (uint32_t)(((double)bm_rand_msg_ts[i] / UINT32_MAX) * (double)bm_params.benchmark_time_s * 1e6); // Be aware of not loosing accuracy
    bm_cli_log("Value is %u us\n", bm_rand_msg_ts[i]);
  }
  return;
}