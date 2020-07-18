/*
 * Zigbee Benchmark Client
 * Acting as a Dimmer Light Switch
 */

#include "sdk_config.h"
#include "zb_error_handler.h"
#include "zboss_api.h"
#include "zboss_api_addons.h"
#include "zigbee_helpers.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bm_config.h"
#include "bm_zigbee.h"

uint32_t LSB_MAC_Address;

/**@brief Function for application main entry.
 */
int main(void) {

  // Init MAC Address
  LSB_MAC_Address = NRF_FICR->DEVICEADDR[1];
  bm_cli_log("Preprogrammed Randomly Static MAC-Address (LSB): %x\n", LSB_MAC_Address);

  // Start Benchmark
  bm_cli_log("Starting Benchmark...\n");
  bm_statemachine();

  //  /* Initialize Zigbee stack. */
  //  bm_zigbee_init();
  //
  //  /** Start Zigbee Stack. */
  //  bm_zigbee_enable();

  //  while (1) {
  //    zboss_main_loop_iteration();
  //    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
  //  }
  //}

  /**
 * @}
 */