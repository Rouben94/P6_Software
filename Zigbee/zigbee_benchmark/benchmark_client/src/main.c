/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 * @brief Dimmer switch for HA profile implementation.
 */

#include <zephyr.h>
#include <device.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>

#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zigbee_helpers.h>
#include <zb_error_handler.h>
#include <zb_nrf_platform.h>
#include "zb_mem_config_custom.h"

#define RUN_STATUS_LED DK_LED1				 /* Node Status LED */
#define SEND_LED DK_LED2					 /* Send LED */
#define RUN_LED_BLINK_INTERVAL K_MSEC(1000)	 /* Blink Interval for the Node Status LED */
#define BM_STACK_SIZE 500					 /* Benchmark Stack Size */
#define BM_THREAD_PRIO 5					 /* Benchmark Thread Priority */
#define GROUP_ID 0xB331						 /* Group ID to send Benchmark message to.*/
#define BUTTON_BENCHMARK DK_BTN4_MSK		 /* Button ID used to start Benchmark Message. */
#define LIGHT_SWITCH_ENDPOINT 1				 /* Source endpoint used to control light bulb. */
#define ERASE_PERSISTENT_CONFIG ZB_FALSE	 /* Do not erase NVRAM to save the network parameters after device reboot or    \
											  * power-off. NOTE: If this option is set to ZB_TRUE then do full device erase \
											  * for all network devices before running other samples.*/
#define ZIGBEE_NETWORK_STATE_LED DK_LED3	 /* LED indicating that light switch successfully joind Zigbee network. */
#define BUTTON_ON DK_BTN1_MSK				 /* Button ID used to switch on the light bulb. */
#define BUTTON_OFF DK_BTN2_MSK				 /* Button ID used to switch off the light bulb. */
#define NUMBER_OF_NETWORK_TIME_ELEMENTS 1000 /* Size of the Benchmark Reporting Array message_info */

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE to compile light switch (End Device) source code.
#endif

LOG_MODULE_REGISTER(app);

struct light_switch_bulb_params
{
	zb_uint8_t endpoint;
	zb_uint16_t short_addr;
};

struct light_switch_button
{
	atomic_t in_progress;
	atomic_t long_poll;
};

struct light_switch_ctx
{
	struct light_switch_bulb_params bulb_params;
	struct light_switch_button button;
};

static struct light_switch_ctx device_ctx;
static zb_uint8_t attr_zcl_version = ZB_ZCL_VERSION;
static zb_uint8_t attr_power_source = ZB_ZCL_BASIC_POWER_SOURCE_UNKNOWN;
static zb_uint16_t attr_identify_time;

/* Declare attribute list for Basic cluster. */
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST(basic_attr_list, &attr_zcl_version,
								 &attr_power_source);

/* Declare attribute list for Identify cluster. */
ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(identify_attr_list, &attr_identify_time);

/* Declare cluster list for Dimmer Switch device (Identify, Basic, Scenes,
 * Groups, On Off, Level Control).
 * Only clusters Identify and Basic have attributes.
 */
ZB_HA_DECLARE_DIMMER_SWITCH_CLUSTER_LIST(dimmer_switch_clusters,
										 basic_attr_list,
										 identify_attr_list);

/* Declare endpoint for Dimmer Switch device. */
ZB_HA_DECLARE_DIMMER_SWITCH_EP(dimmer_switch_ep,
							   LIGHT_SWITCH_ENDPOINT,
							   dimmer_switch_clusters);

/* Declare application's device context (list of registered endpoints)
 * for Dimmer Switch device.
 */
ZB_HA_DECLARE_DIMMER_SWITCH_CTX(dimmer_switch_ctx, dimmer_switch_ep);

/* Struct for benchmark message information */
typedef struct
{
	zb_uint16_t dst_addr;
	zb_ieee_addr_t src_addr;
	zb_uint64_t net_time;
	zb_uint16_t number_of_hops;
	zb_uint16_t message_id;
	zb_uint8_t RSSI;
	bool data_size;
} bm_message_info;

/* Forward declarations */
static void light_switch_send_on_off(zb_bufid_t bufid, zb_uint16_t on_off);
static void bm_send_message_cb(zb_bufid_t bufid, zb_uint16_t on_off);
void bm_save_message_info(bm_message_info * message);

/* Array of structs to save benchmark message info to */
bm_message_info message_info[NUMBER_OF_NETWORK_TIME_ELEMENTS] = {0};

K_THREAD_STACK_DEFINE(bm_stack_area, BM_STACK_SIZE);
struct k_thread bm_thread_data;
//static void bm_send_message(void *arg1, void *arg2, void *arg3);
static void bm_send_message(zb_uint8_t message_id);
static void bm_zigbee(void *arg1, void *arg2, void *arg3);

uint16_t bm_msg_cnt;
uint16_t bm_time_interval_msec;
uint16_t bm_message_info_nr = 0;

/**@brief Callback for button events.
 *
 * @param[in]   button_state  Bitmask containing buttons state.
 * @param[in]   has_changed   Bitmask containing buttons that has
 *                            changed their state.
 */
static void button_handler(u32_t button_state, u32_t has_changed)
{
	zb_bool_t on_off = ZB_FALSE;
	k_tid_t bm_thread;
	zb_uint32_t buttons = button_state & has_changed;

	/* Inform default signal handler about user input at the device. */
	user_input_indicate();

	switch (buttons)
	{
	case BUTTON_ON:
		LOG_INF("ON - Button pressed");
		on_off = ZB_TRUE;
		atomic_set(&device_ctx.button.in_progress, ZB_TRUE);
		dk_set_led(SEND_LED, on_off);
		light_switch_send_on_off(zb_buf_get_out(), on_off);
		atomic_set(&device_ctx.button.in_progress, ZB_FALSE);
		break;

	case BUTTON_OFF:
		LOG_INF("OFF - Button pressed");
		on_off = ZB_FALSE;
		atomic_set(&device_ctx.button.in_progress, ZB_TRUE);
		dk_set_led(SEND_LED, on_off);
		light_switch_send_on_off(zb_buf_get_out(), on_off);
		atomic_set(&device_ctx.button.in_progress, ZB_FALSE);
		break;

	case BUTTON_BENCHMARK:
		LOG_INF("BENCHMARK - Button pressed");
		atomic_set(&device_ctx.button.in_progress, ZB_TRUE);
		bm_msg_cnt = 5;
		bm_time_interval_msec = 20000;
		bm_thread = k_thread_create(&bm_thread_data, bm_stack_area,
									K_THREAD_STACK_SIZEOF(bm_stack_area),
									bm_zigbee,
									NULL, NULL, NULL,
									BM_THREAD_PRIO, 0, K_NO_WAIT);
		LOG_INF("BENCHMARK - Button released");
		atomic_set(&device_ctx.button.in_progress, ZB_FALSE);
		break;

	default:
		LOG_INF("Unhandled button");
		return;
	}
}

/**@brief Function for initializing LEDs and Buttons. */
static void configure_gpio(void)
{
	int err;
	err = dk_buttons_init(button_handler);
	if (err)
	{
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}
	err = dk_leds_init();
	if (err)
	{
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}
}

/**@brief Function for sending ON/OFF requests to the light bulb.
 *
 * @param[in]   bufid    Non-zero reference to Zigbee stack buffer that will be
 *                       used to construct on/off request.
 * @param[in]   on_off   Requested state of the light bulb.
 */
static void light_switch_send_on_off(zb_bufid_t bufid, zb_uint16_t on_off)
{
	u8_t cmd_id = on_off ? ZB_ZCL_CMD_ON_OFF_ON_ID
						 : ZB_ZCL_CMD_ON_OFF_OFF_ID;
	zb_uint16_t group_id = GROUP_ID;

	LOG_INF("Send ON/OFF command: %d to group id: %d", on_off, group_id);

	ZB_ZCL_ON_OFF_SEND_REQ(bufid,
						   group_id,
						   ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT,
						   0,
						   LIGHT_SWITCH_ENDPOINT,
						   ZB_AF_HA_PROFILE_ID,
						   ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
						   cmd_id,
						   NULL);
}

/* Function to send Benchmark Message */
static void bm_send_message_cb(zb_bufid_t bufid, zb_uint16_t level)
{
	zb_uint16_t group_id = GROUP_ID;
	LOG_INF("Benchmark Message Callback send.");

	/* Send Move to level request. Level value is uint8. */
	ZB_ZCL_LEVEL_CONTROL_SEND_MOVE_TO_LEVEL_REQ(bufid,
												group_id,
												ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT,
												0,
												LIGHT_SWITCH_ENDPOINT,
												ZB_AF_HA_PROFILE_ID,
												ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
												NULL,
												level,
												0);
}

static void bm_send_message(zb_uint8_t message_id)
{
	zb_ret_t zb_err_code;
	zb_err_code = zb_buf_get_out_delayed_ext(bm_send_message_cb, message_id, 0);
	ZB_ERROR_CHECK(zb_err_code);

	//bm_send_message_cb(zb_buf_get_out(), message_id);
}

static void bm_zigbee(void *arg1, void *arg2, void *arg3)
{
	bm_message_info *message;
	message->message_id = 0;
	message->RSSI = 0;
	message->number_of_hops = 0;
	message->data_size = 0;
	zb_get_long_address(message->src_addr);
	message->dst_addr = GROUP_ID;
	message->net_time = 0;

	for (u8_t i = 0; i < bm_msg_cnt; i++)
	{
		zb_uint16_t timeout = ZB_RANDOM_VALUE(bm_time_interval_msec / bm_msg_cnt);
		k_sleep(K_MSEC(timeout));

		ZB_SCHEDULE_APP_ALARM_CANCEL(bm_send_message, ZB_ALARM_ANY_PARAM);

		message->net_time = ZB_TIME_BEACON_INTERVAL_TO_MSEC(ZB_TIMER_GET());
		message->message_id += 15;
		LOG_INF("Benchmark Message send, TimeStamp: %llu, MessageID: %d, TimeOut: %u", message->net_time, message->message_id, timeout);

		ZB_SCHEDULE_APP_ALARM(bm_send_message, message->message_id, 0);

		bm_save_message_info(message);
	}

	LOG_INF("Benchmark finished");
}

void bm_save_message_info(bm_message_info * message)
{
	bm_message_info new_message = *message;
	message_info[bm_message_info_nr] = new_message;
	bm_message_info_nr++;
}

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer
 *                      used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid)
{

	/* Update network status LED */
	zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

	ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

	if (bufid)
	{
		zb_buf_free(bufid);
	}
}

void main(void)
{
	int blink_status = 0;

	LOG_INF("Starting ZBOSS Light Switch example");

	/* Initialize. */
	configure_gpio();

	zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);

	zb_set_ed_timeout(ED_AGING_TIMEOUT_64MIN);
	zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(3000));

	/* Initialize application context structure. */
	memset(&device_ctx, 0, sizeof(struct light_switch_ctx));

	/* Set default bulb short_addr. */
	device_ctx.bulb_params.short_addr = 0xFFFF;

	/* Register dimmer switch device context (endpoints). */
	ZB_AF_REGISTER_DEVICE_CTX(&dimmer_switch_ctx);

	/* Start Zigbee default thread */
	zigbee_enable();

	LOG_INF("ZBOSS Light Switch example started");

	while (1)
	{
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(RUN_LED_BLINK_INTERVAL);
	}
}
