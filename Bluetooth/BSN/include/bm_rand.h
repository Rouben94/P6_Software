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
extern uint32_t bm_rand_msg_ts[];     // Randomly generated Message Timestamps

/* Initialize Random Data */
void bm_rand_init();

/* Function to get Random values */
void bm_rand_get(void *dst, int len);

/* Function to implement bubble sort */
void bm_rand32_bubbleSort(uint32_t arr[], uint32_t n);


#ifdef __cplusplus
}
#endif
#endif