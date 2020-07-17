#ifndef BM_CONFIG_H
#define BM_CONFIG_H

#define ERASE_PERSISTENT_CONFIG ZB_FALSE         /**< Do not erase NVRAM to save the network parameters after device reboot or power-off. NOTE: If this option is set to ZB_TRUE then do full device erase for all network devices before running other samples. */
#define MAX_CHILDREN 10                          /**< The maximum amount of connected devices. Setting this value to 0 disables association to this device.  */
#define IEEE_CHANNEL_MASK (1l << ZIGBEE_CHANNEL) /**< Scan only one, predefined channel to find the coordinator. */
#ifdef BOARD_PCA10059                            /**< If it is Dongle */
#define ZIGBEE_NETWORK_STATE_LED BSP_BOARD_LED_0 /**< LED indicating that network is opened for new nodes. */
#else
#define ZIGBEE_NETWORK_STATE_LED BSP_BOARD_LED_2 /**< LED indicating that network is opened for new nodes. */
#endif
#define ZIGBEE_NETWORK_REOPEN_BUTTON BSP_BOARD_BUTTON_0 /**< Button which reopens the Zigbee Network. */
#define ZIGBEE_MANUAL_STEERING ZB_FALSE                 /**< If set to 1 then device will not open the network after forming or reboot. */
#define ZIGBEE_PERMIT_LEGACY_DEVICES ZB_FALSE           /**< If set to 1 then legacy devices (pre-Zigbee 3.0) can join the network. */

#ifndef ZB_COORDINATOR_ROLE
#error Define ZB_COORDINATOR_ROLE to compile coordinator source code.
#endif

/* Benchmark specific Definitions*/
#define BENCHMARK_CLIENT_ENDPOINT 1   /* ZCL Endpoint of the Benchmark Client */
#define BENCHMARK_SERVER_ENDPOINT 10  /* ZCL Endpoint of the Benchmark Server */
#define BENCHMARK_CONTROL_ENDPOINT 11 /* ZCL Endpoint for Benchmark Control */
#define BENCHMARK_REPORTING_ENDPOINT 12 /* ZCL Endpoint for Benchmark Reporting Message */

#define BENCHMARK_BUTTON BSP_BOARD_BUTTON_3

#endif //BM_ZIGBEE_H