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


#define RUN_STATUS_LED             DK_LED1
#define RUN_LED_BLINK_INTERVAL     K_MSEC(1000)

/* Source endpoint used to control light bulb. */
#define LIGHT_SWITCH_ENDPOINT      1
/* Delay between the light switch startup and light bulb finding procedure. */
#define MATCH_DESC_REQ_START_DELAY (2 * ZB_TIME_ONE_SECOND)
/* Timeout for finding procedure. */
#define MATCH_DESC_REQ_TIMEOUT     (5 * ZB_TIME_ONE_SECOND)
/* Find only non-sleepy device. */
#define MATCH_DESC_REQ_ROLE        ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE

/* Do not erase NVRAM to save the network parameters after device reboot or
 * power-off. NOTE: If this option is set to ZB_TRUE then do full device erase
 * for all network devices before running other samples.
 */
#define ERASE_PERSISTENT_CONFIG    ZB_FALSE
/* LED indicating that light switch successfully joind Zigbee network. */
#define ZIGBEE_NETWORK_STATE_LED   DK_LED3
/* LED indicating that light witch found a light bulb to control. */
#define BULB_FOUND_LED             DK_LED4
/* Button ID used to switch on the light bulb. */
#define BUTTON_ON                  DK_BTN1_MSK
/* Button ID used to switch off the light bulb. */
#define BUTTON_OFF                 DK_BTN2_MSK
/* Dim step size - increases/decreses current level (range 0x000 - 0xfe). */
#define DIMM_STEP                  15
/* Button ID used to enable sleepy behavior. */
#define BUTTON_SLEEPY              DK_BTN3_MSK

#define DEFAULT_GROUP_ID                    0xB331                              /**< Group ID, which will be used to control all light sources with a single command. */


/* Transition time for a single step operation in 0.1 sec units.
 * 0xFFFF - immediate change.
 */
#define DIMM_TRANSACTION_TIME      2

/* Time after which the button state is checked again to detect button hold,
 * the dimm command is sent again.
 */
#define BUTTON_LONG_POLL_TMO   ZB_MILLISECONDS_TO_BEACON_INTERVAL(500)

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE to compile light switch (End Device) source code.
#endif

LOG_MODULE_REGISTER(app);

struct light_switch_bulb_params {
	zb_uint8_t  endpoint;
	zb_uint16_t short_addr;
};

struct light_switch_button {
	atomic_t in_progress;
	atomic_t long_poll;
};

struct light_switch_ctx {
	struct light_switch_bulb_params bulb_params;
	struct light_switch_button      button;
};

static struct light_switch_ctx device_ctx;
static zb_uint8_t  attr_zcl_version = ZB_ZCL_VERSION;
static zb_uint8_t  attr_power_source = ZB_ZCL_BASIC_POWER_SOURCE_UNKNOWN;
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

/* Forward declarations */
static void light_switch_button_handler(zb_uint8_t button);
static void find_light_bulb(zb_bufid_t bufid);
static void light_switch_send_on_off(zb_bufid_t bufid, zb_uint16_t on_off);


/**@brief Callback for button events.
 *
 * @param[in]   button_state  Bitmask containing buttons state.
 * @param[in]   has_changed   Bitmask containing buttons that has
 *                            changed their state.
 */
static void button_handler(u32_t button_state, u32_t has_changed)
{
	zb_bool_t on_off;
	zb_ret_t zb_err_code;

	/* Inform default signal handler about user input at the device. */
	user_input_indicate();

	//if (device_ctx.bulb_params.short_addr == 0xFFFF) {
	//	LOG_INF("No bulb found yet.");
	//	return;
	//}

	switch (has_changed) {
	case BUTTON_ON:
		LOG_INF("ON - button changed");
		on_off = ZB_TRUE;
		break;
	case BUTTON_OFF:
		LOG_INF("OFF - button changed");
		on_off = ZB_FALSE;
		break;
	default:
		LOG_INF("Unhandled button");
		return;
	}

	switch (button_state) {
	case BUTTON_ON:
	case BUTTON_OFF:
		LOG_INF("Button pressed");
		atomic_set(&device_ctx.button.in_progress, ZB_TRUE);
		zb_err_code = zigbee_schedule_alarm(light_switch_button_handler,
						    button_state,
						    BUTTON_LONG_POLL_TMO);
		if (zb_err_code == RET_OVERFLOW) {
			LOG_WRN("Can't schedule another alarm, queue is full.");
			atomic_set(&device_ctx.button.in_progress, ZB_FALSE);
		} else {
			ZB_ERROR_CHECK(zb_err_code);
		}
		break;
	case 0:
		LOG_DBG("Button released");

		ZB_SCHEDULE_APP_ALARM_CANCEL(light_switch_button_handler,
					     ZB_ALARM_ANY_PARAM);
		atomic_set(&device_ctx.button.in_progress, ZB_FALSE);

		if (atomic_set(&device_ctx.button.long_poll, ZB_FALSE)
		    == ZB_FALSE) {
			/* Allocate output buffer and send on/off command. */
			zb_err_code = zb_buf_get_out_delayed_ext(
					   light_switch_send_on_off, on_off, 0);
			ZB_ERROR_CHECK(zb_err_code);
		}
	}
}

/**@brief Function for initializing LEDs and Buttons. */
static void configure_gpio(void)
{
	int err;

	err = dk_buttons_init(button_handler);
	if (err) {
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	err = dk_leds_init();
	if (err) {
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
	zb_uint16_t          group_id = DEFAULT_GROUP_ID;

	LOG_INF("Send ON/OFF command: %d to group id: %d", on_off, group_id);

/* 	ZB_ZCL_ON_OFF_SEND_REQ(bufid,
			       device_ctx.bulb_params.short_addr,
			       ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
			       device_ctx.bulb_params.endpoint,
			       LIGHT_SWITCH_ENDPOINT,
			       ZB_AF_HA_PROFILE_ID,
			       ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
			       cmd_id,
			       NULL); */

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

/**@brief Function for sending step requests to the light bulb.
 *
 * @param[in]   bufid        Non-zero reference to Zigbee stack buffer that
 *                           will be used to construct step request.
 * @param[in]   is_step_up   Boolean parameter selecting direction
 *                           of step change.
 */
static void light_switch_send_step(zb_bufid_t bufid, zb_uint16_t is_step_up)
{
	u8_t step_dir = is_step_up ? ZB_ZCL_LEVEL_CONTROL_STEP_MODE_UP :
				     ZB_ZCL_LEVEL_CONTROL_STEP_MODE_DOWN;
	zb_uint16_t          group_id = DEFAULT_GROUP_ID;

	LOG_INF("Send step level command: %d", is_step_up);

/* 	ZB_ZCL_LEVEL_CONTROL_SEND_STEP_REQ(bufid,
					   device_ctx.bulb_params.short_addr,
					   ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
					   device_ctx.bulb_params.endpoint,
					   LIGHT_SWITCH_ENDPOINT,
					   ZB_AF_HA_PROFILE_ID,
					   ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
					   NULL,
					   step_dir,
					   DIMM_STEP,
					   DIMM_TRANSACTION_TIME); */
	
	ZB_ZCL_LEVEL_CONTROL_SEND_STEP_REQ(bufid,
					   group_id,
					   ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT,
					   0,
					   LIGHT_SWITCH_ENDPOINT,
					   ZB_AF_HA_PROFILE_ID,
					   ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
					   NULL,
					   step_dir,
					   DIMM_STEP,
					   DIMM_TRANSACTION_TIME);
	
}

/**@brief Function for sending add group request. As a result all light bulb's
 *        light controlling endpoints will participate in the same group.
 *
 * @param[in]   bufid   Non-zero reference to Zigbee stack buffer that will be used to construct add group request.
 */
static void add_group(zb_bufid_t bufid)
{
    zb_zdo_match_desc_resp_t   * p_resp = (zb_zdo_match_desc_resp_t *) zb_buf_begin(bufid);    // Get the begining of the response
    zb_apsde_data_indication_t * p_ind  = ZB_BUF_GET_PARAM(bufid, zb_apsde_data_indication_t); // Get the pointer to the parameters buffer, which stores APS layer response
    zb_uint16_t                  dst_addr = p_ind->src_addr;
    zb_uint8_t                   dst_ep = *(zb_uint8_t *)(p_resp + 1);

    LOG_INF("Include device 0x%x, ep %d to the group 0x%x", dst_addr, dst_ep, DEFAULT_GROUP_ID);

    ZB_ZCL_GROUPS_SEND_ADD_GROUP_REQ(bufid,
                                     dst_addr,
                                     ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                     dst_ep,
                                     LIGHT_SWITCH_ENDPOINT,
                                     ZB_AF_HA_PROFILE_ID,
                                     ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
                                     NULL,
                                     DEFAULT_GROUP_ID);
}

/**@brief Callback function receiving finding procedure results.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer used to pass received data.
 */
static zb_void_t find_light_bulb_cb(zb_bufid_t bufid)
{
    zb_zdo_match_desc_resp_t   * p_resp = (zb_zdo_match_desc_resp_t *) zb_buf_begin(bufid);    // Get the begining of the response
    zb_apsde_data_indication_t * p_ind  = ZB_BUF_GET_PARAM(bufid, zb_apsde_data_indication_t); // Get the pointer to the parameters buffer, which stores APS layer response
    zb_uint8_t                 * p_match_ep;
    zb_ret_t                     zb_err_code;
    static zb_bool_t             bulb_found = ZB_FALSE;

    if ((p_resp->status == ZB_ZDP_STATUS_SUCCESS) && (p_resp->match_len > 0))
    {
        /* Match EP list follows right after response header */
        p_match_ep = (zb_uint8_t *)(p_resp + 1);

        LOG_INF("Found bulb addr: 0x%x ep: %d", p_ind->src_addr, *p_match_ep);
        bulb_found = ZB_TRUE;

        zb_err_code = ZB_SCHEDULE_APP_CALLBACK(add_group, bufid);
        ZB_ERROR_CHECK(zb_err_code);
        bufid = 0;

		if (bulb_found)
        {
			dk_set_led_on(BULB_FOUND_LED);
        }
    }
    else if (p_resp->status == ZB_ZDP_STATUS_TIMEOUT)
    {
		if (bulb_found)
        {
            dk_set_led_off(BULB_FOUND_LED);
			k_sleep(K_MSEC(200));
			dk_set_led_on(BULB_FOUND_LED);
			k_sleep(K_MSEC(200));
			dk_set_led_off(BULB_FOUND_LED);
			k_sleep(K_MSEC(200));
			dk_set_led_on(BULB_FOUND_LED);
			LOG_INF("Bulb found, Finding Light Bulbs finished");
        }
		else
		{
			LOG_INF("Bulb not found, try again");
       		zb_err_code = ZB_SCHEDULE_APP_ALARM(find_light_bulb, bufid, MATCH_DESC_REQ_START_DELAY);
        	ZB_ERROR_CHECK(zb_err_code);
        	bufid = 0; // Do not free buffer - it will be reused by find_light_bulb callback
		}
    }

    if (bufid)
    {
        zb_buf_free(bufid);
    }
}

/**@brief Function for sending ON/OFF and Level Control find request.
 *
 * @param[in]   bufid   Non-zero reference to Zigbee stack buffer that will be
 *                      used to construct find request.
 */
static void find_light_bulb(zb_bufid_t bufid)
{
	zb_zdo_match_desc_param_t * p_req;

    /* Initialize pointers inside buffer and reserve space for zb_zdo_match_desc_param_t request */
    p_req = zb_buf_initial_alloc(bufid, sizeof(zb_zdo_match_desc_param_t) + (1) * sizeof(zb_uint16_t));

    p_req->nwk_addr         = MATCH_DESC_REQ_ROLE;              // Send to devices specified by MATCH_DESC_REQ_ROLE
    p_req->addr_of_interest = MATCH_DESC_REQ_ROLE;              // Get responses from devices specified by MATCH_DESC_REQ_ROLE
    p_req->profile_id       = ZB_AF_HA_PROFILE_ID;              // Look for Home Automation profile clusters

    /* We are searching for 2 clusters: On/Off and Level Control Server */
    p_req->num_in_clusters  = 2;
    p_req->num_out_clusters = 0;
    /*lint -save -e415 // Suppress warning 415 "likely access of out-of-bounds pointer" */
    p_req->cluster_list[0]  = ZB_ZCL_CLUSTER_ID_ON_OFF;
    p_req->cluster_list[1]  = ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL;
    /*lint -restore */
	(void)zb_zdo_match_desc_req(bufid, find_light_bulb_cb);
}

/**@brief Callback for detecting button press duration.
 *
 * @param[in]   button   BSP Button that was pressed.
 */
static void light_switch_button_handler(zb_uint8_t button)
{
	zb_ret_t zb_err_code;
	zb_bool_t on_off;

	if (dk_get_buttons() & button) {
		atomic_set(&device_ctx.button.long_poll, ZB_TRUE);
		on_off = (button == BUTTON_ON) ? ZB_TRUE : ZB_FALSE;

		/* Allocate output buffer and send step command. */
		zb_err_code = zb_buf_get_out_delayed_ext(light_switch_send_step,
							 on_off, 0);
		ZB_ERROR_CHECK(zb_err_code);

		zb_err_code = zigbee_schedule_alarm(light_switch_button_handler,
						  button, BUTTON_LONG_POLL_TMO);
		if (zb_err_code == RET_OVERFLOW) {
			LOG_WRN("Can't schedule another alarm, queue is full.");
			atomic_set(&device_ctx.button.in_progress, ZB_FALSE);
		} else {
			ZB_ERROR_CHECK(zb_err_code);
		}
	} else {
		atomic_set(&device_ctx.button.long_poll, ZB_FALSE);
	}
}

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer
 *                      used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid)
{
	zb_zdo_app_signal_hdr_t    *sig_hndler = NULL;
	zb_zdo_app_signal_type_t    sig = zb_get_app_signal(bufid, &sig_hndler);
	zb_ret_t                    status = ZB_GET_APP_SIGNAL_STATUS(bufid);
	zb_ret_t                    zb_err_code;

	/* Update network status LED */
	zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

	switch (sig) {
	case ZB_BDB_SIGNAL_DEVICE_REBOOT:
		/* fall-through */
	case ZB_BDB_SIGNAL_STEERING:
		/* Call default signal handler. */
		ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
		if (status == RET_OK) 
		{
			zb_err_code = ZB_SCHEDULE_APP_ALARM(find_light_bulb, bufid, MATCH_DESC_REQ_START_DELAY);
            ZB_ERROR_CHECK(zb_err_code);
            bufid = 0; // Do not free buffer - it will be reused by find_light_bulb callback.
		}
		break;
	default:
		/* Call default signal handler. */
		ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
		break;
	}

	if (bufid) {
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
	//device_ctx.bulb_params.short_addr = 0xFFFF;

	/* Register dimmer switch device context (endpoints). */
	ZB_AF_REGISTER_DEVICE_CTX(&dimmer_switch_ctx);

	/* If "sleepy button" is defined, check its state during Zigbee
	 * initialization and enable sleepy behavior at device if defined button
	 * is pressed. Additionally, power off unused sections of RAM to lower
	 * device power consumption.
	 */
#if defined BUTTON_SLEEPY
	if (dk_get_buttons() & BUTTON_SLEEPY) {
		zigbee_configure_sleepy_behavior(true);
		zigbee_power_down_unused_ram();
	}
#endif

	/* Start Zigbee default thread */
	zigbee_enable();

	LOG_INF("ZBOSS Light Switch example started");

	while (1) {
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(RUN_LED_BLINK_INTERVAL);
	}
}
