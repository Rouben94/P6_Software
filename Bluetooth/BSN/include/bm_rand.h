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

#ifdef __cplusplus
}
#endif
#endif