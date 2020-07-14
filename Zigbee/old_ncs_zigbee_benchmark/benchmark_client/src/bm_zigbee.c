/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 * @brief Dimmer switch for HA profile implementation.
 */

#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <soc.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>
#include <drivers/gpio.h>

#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zigbee_helpers.h>
#include <zb_error_handler.h>
#include <zb_nrf_platform.h>
#include <zb_mem_config_med.h>

#include "bm_zigbee.h"

#define BM_STACK_SIZE 500			  /* Benchmark Stack Size */
#define BM_THREAD_PRIO 5			  /* Benchmark Thread Priority */
#define GROUP_ID 0xB331				  /* Group ID to send Benchmark message to.*/
#define BENCHMARK_CLIENT_ENDPOINT 1	  /* ZCL Endpoint of the Benchmark Client */
#define BENCHMARK_SERVER_ENDPOINT 10  /* ZCL Endpoint of the Benchmark Server */
#define BENCHMARK_CONTROL_ENDPOINT 11 /* ZCL Endpoint for Benchmark Control */
#define BUTTON_BENCHMARK DK_BTN4_MSK  /* Button ID used to start Benchmark Message. */
#define BUTTON_TEST DK_BTN3_MSK		  /* Button ID used to start Test an application. */

#define LIGHT_SWITCH_ENDPOINT 1				 /* Source endpoint used to control light bulb. */
#define ERASE_PERSISTENT_CONFIG ZB_FALSE	 /* Do not erase NVRAM to save the network parameters after device reboot or    \
											  * power-off. NOTE: If this option is set to ZB_TRUE then do full device erase \
											  * for all network devices before running other samples.*/
#define ZIGBEE_NETWORK_STATE_LED DK_LED3	 /* LED indicating that light switch successfully joind Zigbee network. */
#define BUTTON_ON DK_BTN1_MSK				 /* Button ID used to switch on the light bulb. */
#define BUTTON_OFF DK_BTN2_MSK				 /* Button ID used to switch off the light bulb. */
#define NUMBER_OF_NETWORK_TIME_ELEMENTS 1000 /* Size of the Benchmark Reporting Array message_info */

#define BENCHMARK_INIT_BASIC_APP_VERSION 01									  /* Version of the application software (1 byte). */
#define BENCHMARK_INIT_BASIC_STACK_VERSION 10								  /* Version of the implementation of the Zigbee stack (1 byte). */
#define BENCHMARK_INIT_BASIC_HW_VERSION 11									  /* Version of the hardware of the device (1 byte). */
#define BENCHMARK_INIT_BASIC_MANUF_NAME "Nordic"							  /* Manufacturer name (32 bytes). */
#define BENCHMARK_INIT_BASIC_MODEL_ID "Dimable_Light_v0.1"					  /* Model number assigned by manufacturer (32-bytes long string). */
#define BENCHMARK_INIT_BASIC_DATE_CODE "20200329"							  /* First 8 bytes specify the date of manufacturer of the device in ISO 8601 format (YYYYMMDD). The rest (8 bytes) are manufacturer specific.*/
#define BENCHMARK_INIT_BASIC_POWER_SOURCE ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE /* Type of power sources available for the device. For possible values see section 3.2.2.2.8 of ZCL specification.*/
#define BENCHMARK_INIT_BASIC_LOCATION_DESC "Office desk"					  /* Describes the physical location of the device (16 bytes). May be modified during commisioning process.*/
#define BENCHMARK_INIT_BASIC_PH_ENV ZB_ZCL_BASIC_ENV_UNSPECIFIED

/* #if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE to compile light switch (End Device) source code.
#endif */

#ifndef ZB_ROUTER_ROLE
#error Define ZB_ROUTER_ROLE to compile router source code.
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

/* Main application customizable context.
 * Stores all settings and static values.
 */
typedef struct
{
	zb_zcl_basic_attrs_ext_t basic_attr;
	zb_zcl_identify_attrs_t identify_attr;
	zb_zcl_scenes_attrs_t scenes_attr;
	zb_zcl_groups_attrs_t groups_attr;
	zb_zcl_on_off_attrs_t on_off_attr;
	zb_zcl_level_control_attrs_t level_control_attr;
} bm_client_device_ctx_t;

/* Zigbee device application context storage. */
static bm_client_device_ctx_t dev_ctx;

/* Declare attribute list for Basic cluster. */
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT(
	basic_attr_list,
	&dev_ctx.basic_attr.zcl_version,
	&dev_ctx.basic_attr.app_version,
	&dev_ctx.basic_attr.stack_version,
	&dev_ctx.basic_attr.hw_version,
	dev_ctx.basic_attr.mf_name,
	dev_ctx.basic_attr.model_id,
	dev_ctx.basic_attr.date_code,
	&dev_ctx.basic_attr.power_source,
	dev_ctx.basic_attr.location_id,
	&dev_ctx.basic_attr.ph_env,
	dev_ctx.basic_attr.sw_ver);

/* Declare attribute list for Identify cluster. */
ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(identify_attr_list, &dev_ctx.identify_attr.identify_time);
//ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(identify_attr_list, &attr_identify_time);

ZB_ZCL_DECLARE_GROUPS_ATTRIB_LIST(
	groups_attr_list,
	&dev_ctx.groups_attr.name_support);

ZB_ZCL_DECLARE_SCENES_ATTRIB_LIST(
	scenes_attr_list,
	&dev_ctx.scenes_attr.scene_count,
	&dev_ctx.scenes_attr.current_scene,
	&dev_ctx.scenes_attr.current_group,
	&dev_ctx.scenes_attr.scene_valid,
	&dev_ctx.scenes_attr.name_support);

/* On/Off cluster attributes additions data */
ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST(
	on_off_attr_list,
	&dev_ctx.on_off_attr.on_off);

ZB_ZCL_DECLARE_LEVEL_CONTROL_ATTRIB_LIST(
	level_control_attr_list,
	&dev_ctx.level_control_attr.current_level,
	&dev_ctx.level_control_attr.remaining_time);

/* Declare cluster list for Dimmer Switch device (Identify, Basic, Scenes,
 * Groups, On Off, Level Control).
 * Only clusters Identify and Basic have attributes.
 */
ZB_HA_DECLARE_DIMMER_SWITCH_CLUSTER_LIST(dimmer_switch_clusters,
										 basic_attr_list,
										 identify_attr_list);

ZB_HA_DECLARE_LEVEL_CONTROLLABLE_OUTPUT_CLUSTER_LIST(bm_control_clusters,
													 basic_attr_list,
													 identify_attr_list,
													 scenes_attr_list,
													 groups_attr_list,
													 on_off_attr_list,
													 level_control_attr_list);

/* Declare endpoint for Dimmer Switch device. */
ZB_HA_DECLARE_DIMMER_SWITCH_EP(dimmer_switch_ep,
							   LIGHT_SWITCH_ENDPOINT,
							   dimmer_switch_clusters);

ZB_HA_DECLARE_LEVEL_CONTROLLABLE_OUTPUT_EP(bm_control_ep,
										   BENCHMARK_CONTROL_ENDPOINT,
										   bm_control_clusters);

/* Declare application's device context (list of registered endpoints)
 * for Dimmer Switch device.
 */
//ZB_HA_DECLARE_DIMMER_SWITCH_CTX(dimmer_switch_ctx, dimmer_switch_ep);
ZBOSS_DECLARE_DEVICE_CTX_2_EP(bm_client_ctx,
							  dimmer_switch_ep,
							  bm_control_ep);

/* Struct for benchmark message information */
typedef struct
{
	zb_ieee_addr_t dst_addr;
	zb_ieee_addr_t src_addr;
	zb_uint16_t group_addr;
	zb_uint64_t net_time;
	zb_uint16_t number_of_hops;
	zb_uint16_t message_id;
	zb_uint8_t RSSI;
	bool data_size;
} bm_message_info;

/* Forward declarations */
static void light_switch_send_on_off(zb_bufid_t bufid, zb_uint16_t on_off);
static void bm_send_message_cb(zb_bufid_t bufid, zb_uint16_t on_off);
static void bm_send_control_message_cb(zb_bufid_t bufid, zb_uint16_t level);
void bm_save_message_info(bm_message_info message);

/* Array of structs to save benchmark message info to */
bm_message_info message_info[NUMBER_OF_NETWORK_TIME_ELEMENTS] = {0};

K_THREAD_STACK_DEFINE(bm_stack_area, BM_STACK_SIZE);
struct k_thread bm_thread_data;
//static void bm_send_message(void *arg1, void *arg2, void *arg3);
static void bm_send_message(zb_uint8_t message_id);
static void bm_zigbee(void *arg1, void *arg2, void *arg3);
k_tid_t bm_thread;
u32_t button_pressed = ZB_FALSE;

zb_uint16_t bm_msg_cnt;
zb_uint16_t bm_time_interval_msec;
zb_uint16_t bm_message_info_nr = 0;

zb_ieee_addr_t local_node_ieee_addr;
zb_uint16_t local_node_short_addr;
char local_nodel_ieee_addr_buf[17] = {0};
int local_node_addr_len;

/**@brief Callback for button events.
 *
 * @param[in]   button_state  Bitmask containing buttons state.
 * @param[in]   has_changed   Bitmask containing buttons that has
 *                            changed their state.
 */
static void button_handler(u32_t button_state, u32_t has_changed)
{
	zb_bool_t on_off = ZB_FALSE;
	zb_uint8_t level = 0;
	zb_ret_t zb_err_code;

	switch (button_state)
	{
	case BUTTON_ON:
		LOG_INF("ON - Button pressed");
		button_pressed = button_state;
		break;

	case BUTTON_OFF:
		LOG_INF("OFF - Button pressed");
		button_pressed = button_state;
		break;

	case BUTTON_BENCHMARK:
		LOG_INF("BENCHMARK - Button pressed");
		button_pressed = button_state;
		break;

	case BUTTON_TEST:
		LOG_INF("TEST - Button pressed");
		button_pressed = button_state;
		break;

	case 0:
		LOG_INF("Button released");
		switch (button_pressed)
		{
		case BUTTON_ON:
			on_off = ZB_TRUE;
			zb_err_code = zb_buf_get_out_delayed_ext(light_switch_send_on_off, on_off, 0);
			ZB_ERROR_CHECK(zb_err_code);
			break;
		case BUTTON_OFF:
			on_off = ZB_FALSE;
			zb_err_code = zb_buf_get_out_delayed_ext(light_switch_send_on_off, on_off, 0);
			ZB_ERROR_CHECK(zb_err_code);
			break;
		case BUTTON_BENCHMARK:
			bm_msg_cnt = 5;
			bm_time_interval_msec = 20000;
			bm_thread = k_thread_create(&bm_thread_data, bm_stack_area,
										K_THREAD_STACK_SIZEOF(bm_stack_area),
										bm_zigbee,
										NULL, NULL, NULL,
										BM_THREAD_PRIO, 0, K_NO_WAIT);
			break;
		case BUTTON_TEST:
			level = ZB_RANDOM_VALUE(256);
			zb_err_code = zb_buf_get_out_delayed_ext(bm_send_control_message_cb, level, 0);
			ZB_ERROR_CHECK(zb_err_code);
			break;
		}
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

/**@brief Function for initializing all clusters attributes.
 */
static void bm_client_clusters_attr_init(void)
{
	/* Basic cluster attributes data */
	dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
	dev_ctx.basic_attr.app_version = BENCHMARK_INIT_BASIC_APP_VERSION;
	dev_ctx.basic_attr.stack_version = BENCHMARK_INIT_BASIC_STACK_VERSION;
	dev_ctx.basic_attr.hw_version = BENCHMARK_INIT_BASIC_HW_VERSION;

	/* Use ZB_ZCL_SET_STRING_VAL to set strings, because the first byte
	 * should contain string length without trailing zero.
	 *
	 * For example "test" string wil be encoded as:
	 *   [(0x4), 't', 'e', 's', 't']
	 */
	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.mf_name,
		BENCHMARK_INIT_BASIC_MANUF_NAME,
		ZB_ZCL_STRING_CONST_SIZE(BENCHMARK_INIT_BASIC_MANUF_NAME));

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.model_id,
		BENCHMARK_INIT_BASIC_MODEL_ID,
		ZB_ZCL_STRING_CONST_SIZE(BENCHMARK_INIT_BASIC_MODEL_ID));

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.date_code,
		BENCHMARK_INIT_BASIC_DATE_CODE,
		ZB_ZCL_STRING_CONST_SIZE(BENCHMARK_INIT_BASIC_DATE_CODE));

	dev_ctx.basic_attr.power_source = BENCHMARK_INIT_BASIC_POWER_SOURCE;

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.location_id,
		BENCHMARK_INIT_BASIC_LOCATION_DESC,
		ZB_ZCL_STRING_CONST_SIZE(BENCHMARK_INIT_BASIC_LOCATION_DESC));

	dev_ctx.basic_attr.ph_env = BENCHMARK_INIT_BASIC_PH_ENV;

	/* Identify cluster attributes data. */
	dev_ctx.identify_attr.identify_time =
		ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;

	/* On/Off cluster attributes data. */
	dev_ctx.on_off_attr.on_off = (zb_bool_t)ZB_ZCL_ON_OFF_IS_ON;

	dev_ctx.level_control_attr.current_level =
		ZB_ZCL_LEVEL_CONTROL_LEVEL_MAX_VALUE;
	dev_ctx.level_control_attr.remaining_time =
		ZB_ZCL_LEVEL_CONTROL_REMAINING_TIME_DEFAULT_VALUE;

	ZB_ZCL_SET_ATTRIBUTE(
		BENCHMARK_CONTROL_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_ON_OFF,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
		(zb_uint8_t *)&dev_ctx.on_off_attr.on_off,
		ZB_FALSE);

	ZB_ZCL_SET_ATTRIBUTE(
		BENCHMARK_CONTROL_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID,
		(zb_uint8_t *)&dev_ctx.level_control_attr.current_level,
		ZB_FALSE);
}

/* Function to send Benchmark Control Message */
static void bm_send_control_message_cb(zb_bufid_t bufid, zb_uint16_t level)
{
	/* Send Move to level request. Level value is uint8. */
	ZB_ZCL_LEVEL_CONTROL_SEND_MOVE_TO_LEVEL_REQ(bufid,
												local_node_short_addr,
												ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
												BENCHMARK_CONTROL_ENDPOINT,
												BENCHMARK_CLIENT_ENDPOINT,
												ZB_AF_HA_PROFILE_ID,
												ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
												NULL,
												level,
												0);
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
												BENCHMARK_SERVER_ENDPOINT,
												BENCHMARK_CLIENT_ENDPOINT,
												ZB_AF_HA_PROFILE_ID,
												ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
												NULL,
												level,
												0);
}

/* TODO: Description */
static void bm_send_message(zb_uint8_t message_id)
{
	zb_ret_t zb_err_code;
	zb_err_code = zb_buf_get_out_delayed_ext(bm_send_message_cb, message_id, 0);
	ZB_ERROR_CHECK(zb_err_code);
}

/* TODO: Description */
static void bm_zigbee(void *arg1, void *arg2, void *arg3)
{
	bm_message_info message;
	zb_uint8_t random_level_value;
	zb_uint16_t timeout;
	message.message_id = 0;
	message.RSSI = 0;
	message.number_of_hops = 0;
	message.data_size = 0;
	zb_get_long_address(message.src_addr);
	//message.dst_addr = 0;
	message.group_addr = GROUP_ID;
	message.net_time = 0;

	for (u8_t i = 0; i < bm_msg_cnt; i++)
	{
		timeout = ZB_RANDOM_VALUE(bm_time_interval_msec / bm_msg_cnt);
		k_sleep(K_MSEC(timeout));

		random_level_value = ZB_RANDOM_VALUE(256);

		ZB_SCHEDULE_APP_ALARM_CANCEL(bm_send_message, ZB_ALARM_ANY_PARAM);
		message.net_time = ZB_TIME_BEACON_INTERVAL_TO_MSEC(ZB_TIMER_GET());
		message.message_id = ZB_ZCL_GET_SEQ_NUM() + 1;
		LOG_INF("Benchmark Message send, TimeStamp: %lld, MessageID: %d, TimeOut: %u", message.net_time, message.message_id, timeout);

		ZB_SCHEDULE_APP_ALARM(bm_send_message, random_level_value, 0);

		bm_save_message_info(message);
	}

	LOG_INF("Benchmark finished");
}

/* TODO: Description */
void bm_save_message_info(bm_message_info message)
{
	message_info[bm_message_info_nr] = message;
	bm_message_info_nr++;
}

static void bm_send_reporting_message(zb_uint8_t bufid)
{
	LOG_INF("Send Benchmark Report");
}

/* TODO: Description */
static void bm_receive_config(zb_uint8_t bufid)
{
	LOG_INF("Received Config-Set command");
}

/**@brief Callback function for handling custom ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
static zb_uint8_t bm_zcl_handler(zb_bufid_t bufid)
{
	zb_zcl_parsed_hdr_t cmd_info;

	ZB_ZCL_COPY_PARSED_HEADER(bufid, &cmd_info);
	LOG_INF("%s with Endpoint ID: %hd, Cluster ID: %d", __func__, cmd_info.addr_data.common_data.dst_endpoint, cmd_info.cluster_id);

	switch (cmd_info.cluster_id)
	{
	case ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL:
		ZB_SCHEDULE_APP_ALARM(bm_receive_config, bufid, 0);
		break;
	case ZB_ZCL_CLUSTER_ID_ON_OFF:
		ZB_SCHEDULE_APP_ALARM(bm_send_reporting_message, bufid, 0);
		break;
	default:
		break;
	}
	/* if (cmd_info.cluster_id == ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL)
	{
		ZB_SCHEDULE_APP_ALARM(bm_receive_config, bufid, 0);
	}
	else
	{
		return ZB_FALSE;
	} */

	return ZB_FALSE;
}

/**@brief Callback function for handling ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
static zb_void_t zcl_device_cb(zb_bufid_t bufid)
{
	zb_zcl_device_callback_param_t *device_cb_param = ZB_BUF_GET_PARAM(bufid, zb_zcl_device_callback_param_t);
	LOG_INF("%s id %hd", __func__, device_cb_param->device_cb_id);

	/* Set default response value. */
	device_cb_param->status = RET_OK;

	switch (device_cb_param->device_cb_id)
	{
	case ZB_ZCL_LEVEL_CONTROL_SET_VALUE_CB_ID:
		LOG_INF("Level control setting to %d", device_cb_param->cb_param.level_control_set_value_param.new_value);

		break;
	default:
		device_cb_param->status = RET_ERROR;
		break;
	}
	LOG_INF("%s status: %hd", __func__, device_cb_param->status);
}

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer
 *                      used to pass signal.
 */
/* void zboss_signal_handler(zb_bufid_t bufid)
{
	// Update network status LED
	zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

	ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

	if (bufid)
	{
		zb_buf_free(bufid);
	}
} */

void zboss_signal_handler(zb_bufid_t bufid)
{
	zb_zdo_app_signal_hdr_t *sig_hndler = NULL;
	zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &sig_hndler);
	zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);

	/* Update network status LED. */
	zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

	switch (sig)
	{
	case ZB_BDB_SIGNAL_STEERING:
		ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
		if (status == RET_OK)
		{
			/* Read local node address */
			zb_get_long_address(local_node_ieee_addr);
			local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
			local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);
			LOG_INF("Network Steering finished with Local Node Address: Short: 0x%x, IEEE/Long: 0x%s", local_node_short_addr, local_nodel_ieee_addr_buf);

			//bufid = 0; // Do not free buffer - it will be reused by find_light_bulb callback.
		}
		break;
	case ZB_BDB_SIGNAL_DEVICE_REBOOT:
		if (status == RET_OK)
		{
			/* Read local node address */
			zb_get_long_address(local_node_ieee_addr);
			local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
			local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);
			LOG_INF("Node restarted with Local Node Address: Short: 0x%x, IEEE/Long: 0x%s", local_node_short_addr, local_nodel_ieee_addr_buf);
		}
		break;
	default:

		ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
		break;
	}

	/* All callbacks should either reuse or free passed buffers.
	 * If bufid == 0, the buffer is invalid (not passed).
	 */
	if (bufid)
	{
		zb_buf_free(bufid);
	}
}

void bm_zigbee_init(void)
{
	/* Configure GPIOs LEDs and Buttons for Zigbee */
	configure_gpio();

	/* Erase persistent Config at startup */
	zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);

	/* Initialize application context structure. */
	memset(&device_ctx, 0, sizeof(struct light_switch_ctx));

	/* Set default bulb short_addr. */
	device_ctx.bulb_params.short_addr = 0xFFFF;

	/* Register dimmer switch device context (endpoints). */
	//ZB_AF_REGISTER_DEVICE_CTX(&dimmer_switch_ctx);
	ZB_AF_REGISTER_DEVICE_CTX(&bm_client_ctx);

	/* Register callback for handling ZCL commands. */
	ZB_AF_SET_ENDPOINT_HANDLER(BENCHMARK_CLIENT_ENDPOINT, bm_zcl_handler);
	ZB_AF_SET_ENDPOINT_HANDLER(BENCHMARK_CONTROL_ENDPOINT, bm_zcl_handler);
	ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

	/* Initialzie Cluster Attributes */
	bm_client_clusters_attr_init();
}

void bm_zigbee_enable(void)
{
	/* Start Zigbee default thread */
	zigbee_enable();

	LOG_INF("BENCHMARK Server ready");
}
