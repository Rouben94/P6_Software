
#include "sdk_config.h"
#include "zb_error_handler.h"
#include "zb_ha_dimmable_light.h"
#include "zb_mem_config_max.h"
#include "zb_nrf52_internal.h"
#include "zboss_api.h"
#include "zboss_api_addons.h"
#include "zigbee_helpers.h"

#include "app_pwm.h"
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

/* Main application customizable context. Stores all settings and static values. */

APP_PWM_INSTANCE(BULB_PWM_NAME, BULB_PWM_TIMER);

static bulb_device_ctx_t m_dev_ctx;

/* Declare attribute list for Basic cluster. */
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT(basic_attr_list,
    &m_dev_ctx.basic_attr.zcl_version,
    &m_dev_ctx.basic_attr.app_version,
    &m_dev_ctx.basic_attr.stack_version,
    &m_dev_ctx.basic_attr.hw_version,
    m_dev_ctx.basic_attr.mf_name,
    m_dev_ctx.basic_attr.model_id,
    m_dev_ctx.basic_attr.date_code,
    &m_dev_ctx.basic_attr.power_source,
    m_dev_ctx.basic_attr.location_id,
    &m_dev_ctx.basic_attr.ph_env,
    m_dev_ctx.basic_attr.sw_ver);

/* Declare attribute list for Identify cluster. */
ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(identify_attr_list, &m_dev_ctx.identify_attr.identify_time);

/* Declare attribute list for Scenes cluster. */
ZB_ZCL_DECLARE_SCENES_ATTRIB_LIST(scenes_attr_list,
    &m_dev_ctx.scenes_attr.scene_count,
    &m_dev_ctx.scenes_attr.current_scene,
    &m_dev_ctx.scenes_attr.current_group,
    &m_dev_ctx.scenes_attr.scene_valid,
    &m_dev_ctx.scenes_attr.name_support);

/* Declare attribute list for Groups cluster. */
ZB_ZCL_DECLARE_GROUPS_ATTRIB_LIST(groups_attr_list, &m_dev_ctx.groups_attr.name_support);

/* On/Off cluster attributes additions data */
ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST_EXT(on_off_attr_list,
    &m_dev_ctx.on_off_attr.on_off,
    &m_dev_ctx.on_off_attr.global_scene_ctrl,
    &m_dev_ctx.on_off_attr.on_time,
    &m_dev_ctx.on_off_attr.off_wait_time);

ZB_ZCL_DECLARE_LEVEL_CONTROL_ATTRIB_LIST(level_control_attr_list,
    &m_dev_ctx.level_control_attr.current_level,
    &m_dev_ctx.level_control_attr.remaining_time);

/* Declare cluster list for Dimmable Light device (Identify, Basic, Scenes, Groups, On Off, Level Control). */
ZB_HA_DECLARE_DIMMABLE_LIGHT_CLUSTER_LIST(dimmable_light_clusters,
    basic_attr_list,
    identify_attr_list,
    groups_attr_list,
    scenes_attr_list,
    on_off_attr_list,
    level_control_attr_list);

/* Declare endpoint for Dimmable Light device. */
ZB_HA_DECLARE_DIMMABLE_LIGHT_EP(
    dimmable_light_ep,
    BENCHMARK_SERVER_ENDPOINT,
    dimmable_light_clusters);

/* Declare application's device context (list of registered endpoints)
 * for Benchmark Server device.*/
ZBOSS_DECLARE_DEVICE_CTX_1_EP(bm_server_ctx, dimmable_light_ep);

/************************************ Forward Declarations ***********************************************/

char local_nodel_ieee_addr_buf[17] = {0};
int local_node_addr_len;
zb_ieee_addr_t local_node_ieee_addr;
zb_uint16_t local_node_short_addr;

static void buttons_handler(bsp_event_t evt);
void bm_receive_message(zb_uint8_t param);
void bm_read_message_info(zb_uint16_t timeout);

/************************************ Benchmark Client Cluster Attribute Init ***********************************************/

/**@brief Function for initializing all clusters attributes.
 */
static void bm_server_clusters_attr_init(void) {
  /* Basic cluster attributes data */
  m_dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
  m_dev_ctx.basic_attr.app_version = BULB_INIT_BASIC_APP_VERSION;
  m_dev_ctx.basic_attr.stack_version = BULB_INIT_BASIC_STACK_VERSION;
  m_dev_ctx.basic_attr.hw_version = BULB_INIT_BASIC_HW_VERSION;

  /* Use ZB_ZCL_SET_STRING_VAL to set strings, because the first byte should
     * contain string length without trailing zero.
     *
     * For example "test" string wil be encoded as:
     *   [(0x4), 't', 'e', 's', 't']
     */
  ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.mf_name,
      BULB_INIT_BASIC_MANUF_NAME,
      ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_MANUF_NAME));

  ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.model_id,
      BULB_INIT_BASIC_MODEL_ID,
      ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_MODEL_ID));

  ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.date_code,
      BULB_INIT_BASIC_DATE_CODE,
      ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_DATE_CODE));

  m_dev_ctx.basic_attr.power_source = BULB_INIT_BASIC_POWER_SOURCE;

  ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.location_id,
      BULB_INIT_BASIC_LOCATION_DESC,
      ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_LOCATION_DESC));

  m_dev_ctx.basic_attr.ph_env = BULB_INIT_BASIC_PH_ENV;

  /* Identify cluster attributes data */
  m_dev_ctx.identify_attr.identify_time = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;

  /* On/Off cluster attributes data */
  m_dev_ctx.on_off_attr.on_off = (zb_bool_t)ZB_ZCL_ON_OFF_IS_ON;

  m_dev_ctx.level_control_attr.current_level = ZB_ZCL_LEVEL_CONTROL_LEVEL_MAX_VALUE;
  m_dev_ctx.level_control_attr.remaining_time = ZB_ZCL_LEVEL_CONTROL_REMAINING_TIME_DEFAULT_VALUE;

  ZB_ZCL_SET_ATTRIBUTE(BENCHMARK_SERVER_ENDPOINT,
      ZB_ZCL_CLUSTER_ID_ON_OFF,
      ZB_ZCL_CLUSTER_SERVER_ROLE,
      ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
      (zb_uint8_t *)&m_dev_ctx.on_off_attr.on_off,
      ZB_FALSE);

  ZB_ZCL_SET_ATTRIBUTE(BENCHMARK_SERVER_ENDPOINT,
      ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
      ZB_ZCL_CLUSTER_SERVER_ROLE,
      ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID,
      (zb_uint8_t *)&m_dev_ctx.level_control_attr.current_level,
      ZB_FALSE);
}

/************************************ General Init Functions ***********************************************/

/**@brief Function for initializing the application timer.
 */
static void timer_init(void) {
  uint32_t error_code = app_timer_init();
  APP_ERROR_CHECK(error_code);
}

/**@brief Function for initializing LEDs and a single PWM channel.
 */
static void leds_pwm_init(void) {
  ret_code_t err_code;
  app_pwm_config_t pwm_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000L, bsp_board_led_idx_to_pin(BULB_LED));

  /* Initialize PWM running on timer 1 in order to control dimmable light bulb. */
  err_code = app_pwm_init(&BULB_PWM_NAME, &pwm_cfg, NULL);
  APP_ERROR_CHECK(err_code);

  app_pwm_enable(&BULB_PWM_NAME);

  while (app_pwm_channel_duty_set(&BULB_PWM_NAME, 0, 99) == NRF_ERROR_BUSY) {
  }
}

/************************************ Light Bulb Functions ***********************************************/

/**@brief Sets brightness of on-board LED
 *
 * @param[in] brightness_level Brightness level, allowed values 0 ... 255, 0 - turn off, 255 - full brightness
 */
static void light_bulb_onboard_set_brightness(zb_uint8_t brightness_level) {
  app_pwm_duty_t app_pwm_duty;

  /* Scale level value: APP_PWM uses 0-100 scale, but Zigbee level control cluster uses values from 0 up to 255. */
  app_pwm_duty = (brightness_level * 100U) / 255U;

  /* Set the duty cycle - keep trying until PWM is ready. */
  while (app_pwm_channel_duty_set(&BULB_PWM_NAME, 0, app_pwm_duty) == NRF_ERROR_BUSY) {
  }
}

/**@brief Sets brightness of bulb luminous executive element
 *
 * @param[in] brightness_level Brightness level, allowed values 0 ... 255, 0 - turn off, 255 - full brightness
 */
static void light_bulb_set_brightness(zb_uint8_t brightness_level) {
  light_bulb_onboard_set_brightness(brightness_level);
}

/**@brief Function for setting the light bulb brightness.
  *
  * @param[in]   new_level   Light bulb brightness value.
 */
static void level_control_set_value(zb_uint16_t new_level) {
  bm_cli_log("Set level value: %i\n", new_level);

  ZB_ZCL_SET_ATTRIBUTE(BENCHMARK_SERVER_ENDPOINT,
      ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
      ZB_ZCL_CLUSTER_SERVER_ROLE,
      ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID,
      (zb_uint8_t *)&new_level,
      ZB_FALSE);

  /* According to the table 7.3 of Home Automation Profile Specification v 1.2 rev 29, chapter 7.1.3. */
  if (new_level == 0) {
    zb_uint8_t value = ZB_FALSE;
    ZB_ZCL_SET_ATTRIBUTE(BENCHMARK_SERVER_ENDPOINT,
        ZB_ZCL_CLUSTER_ID_ON_OFF,
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
        &value,
        ZB_FALSE);
  } else {
    zb_uint8_t value = ZB_TRUE;
    ZB_ZCL_SET_ATTRIBUTE(BENCHMARK_SERVER_ENDPOINT,
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
static void on_off_set_value(zb_bool_t on) {
  bm_cli_log("Set ON/OFF value: %i\n", on);

  ZB_ZCL_SET_ATTRIBUTE(BENCHMARK_SERVER_ENDPOINT,
      ZB_ZCL_CLUSTER_ID_ON_OFF,
      ZB_ZCL_CLUSTER_SERVER_ROLE,
      ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
      (zb_uint8_t *)&on,
      ZB_FALSE);

  if (on) {
    level_control_set_value(m_dev_ctx.level_control_attr.current_level);
  } else {
    light_bulb_set_brightness(0U);
  }
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
static void buttons_handler(bsp_event_t evt) {
  zb_ret_t zb_err_code;
  zb_uint32_t button;

  /* Inform default signal handler about user input at the device. */
  switch (evt) {
  case BSP_EVENT_KEY_0:
    button = DONGLE_BUTTON_ON;
    bm_cli_log("BUTTON pressed\n");
    break;

  default:
    bm_cli_log("Unhandled BSP Event received: %d\n", evt);
    break;
  }
  if (!m_device_ctx.button.in_progress) {
    m_device_ctx.button.in_progress = ZB_TRUE;
    m_device_ctx.button.timestamp = ZB_TIMER_GET();

    zb_err_code = ZB_SCHEDULE_APP_ALARM(bm_button_handler, button, LIGHT_SWITCH_BUTTON_SHORT_POLL_TMO);
    if (zb_err_code == RET_OVERFLOW) {
      NRF_LOG_WARNING("Can not schedule another alarm, queue is full.\n");
      m_device_ctx.button.in_progress = ZB_FALSE;
    } else {
      ZB_ERROR_CHECK(zb_err_code);
    }
  }
}

/************************************ Benchmark Functions ***********************************************/

/**@brief Function for sending add group request to the local node.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer used to pass received data.
 */
static zb_void_t add_group_id(zb_bufid_t bufid) {
  zb_uint16_t groupID = bm_params.GroupAddress + GROUP_ID;

  zb_get_long_address(local_node_ieee_addr);
  local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
  local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);

  bm_cli_log("Include device 0x%x, ep %d to the group 0x%x\n", local_node_short_addr, BENCHMARK_SERVER_ENDPOINT, groupID);

  ZB_ZCL_GROUPS_SEND_ADD_GROUP_REQ(bufid,
      local_node_short_addr,
      ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
      BENCHMARK_SERVER_ENDPOINT,
      BENCHMARK_CLIENT_ENDPOINT,
      ZB_AF_HA_PROFILE_ID,
      ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
      NULL,
      groupID);
}

/**@brief Function for receiving benchmark message.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer used to pass received data.
 */
void bm_receive_message(zb_bufid_t bufid) {
  bm_message_info message;
  zb_uint8_t lqi = ZB_MAC_LQI_UNDEFINED;
  zb_uint8_t rssi = ZB_MAC_RSSI_UNDEFINED;
  zb_uint8_t addr_type;
  zb_ieee_addr_t ieee_dst_addr;

  zb_zcl_parsed_hdr_t cmd_info;
  ZB_ZCL_COPY_PARSED_HEADER(bufid, &cmd_info);

  message.net_time = synctimer_getSyncTime();
  message.ack_net_time = 0;

  message.message_id = cmd_info.seq_number;
  memcpy(&message.src_addr, &(cmd_info.addr_data.common_data.source.u.short_addr), sizeof(zb_uint16_t));

  zb_get_long_address(ieee_dst_addr);
  message.dst_addr = zb_address_short_by_ieee(ieee_dst_addr);
  message.group_addr = GROUP_ID;

  /* TODO: Number of hops is not available yet */
  message.number_of_hops = 0;
  message.data_size = 0;

  zb_zdo_get_diag_data(message.src_addr, &lqi, &rssi);
  message.rssi = rssi;

  bm_cli_log("Benchmark Packet received with ID: %d from Src Address: 0x%x to Destination 0x%x with RSSI: %d, LQI: %d, Time: %llu\n", message.message_id, message.src_addr, message.dst_addr, rssi, lqi, message.net_time);

  bm_log_append_ram(message);
}

/************************************ Zigbee event handler ***********************************************/

/* TODO: Separate ZCL Handler for each endpoint */

/**@brief Callback function for handling custom ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
static zb_uint8_t bm_zcl_handler(zb_bufid_t bufid) {
  zb_zcl_parsed_hdr_t cmd_info;

  ZB_ZCL_COPY_PARSED_HEADER(bufid, &cmd_info);
  bm_cli_log("%s with Endpoint ID: %hd, Cluster ID: %d\n", __func__, cmd_info.addr_data.common_data.dst_endpoint, cmd_info.cluster_id);

  if (cmd_info.cluster_id == ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) {

    switch (cmd_info.addr_data.common_data.dst_endpoint) {

    case BENCHMARK_SERVER_ENDPOINT:
      ZB_SCHEDULE_APP_CALLBACK(bm_receive_message, bufid);
      break;

    default:
      break;
    }
  }
  return ZB_FALSE;
}

/**@brief Callback function for handling ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer used to pass received data.
 */
static zb_void_t zcl_device_cb(zb_bufid_t bufid) {
  zb_uint8_t cluster_id;
  zb_uint8_t attr_id;
  zb_zcl_device_callback_param_t *p_device_cb_param = ZB_BUF_GET_PARAM(bufid, zb_zcl_device_callback_param_t);

  bm_cli_log("zcl_device_cb id %hd\n", p_device_cb_param->device_cb_id);

  /* Set default response value. */
  p_device_cb_param->status = RET_OK;

  switch (p_device_cb_param->device_cb_id) {
  case ZB_ZCL_LEVEL_CONTROL_SET_VALUE_CB_ID:
    bm_cli_log("Level control setting to %d\n", p_device_cb_param->cb_param.level_control_set_value_param.new_value);
    level_control_set_value(p_device_cb_param->cb_param.level_control_set_value_param.new_value);
    break;

  case ZB_ZCL_SET_ATTR_VALUE_CB_ID:
    cluster_id = p_device_cb_param->cb_param.set_attr_value_param.cluster_id;
    attr_id = p_device_cb_param->cb_param.set_attr_value_param.attr_id;

    if (cluster_id == ZB_ZCL_CLUSTER_ID_ON_OFF) {
      uint8_t value = p_device_cb_param->cb_param.set_attr_value_param.values.data8;

      bm_cli_log("on/off attribute setting to %hd\n", value);
      if (attr_id == ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) {
        on_off_set_value((zb_bool_t)value);
      }
    } else if (cluster_id == ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) {
      uint16_t value = p_device_cb_param->cb_param.set_attr_value_param.values.data16;

      bm_cli_log("level control attribute setting to %hd\n", value);
      if (attr_id == ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID) {
        level_control_set_value(value);
      }
    } else {
      /* Other clusters can be processed here */
      bm_cli_log("Unhandled cluster attribute id: %d\n", cluster_id);
    }
    break;

  default:
    p_device_cb_param->status = RET_ERROR;
    break;
  }
}

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid) {

  zb_zdo_app_signal_hdr_t *sig_hndler = NULL;
  zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &sig_hndler);
  zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);
  zb_ret_t zb_err_code;

  bm_cli_log("Signal Received, %d\n", sig);

  /* Update network status LED */
  zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

  switch (sig) {
  case ZB_BDB_SIGNAL_STEERING:
    bm_cli_log("Zigbee Network Steering\n");
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid)); /* Call default signal handler. */
    if (status == RET_OK) {
      /* Schedule Add Group ID request */
      zb_err_code = ZB_SCHEDULE_APP_ALARM(add_group_id, bufid, 2 * ZB_TIME_ONE_SECOND);
      ZB_ERROR_CHECK(zb_err_code);

      /* Read local node address */
      //      zb_get_long_address(local_node_ieee_addr);
      //      local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
      //      local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);
      //      bm_cli_log("Network Steering finished with Local Node Address: Short: 0x%x, IEEE/Long: 0x%s", local_node_short_addr, local_nodel_ieee_addr_buf);
      bufid = 0; // Do not free buffer - it will be reused by find_light_bulb callback.
    }
    //  case ZB_BDB_SIGNAL_DEVICE_REBOOT:
    //    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid)); /* Call default signal handler. */
    //    if (status == RET_OK) {
    //      /* Read local node address */
    //      zb_get_long_address(local_node_ieee_addr);
    //      local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
    //      local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);
    //      bm_cli_log("Node restarted with Local Node Address: Short: 0x%x, IEEE/Long: 0x%s", local_node_short_addr, local_nodel_ieee_addr_buf);
    //    }
    break;
  default:
    /* No application-specific behavior is required. Call default signal handler. */
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
    break;
  }

  if (bufid) {
    zb_buf_free(bufid);
  }
}

void bm_zigbee_init(void) {
  zb_ieee_addr_t ieee_addr;

  /* Initialize timer, logging system and GPIOs. */
  timer_init();
  leds_pwm_init();

  /* Set Zigbee stack logging level and traffic dump subsystem. */
  ZB_SET_TRACE_LEVEL(ZIGBEE_TRACE_LEVEL);
  ZB_SET_TRACE_MASK(ZIGBEE_TRACE_MASK);
  ZB_SET_TRAF_DUMP_OFF();

  /* Initialize Zigbee stack. */
  ZB_INIT("led_bulb");

  /* Set device address to the value read from FICR registers. */
  zb_osif_get_ieee_eui64(ieee_addr);
  zb_set_long_address(ieee_addr);

  /* Set static long IEEE address. */
  zb_set_network_router_role(IEEE_CHANNEL_MASK);
  zb_set_max_children(MAX_CHILDREN);
  zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);
  zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(3000));

  /* Initialize application context structure. */
  UNUSED_RETURN_VALUE(ZB_MEMSET(&m_dev_ctx, 0, sizeof(m_dev_ctx)));

  /* Register dimmer switch device context (endpoints). */
  ZB_AF_REGISTER_DEVICE_CTX(&bm_server_ctx);

  /* Register callback for handling ZCL commands. */
  ZB_AF_SET_ENDPOINT_HANDLER(BENCHMARK_SERVER_ENDPOINT, bm_zcl_handler);
  ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

  bm_server_clusters_attr_init();
  level_control_set_value(m_dev_ctx.level_control_attr.current_level);
}

void bm_zigbee_enable(void) {
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;

  /** Start Zigbee Stack. */
  zb_err_code = zboss_start_no_autostart();
  ZB_ERROR_CHECK(zb_err_code);
}