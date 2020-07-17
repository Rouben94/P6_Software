#include "bm_rand.h"
#include "bm_cli.h"
#include "bm_config.h"

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#include "nrf52840.h"
#include "zboss_api_core.h"
#endif

uint32_t bm_rand_32 = 0;       // Randomly generated 4 bytes
uint8_t bm_rand_250_byte[250]; // Randomly generated 250 bytes

void bm_rand_get(void *dst, uint8_t len) {

  NRF_RNG->TASKS_STOP = false; // Start Random Number Generator
  NRF_RNG->TASKS_START = true; // Start Random Number Generator
  NRF_RNG->CONFIG = true;      // Turn on BIAS Correction

  for (uint8_t i = 0; i < len; i++) {
        uint16_t cnt = 0;
        while(cnt < 1000){
          cnt++;
            __NOP(); //Wait for Random Data -> ~120us Run time per byte with bias correction. Uniform distribution of 0 and 1 is guaranteed. Time to generate a byte cannot be guaranteed.
        }
    ((uint8_t *)dst)[i] = NRF_RNG->VALUE;
  }
  NRF_RNG->TASKS_STOP = true; // Stop Random Number Generator
}

void bm_rand_init() {
  bm_rand_get(&bm_rand_32, sizeof(bm_rand_32));
  bm_cli_log("32bit Random value initalized with: %u\n", bm_rand_32);
  bm_rand_get(bm_rand_250_byte, sizeof(bm_rand_250_byte));
  bm_cli_log("250Byte Random Data initalized\n");  
  return;
}