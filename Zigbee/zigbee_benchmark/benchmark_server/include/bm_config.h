#ifndef BM_CONFIG_H
#define BM_CONFIG_H

#include "stdbool.h"
#include "stdint.h"

// Change the following to switch the Protokoll Stack and SDK
#define NRF_SDK_Zigbee
//#define ZEPHYR_BLE_MESH

/* =============== Time Sync ===================== */
#define isTimeMaster 0           // Node is the Master (1) or Slave (0)
extern uint32_t LSB_MAC_Address; // LSB of Randomly Static Assigned MAC Address

/* =============== Benchmark Parameters ===================== */

// Change the following to switch the Protokoll Stack and SDK
#define BENCHMARK_SERVER /* Node is Benchmark Server */
//#define BENCHMARK_CLIENT                 /* Node is Benchmark Client */
//#define BENCHMARK_MASTER                 /* Node is Benchmark Master */
#define BENCHMARK_DEFAULT_TIME_S 10      // Default Benchmark Time (used when no Parameter available)
#define BENCHMARK_DEFAULT_PACKETS_CNT 10 // Default Benchmark Packet Count (used when no Parameter available)

typedef struct
{
  uint16_t benchmark_time_s;
  uint16_t benchmark_packet_cnt;
  uint8_t GroupAddress;
} bm_params_t;
extern bm_params_t bm_params, bm_params_buf; // The Buffer store changes while a benchmark is active.

/* =============== Defines for Reporting ===================== */
#define NUMBER_OF_BENCHMARK_REPORT_MESSAGES 3000 /* Size of the Benchmark Reporting Array message_info */

/* =============== BLE MESH Stuff ===================== */
#define BLE_MESH_TTL 7 // Maybee optimize

/* ================= Zigbee Stuff ====================== */
#define ZBOSS_MAIN_LOOP_ITERATION_TIME_MARGIN_MS 1000 // Time Margin needed because zboss can block timecheck. note this time will be added to the Stack Init Time

#define MAX_CHILDREN 10                          /**< The maximum amount of connected devices. Setting this value to 0 disables association to this device.  */
#define IEEE_CHANNEL_MASK (1l << ZIGBEE_CHANNEL) /**< Scan only one, predefined channel to find the coordinator. */
//#define IEEE_CHANNEL_MASK                 0x07fff800U
//#define HA_DIMMABLE_LIGHT_ENDPOINT 10    /**< Device endpoint, used to receive light controlling commands. */
#define ERASE_PERSISTENT_CONFIG ZB_FALSE /**< Do not erase NVRAM to save the network parameters after device reboot or power-off. */
#define BULB_PWM_NAME PWM1               /**< PWM instance used to drive dimmable light bulb. */
#define BULB_PWM_TIMER 2                 /**< Timer number used by PWM. */

/* Basic cluster attributes initial values. */
#define BULB_INIT_BASIC_APP_VERSION 01                                   /**< Version of the application software (1 byte). */
#define BULB_INIT_BASIC_STACK_VERSION 10                                 /**< Version of the implementation of the Zigbee stack (1 byte). */
#define BULB_INIT_BASIC_HW_VERSION 11                                    /**< Version of the hardware of the device (1 byte). */
#define BULB_INIT_BASIC_MANUF_NAME "Nordic"                              /**< Manufacturer name (32 bytes). */
#define BULB_INIT_BASIC_MODEL_ID "Dimable_Light_v0.1"                    /**< Model number assigned by manufacturer (32-bytes long string). */
#define BULB_INIT_BASIC_DATE_CODE "20180416"                             /**< First 8 bytes specify the date of manufacturer of the device in ISO 8601 format (YYYYMMDD). The rest (8 bytes) are manufacturer specific. */
#define BULB_INIT_BASIC_POWER_SOURCE ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE /**< Type of power sources available for the device. For possible values see section 3.2.2.2.8 of ZCL specification. */
#define BULB_INIT_BASIC_LOCATION_DESC "Office desk"                      /**< Describes the physical location of the device (16 bytes). May be modified during commisioning process. */
#define BULB_INIT_BASIC_PH_ENV ZB_ZCL_BASIC_ENV_UNSPECIFIED              /**< Describes the type of physical environment. For possible values see section 3.2.2.2.10 of ZCL specification. */

#define LIGHT_SWITCH_BUTTON_THRESHOLD ZB_TIME_ONE_SECOND                          /**< Number of beacon intervals the button should be pressed to dimm the light bulb. */
#define LIGHT_SWITCH_BUTTON_SHORT_POLL_TMO ZB_MILLISECONDS_TO_BEACON_INTERVAL(50) /**< Delay between button state checks used in order to detect button long press. */
#define LIGHT_SWITCH_BUTTON_LONG_POLL_TMO ZB_MILLISECONDS_TO_BEACON_INTERVAL(300) /**< Time after which the button state is checked again to detect button hold - the dimm command is sent again. */

#ifdef BOARD_PCA10059                            /**< If it is Dongle */
#define ZIGBEE_NETWORK_STATE_LED BSP_BOARD_LED_0 /**< LED indicating that light switch successfully joind Zigbee network. */
#define BULB_LED BSP_BOARD_LED_3                 /**< LED immitaing dimmable light bulb. */
#define DONGLE_BUTTON BSP_EVENT_KEY_0            /**< Button event used trigger button actions on the dongle hardware */
#define DONGLE_BUTTON_ON BSP_BOARD_BUTTON_0      /**< Button ID used to switch on the Dongle button */
#else
#define IDENTIFY_MODE_BSP_EVT BSP_EVENT_KEY_3    /**< Button event used to enter the Bulb into the Identify mode. */
#define ZIGBEE_NETWORK_STATE_LED BSP_BOARD_LED_2 /**< LED indicating that light switch successfully joind Zigbee network. */

#define ZIGBEE_NETWORK_STATE_LED BSP_BOARD_LED_2   /**< LED indicating that light switch successfully joind Zigbee network. */
#define LIGHT_SWITCH_BUTTON_ON BSP_BOARD_BUTTON_0  /**< Button ID used to switch on the light bulb. */
#define LIGHT_SWITCH_BUTTON_OFF BSP_BOARD_BUTTON_1 /**< Button ID used to switch off the light bulb. */
#define BENCHMARK_BUTTON BSP_BOARD_BUTTON_2        /**< Button ID used to switch off the light bulb. */
#define TEST_BUTTON BSP_BOARD_BUTTON_3             /**< Button ID used to switch off the light bulb. */

#endif

/* Benchmark specific Definitions*/
#define BENCHMARK_CLIENT_ENDPOINT 1  /* ZCL Endpoint of the Benchmark Client */
#define BENCHMARK_SERVER_ENDPOINT 10 /* ZCL Endpoint of the Benchmark Server */
#define GROUP_ID 0xB331              /* Group ID which will be used to address a specific group of Benchmark Servers */

#endif //BM_ZIGBEE_H