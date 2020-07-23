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
#define bm_cli_log(...) NRF_LOG_RAW_INFO(__VA_ARGS__)
#endif

// Init th CLI Commands for the Master
#ifdef BENCHMARK_MASTER

typedef struct {
  bool req; // Is true if a cli command was received
  uint32_t MAC;
} bm_cli_cmd_getNodeReport_t;

extern bm_cli_cmd_getNodeReport_t bm_cli_cmd_getNodeReport;

typedef struct {
  bool req; // Is true if a cli command was received
  uint32_t MAC;
  uint8_t GroupAddress;
} bm_cli_cmd_setNodeSettings_t;

extern bm_cli_cmd_setNodeSettings_t bm_cli_cmd_setNodeSettings;

typedef struct {
  bool req; // Is true if a cli command was received
  uint16_t benchmark_time_s;
  uint16_t benchmark_packet_cnt;
} bm_cli_cmd_startBM_t;

extern bm_cli_cmd_startBM_t bm_cli_cmd_startBM;

#ifdef ZEPHYR_BLE_MESH

#elif defined NRF_SDK_Zigbee

void bm_cli_start(void);

void bm_cli_init(void); /* Initialize the Zigbee CLI subsystem */

void bm_cli_process(void);

#endif
#endif
#ifdef NRF_SDK_Zigbee

void bm_cli_init(void);

void bm_cli_log_init(void); /* Initialize the Zigbee LOG subsystem */

#endif

#ifdef __cplusplus
}
#endif
#endif