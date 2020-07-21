/*
 * Zigbee Benchmark Master
 * Acting as a Zigbee Coordinator
 */

#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

uint32_t LSB_MAC_Address;

/**@brief Function for application main entry. */
int main(void) {

#ifdef NRF_SDK_Zigbee
  /* Function for initializing the nrf log module.*/
  /* To Enable Loging before the Mesh stack is run change in sdk_config -> NRF_LOG_DEFERRED = 0 (Performance is worse but logging is possibel) */
  //    ret_code_t err_code = NRF_LOG_INIT(NULL);
  //    APP_ERROR_CHECK(err_code);
  //    NRF_LOG_DEFAULT_BACKENDS_INIT();

  bm_cli_log_init(); /* Initialize the Zigbee LOG subsystem */

//  bm_cli_init(); /* Initialize the Zigbee CLI subsystem */


#endif

  // Init MAC Address
  LSB_MAC_Address = NRF_FICR->DEVICEADDR[1];
  bm_cli_log("Preprogrammed Randomly Static MAC-Address (LSB): %x\n", LSB_MAC_Address);

  // Start Benchmark
  bm_cli_log("Starting Benchmark...\n");
  bm_statemachine();
}