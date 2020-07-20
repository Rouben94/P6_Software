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
#define bm_cli_log(...)                      NRF_LOG_INFO( __VA_ARGS__)
#endif

// Init th CLI Commands for the Master
#ifdef BENCHMARK_MASTER
#ifdef ZEPHYR_BLE_MESH
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
#elif defined NRF_SDK_Zigbee
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#define bm_cli_log(...)                      NRF_LOG_INFO( __VA_ARGS__)
#endif
#endif






#ifdef __cplusplus
}
#endif
#endif