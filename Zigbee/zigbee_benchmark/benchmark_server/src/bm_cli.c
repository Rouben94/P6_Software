#include "bm_cli.h"
#include "bm_config.h"

#ifdef ZEPHYR
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "sdk_config.h"

#endif


//void bm_cli_log(const char *fmt, ...) {
void bm_cli_log(char const * const p_str,...) {
  // Zephyr way to Log info

#ifdef ZEPHYR
  printk(fmt);
#elif defined NRF_SDK_Zigbee

  //NRF_LOG_INFO("HALLO");
  NRF_LOG_INFO(p_str);
  NRF_LOG_PROCESS();
  NRF_LOG_PROCESS();

#endif

  return;
}

