#include "bm_config.h"
#include "bm_cli.h"
#include "bm_statemachine.h"


#ifdef NRF_SDK_Zigbee
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#endif

/**@brief Function for application main entry.
 */
void main(void) {

 #ifdef NRF_SDK_Zigbee
  // Init the LOG Modul
  // To Enable Loging before the Mesh stack is run change in sdk_config -> NRF_LOG_DEFERRED = 0 (Performance is worse but logging is possibel)
  bm_cli_log_init(); /* Initialize the Zigbee LOG subsystem */
 #endif  

  // Start Benchmark
  bm_cli_log("Starting Benchmark...\n");
  bm_statemachine();

}