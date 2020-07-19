#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_CLI_H
#define BM_CLI_H

#include "bm_config.h"

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
// Global Printing Function for Printing Log Info
void bm_cli_log(const char *fmt, ...);
#elif defined NRF_SDK_Zigbee
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#define bm_cli_log(...) NRF_LOG_INFO(__VA_ARGS__)

static void bm_usbd_init(void);

static void bm_usbd_enable(void);

void bm_cli_start(void);

void bm_cli_init(void);

void bm_cli_process(void);

#endif

#ifdef __cplusplus
}
#endif
#endif