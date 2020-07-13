/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 *  @brief Simple Zigbee network coordinator implementation
 */

#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <soc.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>

#include <zboss_api.h>
#include <zb_mem_config_max.h>
#include <zb_error_handler.h>
#include <zigbee_helpers.h>
#include <zb_nrf_platform.h>

#include <shell/shell.h>

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000
#define BENCHMARK_CLIENT_ENDPOINT 1			/* ZCL Endpoint of the Benchmark Client */
#define BENCHMARK_SERVER_ENDPOINT 10		/* ZCL Endpoint of the Benchmark Server */
#define BENCHMARK_CONTROL_ENDPOINT 11		/* ZCL Endpoint for Benchmark Control */

#define ERASE_PERSISTENT_CONFIG ZB_TRUE		  /* Do not erase NVRAM to save the network parameters after device reboot or    \
											   * power-off. NOTE: If this option is set to ZB_TRUE then do full device erase \
											   * for all network devices before running other samples. */
#define ZIGBEE_NETWORK_STATE_LED DK_LED3	  /* LED indicating that network is opened for new nodes */
#define KEY_ZIGBEE_NETWORK_REOPEN DK_BTN1_MSK /* Button which reopens the Zigbee Network */
#define ZIGBEE_MANUAL_STEERING ZB_FALSE		  /* If set to ZB_TRUE then device will not open the network after forming or reboot.*/
#define ZIGBEE_PERMIT_LEGACY_DEVICES ZB_FALSE

#ifndef ZB_COORDINATOR_ROLE
#error Define ZB_COORDINATOR_ROLE to compile coordinator source code.
#endif

LOG_MODULE_REGISTER(app);

static void bm_send_control_message_cb(zb_bufid_t bufid, zb_uint16_t level);

zb_ieee_addr_t local_node_ieee_addr;
zb_uint16_t local_node_short_addr;
char local_nodel_ieee_addr_buf[17] = {0};
int local_node_addr_len;

/**@brief Callback used in order to visualise network steering period.
 *
 * @param[in]   param   Not used. Required by callback type definition.
 */
static zb_void_t steering_finished(zb_uint8_t param)
{
	ARG_UNUSED(param);
	LOG_INF("Network steering finished");
	dk_set_led_off(ZIGBEE_NETWORK_STATE_LED);
}

/**@brief Callback for button events.
 *
 * @param[in]   button_state  Bitmask containing buttons state.
 * @param[in]   has_changed   Bitmask containing buttons
 *                            that has changed their state.
 */
static void button_changed(u32_t button_state, u32_t has_changed)
{
	/* Calculate bitmask of buttons that are pressed
	 * and have changed their state.
	 */
	u32_t buttons = button_state & has_changed;
	zb_bool_t comm_status;

	if (buttons & KEY_ZIGBEE_NETWORK_REOPEN)
	{
		(void)(ZB_SCHEDULE_APP_ALARM_CANCEL(
			steering_finished, ZB_ALARM_ANY_PARAM));

		comm_status = bdb_start_top_level_commissioning(
			ZB_BDB_NETWORK_STEERING);
		if (comm_status)
		{
			LOG_INF("Top level comissioning restated");
		}
		else
		{
			LOG_INF("Top level comissioning hasn't finished yet!");
		}
	}
}

/**@brief Function for initializing LEDs and Buttons. */
static void configure_gpio(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err)
	{
		LOG_ERR("Cannot init buttons (err: %d)\n", err);
	}

	err = dk_leds_init();
	if (err)
	{
		LOG_ERR("Cannot init LEDs (err: %d)\n", err);
	}
}

/* Function to send Benchmark Control Message */
static void bm_send_control_message_cb(zb_bufid_t bufid, zb_uint16_t level)
{

	zb_nwk_broadcast_address_t broadcast_addr = ZB_NWK_BROADCAST_ALL_DEVICES;

	/* Send Move to level request. Level value is uint8. */
	ZB_ZCL_LEVEL_CONTROL_SEND_MOVE_TO_LEVEL_REQ(bufid,
												broadcast_addr,
												ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
												BENCHMARK_CONTROL_ENDPOINT,
												BENCHMARK_CLIENT_ENDPOINT,
												ZB_AF_HA_PROFILE_ID,
												ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
												NULL,
												level,
												0);
}

static void cmd_handler_start_bm(void)
{
	zb_uint8_t level;
	zb_ret_t zb_err_code;

	level = ZB_RANDOM_VALUE(256);
	zb_err_code = zb_buf_get_out_delayed_ext(bm_send_control_message_cb, level, 0);
	ZB_ERROR_CHECK(zb_err_code);
}


/* CLI Command for restarting top level comissioning */
static void cmd_handler_start_top_level_comissioning(void)
{
	zb_bool_t comm_status;
	(void)(ZB_SCHEDULE_APP_ALARM_CANCEL(steering_finished, ZB_ALARM_ANY_PARAM));
	comm_status = bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);

	if (comm_status)
	{
		LOG_INF("Top level comissioning restated");
	}
	else
	{
		LOG_INF("Top level comissioning hasn't finished yet!");
	}
}
SHELL_CMD_REGISTER(start_benchmark, NULL, "Starting Benchmark on Remote Node", cmd_handler_start_bm);
SHELL_CMD_REGISTER(start_comissioning, NULL, "Restarting top level comissioning", cmd_handler_start_top_level_comissioning);

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer
 *                      used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid)
{
	/* Read signal description out of memory buffer. */
	zb_zdo_app_signal_hdr_t *sg_p = NULL;
	zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &sg_p);
	zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);
	zb_ret_t zb_err_code;
	zb_bool_t comm_status;
	zb_time_t timeout_bi;

	switch (sig)
	{
	case ZB_BDB_SIGNAL_DEVICE_REBOOT:
		/* BDB initialization completed after device reboot,
		 * use NVRAM contents during initialization.
		 * Device joined/rejoined and started.
		 */
		if (status == RET_OK)
		{
			if (ZIGBEE_MANUAL_STEERING == ZB_FALSE)
			{
				LOG_INF("Start network steering");
				comm_status = bdb_start_top_level_commissioning(
					ZB_BDB_NETWORK_STEERING);
				ZB_COMM_STATUS_CHECK(comm_status);
			}
			else
			{
				LOG_INF("Coordinator restarted successfully");
			}
		}
		else
		{
			LOG_ERR("Failed to initialize Zigbee stack using NVRAM data (status: %d)",
					status);
		}
		break;

	case ZB_BDB_SIGNAL_STEERING:
		if (status == RET_OK)
		{
			if (ZIGBEE_PERMIT_LEGACY_DEVICES == ZB_TRUE)
			{
				LOG_INF("Allow pre-Zigbee 3.0 devices to join the network");
				zb_bdb_set_legacy_device_support(1);
			}

			/* Schedule an alarm to notify about the end
			 * of steering period
			 */
			LOG_INF("Network steering started");
			zb_err_code = ZB_SCHEDULE_APP_ALARM(
				steering_finished, 0,
				ZB_TIME_ONE_SECOND *
					ZB_ZGP_DEFAULT_COMMISSIONING_WINDOW);
			ZB_ERROR_CHECK(zb_err_code);
		}
		break;

	case ZB_ZDO_SIGNAL_DEVICE_ANNCE:
	{
		zb_zdo_signal_device_annce_params_t *dev_annce_params =
			ZB_ZDO_SIGNAL_GET_PARAMS(
				sg_p, zb_zdo_signal_device_annce_params_t);

		LOG_INF("New device commissioned or rejoined (short: 0x%04hx)",
				dev_annce_params->device_short_addr);

		zb_err_code = ZB_SCHEDULE_APP_ALARM_CANCEL(steering_finished,
												   ZB_ALARM_ANY_PARAM);
		if (zb_err_code == RET_OK)
		{
			LOG_INF("Joining period extended.");
			zb_err_code = ZB_SCHEDULE_APP_ALARM(
				steering_finished, 0,
				ZB_TIME_ONE_SECOND *
					ZB_ZGP_DEFAULT_COMMISSIONING_WINDOW);
			ZB_ERROR_CHECK(zb_err_code);
		}
	}
	break;

	default:
		/* Call default signal handler. */
		ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
		break;
	}

	/* Update network status LED */
	if (ZB_JOINED() &&
		(ZB_SCHEDULE_GET_ALARM_TIME(steering_finished, ZB_ALARM_ANY_PARAM,
									&timeout_bi) == RET_OK))
	{
		dk_set_led_on(ZIGBEE_NETWORK_STATE_LED);
	}
	else
	{
		dk_set_led_off(ZIGBEE_NETWORK_STATE_LED);
	}

	/*
	 * All callbacks should either reuse or free passed buffers.
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
		/* Spin for ever */
		k_sleep(K_MSEC(1000));
	}
}

void main(void)
{
	int blink_status = 0;

	LOG_INF("Starting ZBOSS Coordinator example\n");

	/* Initialize */
	configure_gpio();

	/* Start Zigbee default thread */
	zigbee_enable();

	LOG_INF("ZBOSS Coordinator example started\n");

	while (1)
	{
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}
