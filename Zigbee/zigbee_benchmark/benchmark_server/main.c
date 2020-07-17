/*
 * Zigbee Benchmark Server
 * Acting as a Light Bulb
 */

#include "sdk_config.h"
#include "zb_error_handler.h"
#include "zb_nrf52_internal.h"
#include "zboss_api.h"
#include "zboss_api_addons.h"
#include "zigbee_helpers.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"
#include "bm_zigbee.h"

uint32_t LSB_MAC_Address;

/**@brief Function for application main entry.
 */
int main(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();

  // Init MAC Address
  LSB_MAC_Address = NRF_FICR->DEVICEADDR[1];
  NRF_LOG_INFO("Preprogrammed Randomly Static MAC-Address (LSB): %x", LSB_MAC_Address);
  bm_cli_log("Preprogrammed Randomly Static MAC-Address (LSB): %x", LSB_MAC_Address);
  NRF_LOG_PROCESS();

  // Start Benchmark
  NRF_LOG_INFO("Starting Benchmark...\n");
  NRF_LOG_PROCESS();
  bm_statemachine();

  //  /* Initialize Zigbee stack. */
  // bm_zigbee_init();
  //
  //  /** Start Zigbee Stack. */
  //bm_zigbee_enable();

  //  while (1) {
  //    zboss_main_loop_iteration();
  //    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
  //  }
}