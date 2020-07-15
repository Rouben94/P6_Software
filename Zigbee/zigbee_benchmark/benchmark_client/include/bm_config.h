#ifndef BM_CONFIG_H
#define BM_CONFIG_H

#define IEEE_CHANNEL_MASK                   (1l << ZIGBEE_CHANNEL)              /**< Scan only one, predefined channel to find the coordinator. */
//#define IEEE_CHANNEL_MASK                   0x07fff800U
#define LIGHT_SWITCH_ENDPOINT               1                                   /**< Source endpoint used to control light bulb. */
#define MATCH_DESC_REQ_START_DELAY          (2 * ZB_TIME_ONE_SECOND)            /**< Delay between the light switch startup and light bulb finding procedure. */
#define MATCH_DESC_REQ_TIMEOUT              (5 * ZB_TIME_ONE_SECOND)            /**< Timeout for finding procedure. */
#define MATCH_DESC_REQ_ROLE                 ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE    /**< Find only non-sleepy device. */
#define ERASE_PERSISTENT_CONFIG             ZB_FALSE                            /**< Do not erase NVRAM to save the network parameters after device reboot or power-off. NOTE: If this option is set to ZB_TRUE then do full device erase for all network devices before running other samples. */
#define ZIGBEE_NETWORK_STATE_LED            BSP_BOARD_LED_2                     /**< LED indicating that light switch successfully joind Zigbee network. */
#define BULB_FOUND_LED                      BSP_BOARD_LED_3                     /**< LED indicating that light witch found a light bulb to control. */
#define LIGHT_SWITCH_BUTTON_ON              BSP_BOARD_BUTTON_0                  /**< Button ID used to switch on the light bulb. */
#define LIGHT_SWITCH_BUTTON_OFF             BSP_BOARD_BUTTON_1                  /**< Button ID used to switch off the light bulb. */
#define SLEEPY_ON_BUTTON                    BSP_BOARD_BUTTON_2                  /**< Button ID used to determine if we need the sleepy device behaviour (pressed means yes). */

#define LIGHT_SWITCH_DIMM_STEP              15                                  /**< Dim step size - increases/decreses current level (range 0x000 - 0xfe). */
#define LIGHT_SWITCH_DIMM_TRANSACTION_TIME  2                                   /**< Transition time for a single step operation in 0.1 sec units. 0xFFFF - immediate change. */

#define LIGHT_SWITCH_BUTTON_THRESHOLD       ZB_TIME_ONE_SECOND                      /**< Number of beacon intervals the button should be pressed to dimm the light bulb. */
#define LIGHT_SWITCH_BUTTON_SHORT_POLL_TMO  ZB_MILLISECONDS_TO_BEACON_INTERVAL(50)  /**< Delay between button state checks used in order to detect button long press. */
#define LIGHT_SWITCH_BUTTON_LONG_POLL_TMO   ZB_MILLISECONDS_TO_BEACON_INTERVAL(300) /**< Time after which the button state is checked again to detect button hold - the dimm command is sent again. */

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE to compile light switch (End Device) source code.
#endif

/* Benchmark specific Definitions*/
#define BENCHMARK_CLIENT_ENDPOINT 1	  /* ZCL Endpoint of the Benchmark Client */
#define BENCHMARK_SERVER_ENDPOINT 10  /* ZCL Endpoint of the Benchmark Server */
#define BENCHMARK_CONTROL_ENDPOINT 11 /* ZCL Endpoint for Benchmark Control */
#define NUMBER_OF_NETWORK_TIME_ELEMENTS 1000 /* Size of the Benchmark Reporting Array message_info */


#endif //BM_ZIGBEE_H