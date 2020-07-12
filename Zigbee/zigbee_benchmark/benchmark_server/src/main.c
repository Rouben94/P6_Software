/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 *
 * @brief Simple Zigbee light bulb implementation.
 */

#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <soc.h>
#include <drivers/pwm.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>
#include <drivers/gpio.h>

#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zigbee_helpers.h>
#include <zb_error_handler.h>
#include <zb_nrf_platform.h>
#include <zb_mem_config_med.h>

#define BM_STACK_SIZE 500						/* Benchmark Stack Size */
#define BM_THREAD_PRIO 5						/* Benchmark Thread Priority */
#define GROUP_ID 0xB331							/* Group ID which will be used to address a specific group of Benchmark Servers */
#define BENCHMARK_CLIENT_ENDPOINT 1				/* ZCL Endpoint of the Benchmark Client */
#define BENCHMARK_SERVER_ENDPOINT 10			/* ZCL Endpoint of the Benchmark Server */
#define BULB_LED DK_LED4						/* LED immitaing dimmable light bulb */
#define DONGLE_BUTTON DK_BTN1_MSK				/* Button on the Dongle */
#define ZIGBEE_NETWORK_STATE_LED DK_LED1		/* LED indicating that light switch successfully joind Zigbee network. */
#define PWM_DK_LED4_NODE DT_NODELABEL(pwm_led3) /* Use onboard led4 to act as a light bulb. The app.overlay file has this at node label "pwm_led3" in /pwmleds. */

#define MATCH_DESC_REQ_START_DELAY (2 * ZB_TIME_ONE_SECOND) /* Delay between the light switch startup and light bulb finding procedure. */

/* Nordic PWM nodes don't have flags cells in their specifiers, so this is just future-proofing.*/
#define FLAGS_OR_ZERO(node)                         \
	COND_CODE_1(DT_PHA_HAS_CELL(node, pwms, flags), \
				(DT_PWMS_FLAGS(node)), (0))
#if DT_NODE_HAS_STATUS(PWM_DK_LED4_NODE, okay) /* Get the defines from overlay file. */
#define PWM_DK_LED4_DRIVER DT_PWMS_LABEL(PWM_DK_LED4_NODE)
#define PWM_DK_LED4_CHANNEL DT_PWMS_CHANNEL(PWM_DK_LED4_NODE)
#define PWM_DK_LED4_FLAGS FLAGS_OR_ZERO(PWM_DK_LED4_NODE)
#else
#error "Choose supported PWM driver"
#endif

/*
nRF52840-Dongle LEDs and Buttons
DK_LED1 --> green
DK_LED2 --> red
DK_LED3 --> green
DK_LED4 --> blue
DK_BTN1 --> Button 1
*/
#define HA_DIMMABLE_LIGHT_ENDPOINT 10									 /* Device endpoint, used to receive light controlling commands. */
#define BULB_INIT_BASIC_APP_VERSION 01									 /* Version of the application software (1 byte). */
#define BULB_INIT_BASIC_STACK_VERSION 10								 /* Version of the implementation of the Zigbee stack (1 byte). */
#define BULB_INIT_BASIC_HW_VERSION 11									 /* Version of the hardware of the device (1 byte). */
#define BULB_INIT_BASIC_MANUF_NAME "Nordic"								 /* Manufacturer name (32 bytes). */
#define BULB_INIT_BASIC_MODEL_ID "Dimable_Light_v0.1"					 /* Model number assigned by manufacturer (32-bytes long string). */
#define BULB_INIT_BASIC_DATE_CODE "20200329"							 /* First 8 bytes specify the date of manufacturer of the device in ISO 8601 format (YYYYMMDD). The rest (8 bytes) are manufacturer specific.*/
#define BULB_INIT_BASIC_POWER_SOURCE ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE /* Type of power sources available for the device. For possible values see section 3.2.2.2.8 of ZCL specification.*/
#define BULB_INIT_BASIC_LOCATION_DESC "Office desk"						 /* Describes the physical location of the device (16 bytes). May be modified during commisioning process.*/
#define BULB_INIT_BASIC_PH_ENV ZB_ZCL_BASIC_ENV_UNSPECIFIED				 /* Describes the type of physical environment. For possible values see section 3.2.2.2.10 of ZCL specification.*/
#define LED_PWM_PERIOD_US (USEC_PER_SEC / 50U)							 /* Led PWM period, calculated for 50 Hz signal - in microseconds. */
#define NUMBER_OF_NETWORK_TIME_ELEMENTS 1000							 /* Size of the Benchmark Reporting Array message_info */

#ifndef ZB_ROUTER_ROLE
#error Define ZB_ROUTER_ROLE to compile router source code.
#endif

LOG_MODULE_REGISTER(app);

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
} bulb_device_ctx_t;

/* Zigbee device application context storage. */
static bulb_device_ctx_t dev_ctx;

/* Pointer to PWM device controlling leds with pwm signal. */
static struct device *led_pwm_dev;

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(
	identify_attr_list,
	&dev_ctx.identify_attr.identify_time);

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

/* On/Off cluster attributes additions data */
ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST(
	on_off_attr_list,
	&dev_ctx.on_off_attr.on_off);

ZB_ZCL_DECLARE_LEVEL_CONTROL_ATTRIB_LIST(
	level_control_attr_list,
	&dev_ctx.level_control_attr.current_level,
	&dev_ctx.level_control_attr.remaining_time);

ZB_HA_DECLARE_DIMMABLE_LIGHT_CLUSTER_LIST(
	dimmable_light_clusters,
	basic_attr_list,
	identify_attr_list,
	groups_attr_list,
	scenes_attr_list,
	on_off_attr_list,
	level_control_attr_list);

ZB_HA_DECLARE_DIMMABLE_LIGHT_EP(
	dimmable_light_ep,
	HA_DIMMABLE_LIGHT_ENDPOINT,
	dimmable_light_clusters);

ZB_HA_DECLARE_DIMMABLE_LIGHT_CTX(
	dimmable_light_ctx,
	dimmable_light_ep);

/* Struct for benchmark message information */
typedef struct
{
	zb_ieee_addr_t ieee_dst_addr;
	zb_uint16_t src_addr;
	zb_uint16_t dst_addr;
	zb_uint16_t group_addr;
	zb_uint64_t net_time;
	zb_uint16_t number_of_hops;
	zb_uint16_t message_id;
	zb_uint8_t RSSI;
	bool data_size;
} bm_message_info;

void bm_save_message_info(bm_message_info message);
static void bm_receive_message(zb_uint8_t param);

zb_uint16_t bm_message_info_nr = 0;
char ieee_addr_buf[17] = {0};
int addr_len;

/* Array of structs to save benchmark message info to */
bm_message_info message_info[NUMBER_OF_NETWORK_TIME_ELEMENTS] = {0};

/**@brief Callback for button events.
 *
 * @param[in]   button_state  Bitmask containing buttons state.
 * @param[in]   has_changed   Bitmask containing buttons
 *                            that have changed their state.
 */
static void button_changed(u32_t button_state, u32_t has_changed)
{
	//k_tid_t add_group_thread;

	/* Calculate bitmask of buttons that are pressed
	 * and have changed their state.
	 */
	u32_t buttons = button_state & has_changed;

	if (buttons & DONGLE_BUTTON)
	{
		LOG_INF("ADD GROUP - Button pressed");
	}
}

/**@brief Function for initializing additional PWM leds. */
static void pwm_led_init(void)
{
	led_pwm_dev = device_get_binding(PWM_DK_LED4_DRIVER);

	if (!led_pwm_dev)
	{
		LOG_ERR("Cannot find %s!", PWM_DK_LED4_DRIVER);
	}
}

/**@brief Function for initializing LEDs and Buttons. */
static void configure_gpio(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err)
	{
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	err = dk_leds_init();
	if (err)
	{
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}

	pwm_led_init();
}

/**@brief Sets brightness of bulb luminous executive element
 *
 * @param[in] brightness_level Brightness level, allowed values 0 ... 255,
 *                             0 - turn off, 255 - full brightness.
 */
static void light_bulb_set_brightness(zb_uint8_t brightness_level)
{
	u32_t pulse = brightness_level * LED_PWM_PERIOD_US / 255U;

	if (pwm_pin_set_usec(led_pwm_dev, PWM_DK_LED4_CHANNEL,
						 LED_PWM_PERIOD_US, pulse, PWM_DK_LED4_FLAGS))
	{
		LOG_ERR("Pwm led 4 set fails:\n");
		return;
	}
}

/**@brief Function for setting the light bulb brightness.
 *
 * @param[in] new_level   Light bulb brightness value.
 */
static void level_control_set_value(zb_uint16_t new_level)
{
	LOG_INF("Set level value: %i", new_level);

	ZB_ZCL_SET_ATTRIBUTE(
		HA_DIMMABLE_LIGHT_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID,
		(zb_uint8_t *)&new_level,
		ZB_FALSE);

	/* According to the table 7.3 of Home Automation Profile Specification
	 * v 1.2 rev 29, chapter 7.1.3.
	 */
	if (new_level == 0)
	{
		zb_uint8_t value = ZB_FALSE;

		ZB_ZCL_SET_ATTRIBUTE(
			HA_DIMMABLE_LIGHT_ENDPOINT,
			ZB_ZCL_CLUSTER_ID_ON_OFF,
			ZB_ZCL_CLUSTER_SERVER_ROLE,
			ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
			&value,
			ZB_FALSE);
	}
	else
	{
		zb_uint8_t value = ZB_TRUE;

		ZB_ZCL_SET_ATTRIBUTE(
			HA_DIMMABLE_LIGHT_ENDPOINT,
			ZB_ZCL_CLUSTER_ID_ON_OFF,
			ZB_ZCL_CLUSTER_SERVER_ROLE,
			ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
			&value,
			ZB_FALSE);
	}

	light_bulb_set_brightness(new_level);
}

/**@brief Function for turning ON/OFF the light bulb.
 *
 * @param[in]   on   Boolean light bulb state.
 */
static void on_off_set_value(zb_bool_t on)
{
	LOG_INF("Set ON/OFF value: %i", on);

	ZB_ZCL_SET_ATTRIBUTE(
		HA_DIMMABLE_LIGHT_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_ON_OFF,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
		(zb_uint8_t *)&on,
		ZB_FALSE);

	if (on)
	{
		level_control_set_value(dev_ctx.level_control_attr.current_level);
	}
	else
	{
		light_bulb_set_brightness(0U);
	}
}

/**@brief Function for initializing all clusters attributes.
 */
static void bulb_clusters_attr_init(void)
{
	/* Basic cluster attributes data */
	dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
	dev_ctx.basic_attr.app_version = BULB_INIT_BASIC_APP_VERSION;
	dev_ctx.basic_attr.stack_version = BULB_INIT_BASIC_STACK_VERSION;
	dev_ctx.basic_attr.hw_version = BULB_INIT_BASIC_HW_VERSION;

	/* Use ZB_ZCL_SET_STRING_VAL to set strings, because the first byte
	 * should contain string length without trailing zero.
	 *
	 * For example "test" string wil be encoded as:
	 *   [(0x4), 't', 'e', 's', 't']
	 */
	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.mf_name,
		BULB_INIT_BASIC_MANUF_NAME,
		ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_MANUF_NAME));

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.model_id,
		BULB_INIT_BASIC_MODEL_ID,
		ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_MODEL_ID));

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.date_code,
		BULB_INIT_BASIC_DATE_CODE,
		ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_DATE_CODE));

	dev_ctx.basic_attr.power_source = BULB_INIT_BASIC_POWER_SOURCE;

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.location_id,
		BULB_INIT_BASIC_LOCATION_DESC,
		ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_LOCATION_DESC));

	dev_ctx.basic_attr.ph_env = BULB_INIT_BASIC_PH_ENV;

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
		HA_DIMMABLE_LIGHT_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_ON_OFF,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
		(zb_uint8_t *)&dev_ctx.on_off_attr.on_off,
		ZB_FALSE);

	ZB_ZCL_SET_ATTRIBUTE(
		HA_DIMMABLE_LIGHT_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID,
		(zb_uint8_t *)&dev_ctx.level_control_attr.current_level,
		ZB_FALSE);
}

static void add_group_id_cb(zb_bufid_t bufid)
{
	zb_ieee_addr_t ieee_addr;
	zb_get_long_address(ieee_addr);
	addr_len = ieee_addr_to_str(ieee_addr_buf, sizeof(ieee_addr_buf), ieee_addr);

	LOG_INF("Include device 0x%s, ep %d to the group 0x%x", ieee_addr_buf, BENCHMARK_SERVER_ENDPOINT, GROUP_ID);

	ZB_ZCL_GROUPS_SEND_ADD_GROUP_REQ(bufid,
									 ieee_addr,
									 ZB_APS_ADDR_MODE_64_ENDP_PRESENT,
									 BENCHMARK_SERVER_ENDPOINT,
									 BENCHMARK_CLIENT_ENDPOINT,
									 ZB_AF_HA_PROFILE_ID,
									 ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
									 NULL,
									 GROUP_ID);
}

void bm_save_message_info(bm_message_info message)
{
	message_info[bm_message_info_nr] = message;
	bm_message_info_nr++;
}

/* TODO: Description */
static void bm_receive_message(zb_uint8_t bufid)
{
	zb_zcl_parsed_hdr_t cmd_info;
	ZB_ZCL_COPY_PARSED_HEADER(bufid, &cmd_info);

	LOG_INF("Receiving message");
	bm_message_info message;
	zb_uint8_t lqi = ZB_MAC_LQI_UNDEFINED;
	zb_uint8_t rssi = ZB_MAC_RSSI_UNDEFINED;
	zb_uint8_t addr_type;

	message.message_id = cmd_info.seq_number;
	memcpy(&message.src_addr, &(cmd_info.addr_data.common_data.source.u.short_addr), sizeof(zb_uint16_t));
	addr_type = cmd_info.addr_data.common_data.source.addr_type;

	zb_get_long_address(message.ieee_dst_addr);
	message.dst_addr = zb_address_short_by_ieee(message.ieee_dst_addr);
	message.group_addr = GROUP_ID;

	zb_zdo_get_diag_data(message.src_addr, &lqi, &rssi);
	message.RSSI = rssi;
	message.number_of_hops = 0;
	//message.data_size = ZB_TRUE;

	message.net_time = ZB_TIME_BEACON_INTERVAL_TO_MSEC(ZB_TIMER_GET());

	LOG_INF("Benchmark Packet received with ID: %d from Src Address: 0x%x to Destination 0x%x with RSSI: %d, LQI: %d, Time: %llu", message.message_id, message.src_addr, message.dst_addr, rssi, lqi, message.net_time);

	bm_save_message_info(message);
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

	if (cmd_info.cluster_id == ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL)
	{
		ZB_SCHEDULE_APP_ALARM(bm_receive_message, bufid, 0);
	}
	else
	{
		return ZB_FALSE;
	}

	return ZB_FALSE;
}

/**@brief Callback function for handling standard ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
static zb_void_t zcl_device_cb(zb_bufid_t bufid)
{
	zb_uint8_t cluster_id;
	zb_uint8_t attr_id;
	zb_zcl_device_callback_param_t *device_cb_param = ZB_BUF_GET_PARAM(bufid, zb_zcl_device_callback_param_t);

	LOG_INF("%s id %hd", __func__, device_cb_param->device_cb_id);

	// Set default response value.
	device_cb_param->status = RET_OK;

	switch (device_cb_param->device_cb_id)
	{
	case ZB_ZCL_LEVEL_CONTROL_SET_VALUE_CB_ID:
		LOG_INF("Level control setting to %d", device_cb_param->cb_param.level_control_set_value_param.new_value);
		level_control_set_value(device_cb_param->cb_param.level_control_set_value_param.new_value);
		/* bm_thread = k_thread_create(&bm_thread_data, bm_stack_area,
									K_THREAD_STACK_SIZEOF(bm_stack_area),
									bm_zigbee_receive_message,
									NULL, NULL, NULL,
									BM_THREAD_PRIO, 0, K_NO_WAIT); */

		break;

	case ZB_ZCL_SET_ATTR_VALUE_CB_ID:
		cluster_id = device_cb_param->cb_param.set_attr_value_param.cluster_id;
		attr_id = device_cb_param->cb_param.set_attr_value_param.attr_id;

		if (cluster_id == ZB_ZCL_CLUSTER_ID_ON_OFF)
		{
			u8_t value = device_cb_param->cb_param.set_attr_value_param.values.data8;

			LOG_INF("on/off attribute setting to %hd", value);
			if (attr_id == ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID)
			{
				on_off_set_value((zb_bool_t)value);
			}
		}
		else if (cluster_id == ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL)
		{
			u16_t value = device_cb_param->cb_param.set_attr_value_param.values.data16;

			LOG_INF("level control attribute setting to %hd",
					value);
			if (attr_id ==
				ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID)
			{
				level_control_set_value(value);
			}
		}
		else
		{
			// Other clusters can be processed here
			LOG_INF("Unhandled cluster attribute id: %d",
					cluster_id);
		}
		break;

	default:
		device_cb_param->status = RET_ERROR;
		break;
	}
}

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer
 *                      used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid)
{
	zb_zdo_app_signal_hdr_t *sig_hndler = NULL;
	zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &sig_hndler);
	zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);
	zb_ret_t zb_err_code;

	/* Update network status LED. */
	zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

	switch (sig)
	{
	case ZB_BDB_SIGNAL_STEERING:
		ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
		if (status == RET_OK)
		{
			//add_group_id_cb(zb_buf_get(ZB_TRUE,zb_buf_get_max_size(bufid)));
			zb_err_code = ZB_SCHEDULE_APP_ALARM(add_group_id_cb, bufid, MATCH_DESC_REQ_START_DELAY);
			ZB_ERROR_CHECK(zb_err_code);
			LOG_INF("Network Steering");
			bufid = 0; // Do not free buffer - it will be reused by find_light_bulb callback.
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

void error(void)
{
	dk_set_leds_state(DK_ALL_LEDS_MSK, DK_NO_LEDS_MSK);

	while (true)
	{
		/* Spin forever */
		k_sleep(K_MSEC(1000));
	}
}

void main(void)
{
	//int blink_status = 0;

	LOG_INF("Starting ZBOSS Light Bulb example");

	/* Initialize */
	configure_gpio();

	/* Register dimmer switch device context (endpoints). */
	ZB_AF_REGISTER_DEVICE_CTX(&dimmable_light_ctx);

	/* Register callback for handling ZCL commands. */
	ZB_AF_SET_ENDPOINT_HANDLER(BENCHMARK_SERVER_ENDPOINT, bm_zcl_handler);
	ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

	bulb_clusters_attr_init();
	level_control_set_value(dev_ctx.level_control_attr.current_level);

	/* Start Zigbee default thread */
	zigbee_enable();

	LOG_INF("ZBOSS Light Bulb example started");

	while (1)
	{
		//dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		//k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}
