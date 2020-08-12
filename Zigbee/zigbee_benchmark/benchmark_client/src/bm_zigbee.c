/*
This file is part of Zigbee-Benchmark.

Zigbee-Benchmark is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Zigbee-Benchmark is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Zigbee-Benchmark. If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR 	   :    Cyrill Horath      */

#include "nrf_802154.h"
#include "sdk_config.h"
#include "zb_error_handler.h"
#include "zboss_api.h"
#include "zboss_api_addons.h"
#include "zigbee_helpers.h"

#include "bm_mem_config_custom.h"

#include "app_timer.h"
#include "boards.h"
#include "bsp.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bm_cli.h"
#include "bm_config.h"
#include "bm_log.h"
#include "bm_rand.h"
#include "bm_simple_buttons_and_leds.h"
#include "bm_timesync.h"
#include "bm_zigbee.h"

static light_switch_ctx_t m_device_ctx;
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

/* Declare cluster list for Dimmer Switch device (Identify, Basic, Scenes, Groups, On Off, Level Control). */
/* Only clusters Identify and Basic have attributes. */
ZB_HA_DECLARE_DIMMER_SWITCH_CLUSTER_LIST(dimmer_switch_clusters,
    basic_attr_list,
    identify_attr_list);

/* Declare endpoint for Dimmer Switch device. */
ZB_HA_DECLARE_DIMMER_SWITCH_EP(dimmer_switch_ep,
    BENCHMARK_CLIENT_ENDPOINT,
    dimmer_switch_clusters);

/* Declare application's device context (list of registered endpoints) for Dimmer Switch device. */
ZBOSS_DECLARE_DEVICE_CTX_1_EP(bm_client_ctx, dimmer_switch_ep);

/************************************ Forward Declarations ***********************************************/

void buttons_handler(bsp_event_t evt);
void bm_send_message(void);
void bm_read_message_info(zb_uint16_t dst_addr_short, zb_uint16_t tsn);

zb_uint16_t seq_num = 0;
zb_uint16_t msg_sent_cnt = 0;
zb_uint16_t rem_dev_id;

zb_ieee_addr_t local_node_ieee_addr;
zb_uint16_t local_node_short_addr;
char local_nodel_ieee_addr_buf[17] = {0};
int local_node_addr_len;

zb_uint32_t stack_enable_max_delay_ms = STACK_STARTUP_MAX_DELAY;
zb_uint32_t network_formation_delay = NETWORK_FORMATION_DELAY;

static const zb_uint8_t g_key_nwk[16] = {0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0, 0, 0, 0, 0, 0, 0, 0};

/************************************ Benchmark Client Cluster Attribute Init ***********************************************/

/**@brief Function for initializing all clusters attributes.
 */
static void bm_client_clusters_attr_init(void) {
  /* Basic cluster attributes data */
  dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
  dev_ctx.basic_attr.app_version = BENCHMARK_INIT_BASIC_APP_VERSION;
  dev_ctx.basic_attr.stack_version = BENCHMARK_INIT_BASIC_STACK_VERSION;
  dev_ctx.basic_attr.hw_version = BENCHMARK_INIT_BASIC_HW_VERSION;

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
}

/************************************ General Init Functions ***********************************************/

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void) {
  ret_code_t err_code;

  // Initialize timer module.
  err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}

/* Get device address of the local device.*/
void bm_get_ieee_eui64(zb_ieee_addr_t ieee_eui64) {
  uint64_t factoryAddress;
  factoryAddress = NRF_FICR->DEVICEADDR[0];
  memcpy(ieee_eui64, &factoryAddress, sizeof(factoryAddress));
}

/************************************ Button Handler Functions ***********************************************/

/**@brief Callback for detecting button press duration.
 *
 * @param[in]   button   BSP Button that was pressed.
 */
zb_void_t bm_button_handler(zb_uint8_t button) {
  zb_time_t current_time;
  zb_bool_t short_expired;
  zb_bool_t on_off;
  zb_ret_t zb_err_code;
  zb_uint8_t random_level_value;

  bm_cli_log("Button pressed: %d\n", button);

  current_time = ZB_TIMER_GET();

  switch (button) {

  case DONGLE_BUTTON:

    break;

  default:
    bm_cli_log("Unhandled BSP Event received: %d\n", button);
    return;
  }

  m_device_ctx.button.in_progress = ZB_FALSE;
}

/**@brief Callback for button events.
 *
 * @param[in]   evt      Incoming event from the BSP subsystem.
 */
void buttons_handler(bsp_event_t evt) {
  zb_ret_t zb_err_code;
  zb_uint32_t button;

  switch (evt) {
  case BSP_EVENT_KEY_0:
    button = DONGLE_BUTTON_ON;

    break;

  default:
    bm_cli_log("Unhandled BSP Event received: %d\n", evt);
    return;
  }

  if (!m_device_ctx.button.in_progress) {
    m_device_ctx.button.in_progress = ZB_TRUE;
    m_device_ctx.button.timestamp = ZB_TIMER_GET();

    zb_err_code = ZB_SCHEDULE_APP_ALARM(bm_button_handler, button, LIGHT_SWITCH_BUTTON_SHORT_POLL_TMO);
    if (zb_err_code == RET_OVERFLOW) {
      NRF_LOG_WARNING("Can not schedule another alarm, queue is full.");
      m_device_ctx.button.in_progress = ZB_FALSE;
    } else {
      ZB_ERROR_CHECK(zb_err_code);
    }
  }
}

/************************************ Benchmark Functions ***********************************************/

/* Callback function for Benchmark send message status. Free's the used buffer. */
void bm_send_message_status_cb(zb_bufid_t bufid) {
  zb_zcl_command_send_status_t *send_cmd_status = ZB_BUF_GET_PARAM(bufid, zb_zcl_command_send_status_t);

  msg_sent_cnt++;
  bm_cli_log("Message successfully sent: %d, Status Code: %d\n", msg_sent_cnt, send_cmd_status->status);

  if (bufid) {
    zb_buf_free(bufid);
  }
}

/* Function to send Benchmark Message to group destination. */
void bm_send_group_message_cb(zb_bufid_t bufid, zb_uint16_t seq_num) {
  zb_uint16_t groupID = bm_params.GroupAddress + GROUP_ID;
  bm_cli_log("Benchmark send message cb group: 0x%x, Seq Num: %d\n", groupID, seq_num);

  /* Send Move to level request. Added manufacturer specific field in ZCL Header to transmit 16 Bit Benchmark sequence number. */
  zb_uint8_t *cmd_ptr = ZB_ZCL_START_PACKET(bufid);
  ZB_ZCL_CONSTRUCT_GENERAL_COMMAND_REQ_FRAME_CONTROL_A(cmd_ptr, (ZB_ZCL_FRAME_DIRECTION_TO_SRV), (ZB_ZCL_MANUFACTURER_SPECIFIC), ZB_ZCL_DISABLE_DEFAULT_RESPONSE);
  ZB_ZCL_CONSTRUCT_COMMAND_HEADER_EXT(cmd_ptr, ZB_ZCL_GET_SEQ_NUM(), (ZB_TRUE), seq_num, (bm_params.AdditionalPayloadSize));
  /* Minimal Payload size defined as 2 Bytes. */
  ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr, (DUMMY_PAYLOAD));
  ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr, (DUMMY_PAYLOAD));
  /* Additional Payload added as 8 Bit Dummy Payload Packets. */
  for (zb_uint8_t i = 0; i < bm_params.AdditionalPayloadSize; i++) {
    ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr, (DUMMY_PAYLOAD));
  }
  ZB_ZCL_FINISH_PACKET(bufid, cmd_ptr)
  ZB_ZCL_SEND_COMMAND_SHORT(bufid, groupID, ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT, BENCHMARK_SERVER_ENDPOINT, BENCHMARK_CLIENT_ENDPOINT, ZB_AF_HA_PROFILE_ID, ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, bm_send_message_status_cb);
}

/* Callback function to send direct Benchmark Message.*/
void bm_send_dir_message_cb(zb_bufid_t bufid, zb_uint16_t dst_addr_short, zb_uint16_t seq_num) {
  bm_cli_log("Benchmark send message cb dst: 0x%x, Bufid: %d, Seq Num: %d\n", dst_addr_short, bufid, seq_num);

  /* Send Move to level request. Added manufacturer specific field in ZCL Header to transmit 16 Bit Benchmark sequence number. */
  zb_uint8_t *cmd_ptr = ZB_ZCL_START_PACKET(bufid);
  ZB_ZCL_CONSTRUCT_GENERAL_COMMAND_REQ_FRAME_CONTROL_A(cmd_ptr, (ZB_ZCL_FRAME_DIRECTION_TO_SRV), (ZB_ZCL_MANUFACTURER_SPECIFIC), ZB_ZCL_DISABLE_DEFAULT_RESPONSE);
  ZB_ZCL_CONSTRUCT_COMMAND_HEADER_EXT(cmd_ptr, ZB_ZCL_GET_SEQ_NUM(), (ZB_TRUE), seq_num, (ZB_ZCL_CMD_LEVEL_CONTROL_MOVE_TO_LEVEL));
  /* Minimal Payload size defined as 2 Bytes. */
  ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr, (DUMMY_PAYLOAD));
  ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr, (DUMMY_PAYLOAD));
  /* Additional Payload added as 8 Bit Dummy Payload Packets. */
  for (zb_uint8_t i = 0; i < bm_params.AdditionalPayloadSize; i++) {
    ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr, (DUMMY_PAYLOAD));
  }
  ZB_ZCL_FINISH_PACKET(bufid, cmd_ptr)
  ZB_ZCL_SEND_COMMAND_SHORT(bufid, dst_addr_short, ZB_APS_ADDR_MODE_16_ENDP_PRESENT, BENCHMARK_SERVER_ENDPOINT, BENCHMARK_CLIENT_ENDPOINT, ZB_AF_HA_PROFILE_ID, ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, bm_send_message_status_cb);
}

/* Benchmark send message function. Function will be called from BENCHMARK state in bm_statemachine.c */
void bm_send_message(void) {
  zb_ret_t zb_err_code;
  zb_bufid_t bufid;
  zb_uint16_t dst_addr_short;
  uint64_t dst_addr_conf[3] = {bm_params.DestMAC_1, bm_params.DestMAC_2, bm_params.DestMAC_3};
  zb_ieee_addr_t dst_ieee_addr;
  char ieee_addr_buf[17] = {0};
  int addr_len;
  seq_num++;

  if ((dst_addr_conf[0] == 0) && (dst_addr_conf[1] == 0) && (dst_addr_conf[2] == 0)) {
    bm_cli_log("Benchmark send message to group address: 0x%x\n", bm_params.GroupAddress + GROUP_ID);
    bm_read_message_info(dst_addr_short, seq_num);
    zb_err_code = zb_buf_get_out_delayed_ext(bm_send_group_message_cb, seq_num, 0);
    ZB_ERROR_CHECK(zb_err_code);

  } else {
    for (uint8_t i = 0; i < 3; i++) {
      memcpy(dst_ieee_addr, &dst_addr_conf[i], sizeof(dst_addr_conf[i]));
      if (dst_addr_conf[i]) {
        dst_addr_short = zb_address_short_by_ieee(dst_ieee_addr);
        if (dst_addr_short != 0xFFFF) {
          addr_len = ieee_addr_to_str(ieee_addr_buf, sizeof(ieee_addr_buf), dst_ieee_addr);
          bm_cli_log("Benchmark send direct message to address: 0x%x, Index: %d\n", dst_addr_short, i);
          bm_read_message_info(dst_addr_short, seq_num);
          bufid = zb_buf_get_out();
          bm_send_dir_message_cb(bufid, dst_addr_short, seq_num);
        } else {
          bm_cli_log("Error read short address of destination\n");
        }
      }
    }
  }
}

/* Benchmark read message info function. Reads Benchmark data from message.*/
void bm_read_message_info(zb_uint16_t dst_addr_short, zb_uint16_t tsn) {
  bm_message_info message;
  zb_ieee_addr_t ieee_src_addr;
  zb_bool_t led_toggle;

  message.number_of_hops = 0;
  message.data_size = bm_params.AdditionalPayloadSize;
  message.rssi = 0;
  message.ack_net_time = 0;

  zb_get_long_address(ieee_src_addr);
  message.src_addr = zb_address_short_by_ieee(ieee_src_addr);
  message.group_addr = bm_params.GroupAddress + GROUP_ID;

  if (dst_addr_short == 0) {
    message.dst_addr = message.group_addr;
  } else {
    message.dst_addr = dst_addr_short;
  }
  message.message_id = tsn;
  message.net_time = synctimer_getSyncTime();

  bm_cli_log("Benchmark read message data: Destination Address: 0x%x TimeStamp: %lld, MessageID: %d (%d)\n", message.dst_addr, message.net_time, message.message_id, seq_num);
  bm_log_append_ram(message);

  /* Toggle blue LED to indicate  */
  led_toggle = (zb_bool_t)seq_num % 2;
  if (led_toggle) {
    bm_led2_set(true);
  } else {
    bm_led2_set(false);
  }
}

/************************************ Zigbee event handler ***********************************************/

/**@brief Callback function for handling ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
static zb_void_t zcl_device_cb(zb_bufid_t bufid) {
  zb_zcl_device_callback_param_t *device_cb_param = ZB_BUF_GET_PARAM(bufid, zb_zcl_device_callback_param_t);
  bm_cli_log("%s id %hd\n", __func__, device_cb_param->device_cb_id);

  /* Set default response value. */
  device_cb_param->status = RET_OK;
}

/************************************ ZBOSS signal handler ***********************************************/
/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid) {
  zb_zdo_app_signal_hdr_t *p_sg_p = NULL;
  zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &p_sg_p);
  zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);
  zb_ret_t zb_err_code;
  zb_uint32_t active_channel_msk;
  uint8_t active_channel;

  /* Update network status LED */
  zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

  switch (sig) {
  case ZB_BDB_SIGNAL_STEERING:
    /* Call default signal handler. */
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
    bm_cli_log("Zigbee Network Steering\n");
    bm_cli_log("Active channel %d\n", nrf_802154_channel_get());

    if (status == RET_OK) {
      /* Read local node address */
      zb_get_long_address(local_node_ieee_addr);
      local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
      local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);
      bm_cli_log("Network Steering finished with Local Node Address: Short: 0x%x, IEEE/Long: 0x%s\n", local_node_short_addr, local_nodel_ieee_addr_buf);
    }
    break;
  case ZB_ZDO_SIGNAL_DEVICE_ANNCE:
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid)); /* Call default signal handler. */

    zb_zdo_signal_device_annce_params_t *dev_annce_params = ZB_ZDO_SIGNAL_GET_PARAMS(p_sg_p, zb_zdo_signal_device_annce_params_t);
    rem_dev_id = dev_annce_params->device_short_addr;
    bm_cli_log("New device commissioned or rejoined (short: 0x%04hx)\n", dev_annce_params->device_short_addr);
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

/**************************************** Zigbee Stack Init and Enable ***********************************************/

/** Init Zigbee Stack. */
void bm_zigbee_init(void) {
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;
  //  uint64_t ext_pan_id_64 = DEFAULT_PAN_ID_EXT;
  //  zb_ext_pan_id_t ext_pan_id;
  //  memcpy(ext_pan_id, &ext_pan_id_64, sizeof(ext_pan_id_64));

  /* Initialize timers. */
  timers_init();

  /* Set Zigbee stack logging level and traffic dump subsystem. */
  ZB_SET_TRACE_LEVEL(ZIGBEE_TRACE_LEVEL);
  ZB_SET_TRACE_MASK(ZIGBEE_TRACE_MASK);
  ZB_SET_TRAF_DUMP_OFF();

  /* Initialize Zigbee stack. */
  ZB_INIT("Benchmark Client");

  /* Set device address to the value read from FICR registers. */
  bm_get_ieee_eui64(ieee_addr);
  zb_set_long_address(ieee_addr);

  /* Set short and extended pan id to the default value. */
//  zb_set_pan_id((zb_uint16_t)DEFAULT_PAN_ID_SHORT);
//  zb_set_extended_pan_id(ext_pan_id);
//  zb_secur_setup_nwk_key((zb_uint8_t *)g_key_nwk, 0);

  zb_set_network_router_role(IEEE_CHANNEL_MASK);
  zb_set_max_children(MAX_CHILDREN);
  //  zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);
  zigbee_erase_persistent_storage(bm_params.Ack);

  /* Initialize application context structure. */
  UNUSED_RETURN_VALUE(ZB_MEMSET(&m_device_ctx, 0, sizeof(light_switch_ctx_t)));

  /* Register dimmer switch device context (endpoints). */
  ZB_AF_REGISTER_DEVICE_CTX(&bm_client_ctx);

  /* Register callback for handling ZCL commands. */
  ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

  /* Initialzie Cluster Attributes */
  bm_client_clusters_attr_init();
}

/** Start Zigbee Stack. */
void bm_zigbee_enable(void) {
  zb_ret_t zb_err_code;
  zb_uint16_t stack_enable_delay_ms = (bm_rand_32 % stack_enable_max_delay_ms) + network_formation_delay;

  /* Stack Enable Timeout to prevent crash at stack startup in order of too many simultaneous commissioning requests. */
  bm_cli_log("Zigbee Stack starts in: %d milliseconds\n", stack_enable_delay_ms);
  bm_sleep(stack_enable_delay_ms);
  zb_err_code = zboss_start_no_autostart();
  ZB_ERROR_CHECK(zb_err_code);
  bm_led2_set(true);
  bm_cli_log("BENCHMARK Client ready\n");
}