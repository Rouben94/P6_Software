
#include "sdk_config.h"
#include "zb_error_handler.h"
#include "zb_mem_config_max.h"
#include "zboss_api.h"
#include "zboss_api_addons.h"
#include "zigbee_helpers.h"

#include "app_timer.h"
#include "boards.h"
#include "bsp.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bm_cli.h"
#include "bm_config.h"
#include "bm_log.h"
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

/* Declare attribute list for Scenes cluster. */
ZB_ZCL_DECLARE_SCENES_ATTRIB_LIST(
    scenes_attr_list,
    &dev_ctx.scenes_attr.scene_count,
    &dev_ctx.scenes_attr.current_scene,
    &dev_ctx.scenes_attr.current_group,
    &dev_ctx.scenes_attr.scene_valid,
    &dev_ctx.scenes_attr.name_support);

/* Declare attribute list for Groups cluster. */
ZB_ZCL_DECLARE_GROUPS_ATTRIB_LIST(
    groups_attr_list,
    &dev_ctx.groups_attr.name_support);

/* On/Off cluster attributes additions data */
ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST(
    on_off_attr_list,
    &dev_ctx.on_off_attr.on_off);

ZB_ZCL_DECLARE_LEVEL_CONTROL_ATTRIB_LIST(
    level_control_attr_list,
    &dev_ctx.level_control_attr.current_level,
    &dev_ctx.level_control_attr.remaining_time);

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
static void light_switch_send_on_off(zb_bufid_t bufid, zb_uint16_t on_off);
void bm_send_message_cb(zb_bufid_t bufid, zb_uint16_t level);
void bm_send_control_message_cb(zb_bufid_t bufid, zb_uint16_t level);
void bm_send_message(void);
void bm_read_message_info(zb_uint8_t seq_num);
void bm_report_data(zb_uint8_t param);

zb_uint8_t seq_num = 0;

zb_ieee_addr_t local_node_ieee_addr;
zb_uint16_t local_node_short_addr;
char local_nodel_ieee_addr_buf[17] = {0};
int local_node_addr_len;

bm_tid_overflow_handler_t bm_tid_overflow_handler[max_number_of_nodes]; /* Expect not more than max_number_of_nodes Different Adresses */

/************************************ Benchmark Client Cluster Attribute Init ***********************************************/

/**@brief Function for initializing all clusters attributes.
 */
static void bm_client_clusters_attr_init(void) {
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

  /* Inform default signal handler about user input at the device. */
  //  user_input_indicate();

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

/* Insert the tid and src address to get the merged tid with the tid overlfow cnt -> resulting in a uint16_t */
uint16_t bm_get_overflow_tid_from_overflow_handler(uint8_t tid, uint16_t src_addr) {
  // Get the TID in array
  for (int i = 0; i < max_number_of_nodes; i++) {
    if (bm_tid_overflow_handler[i].src_addr == src_addr) {
      // Check if Overflow happend
      if ((bm_tid_overflow_handler[i].last_TID_seen - tid) > 250) {
        bm_tid_overflow_handler[i].TID_OverflowCnt++;
      }
      // Add the last seen TID
      bm_tid_overflow_handler[i].last_TID_seen = tid;
      return (uint16_t)(bm_tid_overflow_handler[i].TID_OverflowCnt << 8) | (tid & 0xff);
      //return bm_tid_overflow_handler[i].TID_OverflowCnt;
    } else if (bm_tid_overflow_handler[i].src_addr == 0) {
      // Add the Src Adress
      bm_tid_overflow_handler[i].src_addr = src_addr;
      bm_tid_overflow_handler[i].last_TID_seen = tid;
      bm_tid_overflow_handler[i].TID_OverflowCnt = 0;
      return (uint16_t)(bm_tid_overflow_handler[i].TID_OverflowCnt << 8) | (tid & 0xff);
    }
  }
  return 0; // Default return 0
}

/* Function to send Benchmark Message */
void bm_send_message_cb(zb_bufid_t bufid, zb_uint16_t level) {
  zb_uint16_t groupID = bm_params.GroupAddress + GROUP_ID;
  //  bm_cli_log("Benchmark Message Callback send to Group Address: 0x%x, ID: %x\n", groupID, level);

  /* Send Move to level request. Level value is uint8. */
  ZB_ZCL_LEVEL_CONTROL_SEND_MOVE_TO_LEVEL_REQ(bufid,
      groupID,
      ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT,
      BENCHMARK_SERVER_ENDPOINT,
      BENCHMARK_CLIENT_ENDPOINT,
      ZB_AF_HA_PROFILE_ID,
      ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
      NULL,
      level,
      0);
}

void bm_send_message(void) {
  zb_ret_t zb_err_code;
  seq_num++;

  bm_read_message_info(seq_num);
  zb_err_code = zb_buf_get_out_delayed_ext(bm_send_message_cb, seq_num, 0);
  //  zb_err_code = zb_buf_get_out_delayed_ext(bm_send_message_cb, random_level_value, 0);
  ZB_ERROR_CHECK(zb_err_code);
}

/* TODO: Description */
void bm_read_message_info(zb_uint8_t seq_num) {
  bm_message_info message;
  zb_ieee_addr_t ieee_src_addr;
  zb_uint8_t lqi = ZB_MAC_LQI_UNDEFINED;
  zb_int8_t rssi = ZB_MAC_RSSI_UNDEFINED;

  message.number_of_hops = 0;
  message.data_size = 0;

  zb_get_long_address(ieee_src_addr);
  message.src_addr = zb_address_short_by_ieee(ieee_src_addr);
  message.group_addr = bm_params.GroupAddress + GROUP_ID;
  message.dst_addr = message.group_addr;

  message.net_time = synctimer_getSyncTime();
  message.ack_net_time = 0;

  message.message_id = bm_get_overflow_tid_from_overflow_handler(seq_num, message.src_addr);
  //  message.message_id = ZB_ZCL_GET_SEQ_NUM() + 1;

  zb_zdo_get_diag_data(message.src_addr, &lqi, &rssi);
  message.rssi = rssi;

  bm_cli_log("Benchmark Message send to Group Address: 0x%x TimeStamp: %lld, MessageID: %d (%d)\n", message.group_addr, message.net_time, message.message_id, seq_num);

  bm_log_append_ram(message);
}

/************************************ Zigbee event handler ***********************************************/

/**@brief Callback function for handling custom ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
//static zb_uint8_t bm_zcl_handler(zb_bufid_t bufid) {
//  zb_zcl_parsed_hdr_t cmd_info;
//
//  ZB_ZCL_COPY_PARSED_HEADER(bufid, &cmd_info);
//  bm_cli_log("%s with Endpoint ID: %hd, Cluster ID: %d", __func__, cmd_info.addr_data.common_data.dst_endpoint, cmd_info.cluster_id);
//
//  switch (cmd_info.cluster_id) {
//  case ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL:
//    ZB_SCHEDULE_APP_ALARM(bm_receive_config, bufid, 0);
//    break;
//  case ZB_ZCL_CLUSTER_ID_ON_OFF:
//    ZB_SCHEDULE_APP_ALARM(bm_receive_config, bufid, 0);
//    break;
//  default:
//    break;
//  }
//
//  return ZB_FALSE;
//}

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

  switch (device_cb_param->device_cb_id) {
  case ZB_ZCL_LEVEL_CONTROL_SET_VALUE_CB_ID:
    bm_cli_log("Level control setting to %d\n", device_cb_param->cb_param.level_control_set_value_param.new_value);

    break;
  default:
    device_cb_param->status = RET_ERROR;
    break;
  }
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

  /* Update network status LED */
  zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

  switch (sig) {
  case ZB_BDB_SIGNAL_DEVICE_REBOOT:
    /* fall-through */
  case ZB_BDB_SIGNAL_STEERING:
    /* Call default signal handler. */
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
    if (status == RET_OK) {

      /* Read local node address */
      zb_get_long_address(local_node_ieee_addr);
      local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
      local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);
      bm_cli_log("Network Steering finished with Local Node Address: Short: 0x%x, IEEE/Long: 0x%s\n", local_node_short_addr, local_nodel_ieee_addr_buf);
    }
    break;
    //  case ZB_BDB_SIGNAL_DEVICE_REBOOT:
    //    /* Call default signal handler. */
    //    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
    //    if (status == RET_OK) {
    //      /* Read local node address */
    //      zb_get_long_address(local_node_ieee_addr);
    //      local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
    //      local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);
    //      bm_cli_log("Node restarted with Local Node Address: Short: 0x%x, IEEE/Long: 0x%s", local_node_short_addr, local_nodel_ieee_addr_buf);
    //    }
    //    break;
  default:
    /* Call default signal handler. */
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
    break;
  }

  if (bufid) {
    zb_buf_free(bufid);
  }
}

void bm_zigbee_init(void) {
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;

  /* Initialize timers, loging system and GPIOs. */
  timers_init();

  /* Set Zigbee stack logging level and traffic dump subsystem. */
  ZB_SET_TRACE_LEVEL(ZIGBEE_TRACE_LEVEL);
  ZB_SET_TRACE_MASK(ZIGBEE_TRACE_MASK);
  ZB_SET_TRAF_DUMP_OFF();

  /* Initialize Zigbee stack. */
  ZB_INIT("light_switch");

  /* Set device address to the value read from FICR registers. */
  zb_osif_get_ieee_eui64(ieee_addr);
  zb_set_long_address(ieee_addr);

  zb_set_network_router_role(IEEE_CHANNEL_MASK);
  zb_set_max_children(MAX_CHILDREN);
  zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);

  zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(3000));

  /* Initialize application context structure. */
  UNUSED_RETURN_VALUE(ZB_MEMSET(&m_device_ctx, 0, sizeof(light_switch_ctx_t)));

  /* Set default bulb short_addr. */
  m_device_ctx.bulb_params.short_addr = 0xFFFF;

  /* Register dimmer switch device context (endpoints). */
  ZB_AF_REGISTER_DEVICE_CTX(&bm_client_ctx);

  /* Register callback for handling ZCL commands. */
  ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

  /* Initialzie Cluster Attributes */
  bm_client_clusters_attr_init();
}

void bm_zigbee_enable(void) {
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;

  /** Start Zigbee Stack. */
  zb_err_code = zboss_start_no_autostart();
  ZB_ERROR_CHECK(zb_err_code);
}