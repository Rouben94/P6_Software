#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_CONFIG_H
#define BM_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// Change the following to switch the Protokoll Stack and SDK
//#define NRF_SDK_Zigbee
#define ZEPHYR_BLE_MESH

/* =============== Time Sync ===================== */
#define isTimeMaster 1           // Node is the Master (1) or Slave (0)
extern uint32_t LSB_MAC_Address; // LSB of Randomly Static Assigned MAC Address

/* =============== Defines for Reporting ===================== */
#define NUMBER_OF_BENCHMARK_REPORT_MESSAGES 1000 /* Size of the Benchmark Reporting Array message_info */

/* =============== BLE MESH Stuff ===================== */
#define BLE_MESH_TTL 7 // Maybee optimize

/* ================= Zigbee Stuff ====================== */
#define MAX_CHILDREN 10                          /**< The maximum amount of connected devices. Setting this value to 0 disables association to this device.  */
#define IEEE_CHANNEL_MASK (1l << ZIGBEE_CHANNEL) /**< Scan only one, predefined channel to find the coordinator. */
//#define IEEE_CHANNEL_MASK                 0x07fff800U
#define HA_DIMMABLE_LIGHT_ENDPOINT 10    /**< Device endpoint, used to receive light controlling commands. */
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

#ifdef BOARD_PCA10059                            /**< If it is Dongle */
#define IDENTIFY_MODE_BSP_EVT BSP_EVENT_KEY_0    /**< Button event used to enter the Bulb into the Identify mode. */
#define ZIGBEE_NETWORK_STATE_LED BSP_BOARD_LED_0 /**< LED indicating that light switch successfully joind Zigbee network. */
#else
#define IDENTIFY_MODE_BSP_EVT BSP_EVENT_KEY_3    /**< Button event used to enter the Bulb into the Identify mode. */
#define ZIGBEE_NETWORK_STATE_LED BSP_BOARD_LED_2 /**< LED indicating that light switch successfully joind Zigbee network. */
#endif
#define BULB_LED BSP_BOARD_LED_3 /**< LED immitaing dimmable light bulb. */

/* Benchmark specific Definitions*/
#define GROUP_ID 0xB331                      /* Group ID which will be used to address a specific group of Benchmark Servers */
#define BENCHMARK_CLIENT_ENDPOINT 1          /* ZCL Endpoint of the Benchmark Client */
#define BENCHMARK_SERVER_ENDPOINT 10         /* ZCL Endpoint of the Benchmark Server */
#define BENCHMARK_CONTROL_ENDPOINT 11        /* ZCL Endpoint for Benchmark Control */
#define BENCHMARK_REPORTING_ENDPOINT 12      /* ZCL Endpoint for Benchmark Reporting Message */
#define NUMBER_OF_NETWORK_TIME_ELEMENTS 1000 /* Size of the Benchmark Reporting Array message_info */
#define DONGLE_BUTTON_BSP_EVT BSP_EVENT_KEY_0

#define BENCHMARK_CUSTOM_CMD_ID 0x00 /* Custom Benchmark Command ID */

#ifdef __cplusplus
}
#endif
#endif //BM_ZIGBEE_H