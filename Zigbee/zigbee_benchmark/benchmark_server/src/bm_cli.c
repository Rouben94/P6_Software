#include "bm_cli.h"
#include "bm_config.h"

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
void bm_cli_log(const char *fmt, ...) {
  // Zephyr way to Log info
  printk(fmt);
}
#elif defined NRF_SDK_Zigbee
// function is Define in header file
#endif