#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_CLI_H
#define BM_CLI_H

#ifdef ZEPHYR
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "sdk_config.h"

#endif

// Global Printing Function for Printing Log Info
void bm_cli_log(char const * const p_str,...);

//#define bm_cli_log(...) NRF_LOG_INFO(__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif