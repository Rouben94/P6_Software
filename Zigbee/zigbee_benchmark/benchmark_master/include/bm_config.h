#ifndef BM_CONFIG_H
#define BM_CONFIG_H

#include "stdbool.h"
#include "stdint.h"

// Change the following to switch the Protokoll Stack and SDK
#define NRF_SDK_Zigbee
//#define ZEPHYR_BLE_MESH

/* =============== Time Sync ===================== */
#define isTimeMaster 1           // Node is the Master (1) or Slave (0)
extern uint32_t LSB_MAC_Address; // LSB of Randomly Static Assigned MAC Address

/* =============== Benchmark Parameters ===================== */

// Change the following to switch the Protokoll Stack and SDK
//#define BENCHMARK_SERVER                 /* Node is Benchmark Server */
//#define BENCHMARK_CLIENT                 /* Node is Benchmark Client */
#define BENCHMARK_MASTER                 /* Node is Benchmark Master */
#define BENCHMARK_DEFAULT_TIME_S 10      // Default Benchmark Time (used when no Parameter available)
#define BENCHMARK_DEFAULT_PACKETS_CNT 10 // Default Benchmark Packet Count (used when no Parameter available)

/* =============== Defines for Reporting ===================== */
#define NUMBER_OF_BENCHMARK_REPORT_MESSAGES 3000 /* Size of the Benchmark Reporting Array message_info */

/* =============== BLE MESH Stuff ===================== */
#define BLE_MESH_TTL 7 // Maybee optimize

/* ================= Zigbee Stuff ====================== */
#define ZBOSS_MAIN_LOOP_ITERATION_TIME_MARGIN_MS 1000 // Time Margin needed because zboss can block timecheck. note this time will be added to the Stack Init Time
#define MAX_CHILDREN 10                               /**< The maximum amount of connected devices. Setting this value to 0 disables association to this device.  */

#define ERASE_PERSISTENT_CONFIG ZB_FALSE         /**< Do not erase NVRAM to save the network parameters after device reboot or power-off. NOTE: If this option is set to ZB_TRUE then do full device erase for all network devices before running other samples. */
#define MAX_CHILDREN 10                          /**< The maximum amount of connected devices. Setting this value to 0 disables association to this device.  */
#define IEEE_CHANNEL_MASK (1l << ZIGBEE_CHANNEL) /**< Scan only one, predefined channel to find the coordinator. */
//#define IEEE_CHANNEL_MASK                   0x07fff800U
#define ZB_COORDINATOR_ROLE

#define ZIGBEE_NETWORK_STATE_LED BSP_BOARD_LED_2        /**< LED indicating that network is opened for new nodes. */
#define ZIGBEE_NETWORK_REOPEN_BUTTON BSP_BOARD_BUTTON_0 /**< Button which reopens the Zigbee Network. */
#define ZIGBEE_MANUAL_STEERING ZB_FALSE                 /**< If set to 1 then device will not open the network after forming or reboot. */
#define ZIGBEE_PERMIT_LEGACY_DEVICES ZB_FALSE           /**< If set to 1 then legacy devices (pre-Zigbee 3.0) can join the network. */
#define BENCHMARK_BUTTON BSP_BOARD_BUTTON_2             /**< Button ID used to switch off the light bulb. */
#define TEST_BUTTON BSP_BOARD_BUTTON_3                  /**< Button ID used to switch off the light bulb. */

#ifndef ZB_COORDINATOR_ROLE
#error Define ZB_COORDINATOR_ROLE to compile coordinator source code.
#endif

/* Benchmark specific Definitions*/
#define BENCHMARK_CLIENT_ENDPOINT 1     /* ZCL Endpoint of the Benchmark Client */
#define BENCHMARK_SERVER_ENDPOINT 10    /* ZCL Endpoint of the Benchmark Server */
#define BENCHMARK_CONTROL_ENDPOINT 11   /* ZCL Endpoint for Benchmark Control */
#define BENCHMARK_REPORTING_ENDPOINT 12 /* ZCL Endpoint for Benchmark Reporting Message */

#define BENCHMARK_CUSTOM_CMD_ID 0x00 /* Custom Benchmark Command ID */

/* =============== Benchmark Parameters ===================== */
#define BENCHMARK_DEFAULT_TIME_S 10      // Default Benchmark Time (used when no Parameter available)
#define BENCHMARK_DEFAULT_PACKETS_CNT 10 // Default Benchmark Packet Count (used when no Parameter available)

typedef struct
{
  uint16_t benchmark_time_s;
  uint16_t benchmark_packet_cnt;
} bm_params_t;
extern bm_params_t bm_params, bm_params_buf; // The Buffer store changes while a benchmark is active.

#endif //BM_ZIGBEE_H