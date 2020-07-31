
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

/**@brief Function for application main entry.
 */
int main(void) {
#if defined (NRF_SDK_ZIGBEE) || defined (NRF_SDK_Thread)

  // Init the LOG Modul
  // To Enable Loging before the Mesh stack is run change in sdk_config -> NRF_LOG_DEFERRED = 0 (Performance is worse but logging is possibel)
  bm_cli_log_init(); /* Initialize the Zigbee LOG subsystem */
#endif
  bm_statemachine();
}