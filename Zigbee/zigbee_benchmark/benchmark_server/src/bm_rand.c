#include "bm_rand.h"
#include "bm_cli.h"
#include "bm_config.h"

#ifdef ZEPHYR
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#include "nrf52840.h"
#include "zboss_api_core.h"
#endif

uint32_t bm_rand_32 = 0;       // Randomly generated 4 bytes
uint8_t bm_rand_250_byte[250]; // Randomly generated 250 bytes

void bm_rand_get(void *dst, uint8_t len) {

  // Zephyr Way
  //bt_rand(dst,len);

  // NRF Connect SDK Way
  NRF_RNG->TASKS_STOP = false; // Start Random Number Generator
  NRF_RNG->TASKS_START = true; // Start Random Number Generator
  NRF_RNG->CONFIG = true;      // Turn on BIAS Correction

  for (uint8_t i = 0; i < len; i++) {
        /*
        while(NRF_RNG->EVENTS_VALRDY == false || !NRF_RNG->TASKS_STOP){
            __NOP(); //Wait for Random Data -> ~120us Run time per byte with bias correction. Uniform distribution of 0 and 1 is guaranteed. Time to generate a byte cannot be guaranteed.
        }
        */
        uint16_t cnt = 0;
        while(cnt < 10){
          cnt++;
            __NOP(); //Wait for Random Data -> ~120us Run time per byte with bias correction. Uniform distribution of 0 and 1 is guaranteed. Time to generate a byte cannot be guaranteed.
        }
    ((uint8_t *)dst)[i] = NRF_RNG->VALUE;
  }
  NRF_RNG->TASKS_STOP = true; // Stop Random Number Generator



}

void bm_rand_init() {

  //bt_rand(&bm_rand_250_byte,sizeof(bm_rand_250_byte));
  #ifdef ZEPHYR
bm_rand_get(&bm_rand_32, sizeof(bm_rand_32));
  NRF_LOG_INFO("32bit Random value initalized with: %u\n", bm_rand_32);
  bm_rand_get(bm_rand_250_byte, sizeof(bm_rand_250_byte));
  NRF_LOG_INFO("250Byte Random Data initalized\n");
#elif defined NRF_SDK_Zigbee

bm_rand_get(&bm_rand_32, sizeof(bm_rand_32));
  NRF_LOG_INFO("32bit Random value initalized with: %u\n", bm_rand_32);
  bm_rand_get(bm_rand_250_byte, sizeof(bm_rand_250_byte));
  NRF_LOG_INFO("250Byte Random Data initalized\n");
#endif
  
  
  return;
}