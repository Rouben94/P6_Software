#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_RAND_H
#define BM_RAND_H

#include "bm_config.h"

#ifdef ZEPHYR
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#endif

extern uint32_t bm_rand_32;           // Randomly generated 4 bytes
extern uint8_t bm_rand_250_byte[250]; // Randomly generated 250 bytes

/* Initialize Random Data */
void bm_rand_init();

#ifdef __cplusplus
}
#endif
#endif