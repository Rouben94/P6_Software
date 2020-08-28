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
extern "C"
{
#endif

#ifndef BM_RAND_H
#define BM_RAND_H

#include "bm_config.h"

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#endif

    extern uint32_t bm_rand_32;           // Randomly generated 4 bytes
    extern uint64_t bm_rand_msg_ts[1000]; // Randomly generated Message Timestamps

    /* Initialize Random Data */
    void bm_rand_init();

    /* Function to get Random values */
    void bm_rand_get(void *dst, int len);

    /* Function to implement bubble sort */
    void bm_rand64_bubbleSort(uint64_t arr[], int len);

    void bm_rand_init_message_ts();

#if defined ZEPHYR_BLE_MESH || defined NRF_SDK_ZIGBEE || defined NRF_SDK_MESH
    /** Defines for Random Transaction Events in Benchmark
     * Gerated by Random.org. Uses a lot of RAM (be aware) **/
    extern uint16_t rand16_26_1000[25][1000];
#endif //defined ZEPHYR_BLE_MESH || defined NRF_SDK_ZIGBEE

#ifdef NRF_SDK_THREAD
    /** Defines for Random Transaction Events in Benchmark
     * Gerated by Random.org. Uses a lot of RAM (be aware) **/
    extern const uint16_t rand16_26_1000[25][1000];
#endif //NRF_SDK_THREAD

#ifdef __cplusplus
}
#endif
#endif