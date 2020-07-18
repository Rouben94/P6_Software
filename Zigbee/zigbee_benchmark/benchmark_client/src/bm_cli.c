#include "bm_cli.h"
#include "bm_config.h"

/* Init the Parameters */
bm_params_t bm_params = {BENCHMARK_DEFAULT_TIME_S,BENCHMARK_DEFAULT_PACKETS_CNT};
bm_params_t bm_params_buf = {BENCHMARK_DEFAULT_TIME_S,BENCHMARK_DEFAULT_PACKETS_CNT};

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
void bm_cli_log(const char *fmt, ...) {
  // Zephyr way to Log info
  printk(fmt);
}
#elif defined NRF_SDK_Zigbee
// function is Define in header file
#endif