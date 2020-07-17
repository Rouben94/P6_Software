#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_CLI_H
#define BM_CLI_H

#include "bm_config.h"

#ifdef ZEPHYR
#include <zephyr.h>
// Global Printing Function for Printing Log Info
void bm_cli_log(const char *fmt, ...);
#elif defined NRF_SDK_Zigbee
#include "nrf_log.h"
#define bm_cli_log(...)                      NRF_LOG_INFO( __VA_ARGS__)
#endif





#ifdef __cplusplus
}
#endif
#endif