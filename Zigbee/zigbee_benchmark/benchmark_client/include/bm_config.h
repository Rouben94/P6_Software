#ifndef BM_CONFIG_H
#define BM_CONFIG_H

#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"

#define IEEE_CHANNEL_MASK (1l << ZIGBEE_CHANNEL) /**< Scan only one, predefined channel to find the coordinator. */
//#define IEEE_CHANNEL_MASK                   0x07fff800U
#define LIGHT_SWITCH_ENDPOINT 1                              /**< Source endpoint used to control light bulb. */
#define LIGHT_BULB_ENDPOINT 10                               /**< Destination endpoint used to control light bulb. */
#define MATCH_DESC_REQ_START_DELAY (2 * ZB_TIME_ONE_SECOND)  /**< Delay between the light switch startup and light bulb finding procedure. */
#define MATCH_DESC_REQ_TIMEOUT (5 * ZB_TIME_ONE_SECOND)      /**< Timeout for finding procedure. */
#define MATCH_DESC_REQ_ROLE ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE /**< Find only non-sleepy device. */
#define ERASE_PERSISTENT_CONFIG ZB_FALSE                     /**< Do not erase NVRAM to save the network parameters after device reboot or power-off. NOTE: If this option is set to ZB_TRUE then do full device erase for all network devices before running other samples. */
#define ZIGBEE_NETWORK_STATE_LED BSP_BOARD_LED_2             /**< LED indicating that light switch successfully joind Zigbee network. */
#define BULB_FOUND_LED BSP_BOARD_LED_3                       /**< LED indicating that light witch found a light bulb to control. */
#define LIGHT_SWITCH_BUTTON_ON BSP_BOARD_BUTTON_0            /**< Button ID used to switch on the light bulb. */
#define LIGHT_SWITCH_BUTTON_OFF BSP_BOARD_BUTTON_1           /**< Button ID used to switch off the light bulb. */
#define BENCHMARK_BUTTON BSP_BOARD_BUTTON_2                  /**< Button ID used to switch off the light bulb. */
#define TEST_BUTTON BSP_BOARD_BUTTON_3                       /**< Button ID used to switch off the light bulb. */
#define SLEEPY_ON_BUTTON BSP_BOARD_BUTTON_2                  /**< Button ID used to determine if we need the sleepy device behaviour (pressed means yes). */

#define BENCHMARK_INIT_BASIC_APP_VERSION 01                                   /* Version of the application software (1 byte). */
#define BENCHMARK_INIT_BASIC_STACK_VERSION 10                                 /* Version of the implementation of the Zigbee stack (1 byte). */
#define BENCHMARK_INIT_BASIC_HW_VERSION 11                                    /* Version of the hardware of the device (1 byte). */
#define BENCHMARK_INIT_BASIC_MANUF_NAME "Nordic"                              /* Manufacturer name (32 bytes). */
#define BENCHMARK_INIT_BASIC_MODEL_ID "Dimable_Light_v0.1"                    /* Model number assigned by manufacturer (32-bytes long string). */
#define BENCHMARK_INIT_BASIC_DATE_CODE "20200329"                             /* First 8 bytes specify the date of manufacturer of the device in ISO 8601 format (YYYYMMDD). The rest (8 bytes) are manufacturer specific.*/
#define BENCHMARK_INIT_BASIC_POWER_SOURCE ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE /* Type of power sources available for the device. For possible values see section 3.2.2.2.8 of ZCL specification.*/
#define BENCHMARK_INIT_BASIC_LOCATION_DESC "Office desk"                      /* Describes the physical location of the device (16 bytes). May be modified during commisioning process.*/
#define BENCHMARK_INIT_BASIC_PH_ENV ZB_ZCL_BASIC_ENV_UNSPECIFIED

#define LIGHT_SWITCH_DIMM_STEP 15            /**< Dim step size - increases/decreses current level (range 0x000 - 0xfe). */
#define LIGHT_SWITCH_DIMM_TRANSACTION_TIME 2 /**< Transition time for a single step operation in 0.1 sec units. 0xFFFF - immediate change. */

#define LIGHT_SWITCH_BUTTON_THRESHOLD ZB_TIME_ONE_SECOND                          /**< Number of beacon intervals the button should be pressed to dimm the light bulb. */
#define LIGHT_SWITCH_BUTTON_SHORT_POLL_TMO ZB_MILLISECONDS_TO_BEACON_INTERVAL(50) /**< Delay between button state checks used in order to detect button long press. */
#define LIGHT_SWITCH_BUTTON_LONG_POLL_TMO ZB_MILLISECONDS_TO_BEACON_INTERVAL(300) /**< Time after which the button state is checked again to detect button hold - the dimm command is sent again. */

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE to compile light switch (End Device) source code.
#endif

/* Benchmark specific Definitions*/
#define BENCHMARK_CLIENT_ENDPOINT 1     /* ZCL Endpoint of the Benchmark Client */
#define BENCHMARK_SERVER_ENDPOINT 10    /* ZCL Endpoint of the Benchmark Server */
#define BENCHMARK_CONTROL_ENDPOINT 11   /* ZCL Endpoint for Benchmark Control */
#define BENCHMARK_REPORTING_ENDPOINT 12 /* ZCL Endpoint for Benchmark Reporting Message */
#define GROUP_ID 0xB331                 /* Group ID to send Benchmark message to.*/

#define BENCHMARK_CUSTOM_CMD_ID 0x55 /* Custom Benchmark Command ID */

#define NUMBER_OF_BENCHMARK_REPORT_MESSAGES 3000 /* Size of the Benchmark Reporting Array message_info */

#endif //BM_ZIGBEE_H