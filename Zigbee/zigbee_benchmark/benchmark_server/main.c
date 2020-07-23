
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

uint32_t MSB_MAC_Address, LSB_MAC_Address;

/**@brief Function for application main entry.
 */
int main(void) {

#ifdef NRF_SDK_Zigbee
  // Init the LOG Modul
  // To Enable Loging before the Mesh stack is run change in sdk_config -> NRF_LOG_DEFERRED = 0 (Performance is worse but logging is possibel)

  bm_cli_log_init(); /* Initialize the Zigbee LOG subsystem */

#endif

  // Init MAC Address
  MSB_MAC_Address = NRF_FICR->DEVICEADDR[1];
  LSB_MAC_Address = NRF_FICR->DEVICEADDR[0];
  bm_cli_log("Preprogrammed randomly static device address (MSB): %x, (LSB) %x", MSB_MAC_Address, LSB_MAC_Address);
  // Start Benchmark
  bm_cli_log("Starting Benchmark SERVER");
  bm_statemachine();
}