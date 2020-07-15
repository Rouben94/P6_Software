/*
 * Zigbee Benchmark Master
 * Acting as a Zigbee Coordinator
 */

#include "zb_error_handler.h"
#include "zboss_api.h"
#include "zigbee_helpers.h"

#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bm_config.h"
#include "bm_zigbee.h"

/**@brief Function for application main entry. */
int main(void) {
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;

  /* Initialize Zigbee stack. */
  bm_zigbee_init();

  /** Start Zigbee Stack. */
  bm_zigbee_enable();

  while (1) {
    zboss_main_loop_iteration();
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());

  }
}

/**
 * @}
 */