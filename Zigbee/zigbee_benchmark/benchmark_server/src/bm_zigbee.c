

#include "sdk_config.h"
#include "zb_error_handler.h"
#include "zb_ha_dimmable_light.h"
#include "zb_mem_config_med.h"
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

#include "bm_config.h"
#include "bm_zigbee.h"

/* Declare endpoint for Dimmable Light device with scenes. */
#define ZB_HA_DECLARE_LIGHT_EP(ep_name, ep_id, cluster_list)                      \
  ZB_ZCL_DECLARE_HA_DIMMABLE_LIGHT_SIMPLE_DESC(ep_name, ep_id,                    \
      ZB_HA_DIMMABLE_LIGHT_IN_CLUSTER_NUM, ZB_HA_DIMMABLE_LIGHT_OUT_CLUSTER_NUM); \
  ZBOSS_DEVICE_DECLARE_REPORTING_CTX(reporting_info##device_ctx_name,             \
      ZB_HA_DIMMABLE_LIGHT_REPORT_ATTR_COUNT);                                    \
  ZBOSS_DEVICE_DECLARE_LEVEL_CONTROL_CTX(cvc_alarm_info##device_ctx_name,         \
      ZB_HA_DIMMABLE_LIGHT_CVC_ATTR_COUNT);                                       \
  ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id, ZB_AF_HA_PROFILE_ID,                \
      0,                                                                          \
      NULL,                                                                       \
      ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t),                     \
      cluster_list,                                                               \
      (zb_af_simple_desc_1_1_t *)&simple_desc_##ep_name,                          \
      ZB_HA_DIMMABLE_LIGHT_REPORT_ATTR_COUNT,                                     \
      reporting_info##device_ctx_name,                                            \
      ZB_HA_DIMMABLE_LIGHT_CVC_ATTR_COUNT,                                        \
      cvc_alarm_info##device_ctx_name)

#if !defined ZB_ROUTER_ROLE
#error Define ZB_ROUTER_ROLE to compile light bulb (Router) source code.
#endif

/* Main application customizable context. Stores all settings and static values. */
typedef struct
{
  zb_zcl_basic_attrs_ext_t basic_attr;
  zb_zcl_identify_attrs_t identify_attr;
  zb_zcl_scenes_attrs_t scenes_attr;
  zb_zcl_groups_attrs_t groups_attr;
  zb_zcl_on_off_attrs_ext_t on_off_attr;
  zb_zcl_level_control_attrs_t level_control_attr;
} bulb_device_ctx_t;

APP_PWM_INSTANCE(BULB_PWM_NAME, BULB_PWM_TIMER);
static bulb_device_ctx_t m_dev_ctx;

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(identify_attr_list, &m_dev_ctx.identify_attr.identify_time);

ZB_ZCL_DECLARE_GROUPS_ATTRIB_LIST(groups_attr_list, &m_dev_ctx.groups_attr.name_support);

ZB_ZCL_DECLARE_SCENES_ATTRIB_LIST(scenes_attr_list,
    &m_dev_ctx.scenes_attr.scene_count,
    &m_dev_ctx.scenes_attr.current_scene,
    &m_dev_ctx.scenes_attr.current_group,
    &m_dev_ctx.scenes_attr.scene_valid,
    &m_dev_ctx.scenes_attr.name_support);

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

/* On/Off cluster attributes additions data */
ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST_EXT(on_off_attr_list,
    &m_dev_ctx.on_off_attr.on_off,
    &m_dev_ctx.on_off_attr.global_scene_ctrl,
    &m_dev_ctx.on_off_attr.on_time,
    &m_dev_ctx.on_off_attr.off_wait_time);

ZB_ZCL_DECLARE_LEVEL_CONTROL_ATTRIB_LIST(level_control_attr_list,
    &m_dev_ctx.level_control_attr.current_level,
    &m_dev_ctx.level_control_attr.remaining_time);

ZB_HA_DECLARE_DIMMABLE_LIGHT_CLUSTER_LIST(dimmable_light_clusters,
    basic_attr_list,
    identify_attr_list,
    groups_attr_list,
    scenes_attr_list,
    on_off_attr_list,
    level_control_attr_list);

ZB_HA_DECLARE_LIGHT_EP(dimmable_light_ep,
    HA_DIMMABLE_LIGHT_ENDPOINT,
    dimmable_light_clusters);

ZB_HA_DECLARE_DIMMABLE_LIGHT_CTX(dimmable_light_ctx,
    dimmable_light_ep);

zb_uint16_t bm_message_info_nr = 0;
char local_nodel_ieee_addr_buf[17] = {0};
int local_node_addr_len;
zb_ieee_addr_t local_node_ieee_addr;
zb_uint16_t local_node_short_addr;

void bm_save_message_info(bm_message_info message);
static void bm_receive_message(zb_uint8_t param);

/* Array of structs to save benchmark message info to */
bm_message_info message_info[NUMBER_OF_NETWORK_TIME_ELEMENTS] = {0};

/**@brief Function for initializing the application timer.
 */
static void timer_init(void) {
  uint32_t error_code = app_timer_init();
  APP_ERROR_CHECK(error_code);
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

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
  NRF_LOG_INFO("Set level value: %i", new_level);

  ZB_ZCL_SET_ATTRIBUTE(HA_DIMMABLE_LIGHT_ENDPOINT,
      ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
      ZB_ZCL_CLUSTER_SERVER_ROLE,
      ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID,
      (zb_uint8_t *)&new_level,
      ZB_FALSE);

  /* According to the table 7.3 of Home Automation Profile Specification v 1.2 rev 29, chapter 7.1.3. */
  if (new_level == 0) {
    zb_uint8_t value = ZB_FALSE;
    ZB_ZCL_SET_ATTRIBUTE(HA_DIMMABLE_LIGHT_ENDPOINT,
        ZB_ZCL_CLUSTER_ID_ON_OFF,
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
        &value,
        ZB_FALSE);
  } else {
    zb_uint8_t value = ZB_TRUE;
    ZB_ZCL_SET_ATTRIBUTE(HA_DIMMABLE_LIGHT_ENDPOINT,
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
  NRF_LOG_INFO("Set ON/OFF value: %i", on);

  ZB_ZCL_SET_ATTRIBUTE(HA_DIMMABLE_LIGHT_ENDPOINT,
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

/**@brief Callback for button events.
 *
 * @param[in]   evt      Incoming event from the BSP subsystem.
 */
static void buttons_handler(bsp_event_t evt) {
  zb_ret_t zb_err_code;

  switch (evt) {
  //  case IDENTIFY_MODE_BSP_EVT:
  //    /* Check if endpoint is in identifying mode, if not put desired endpoint in identifying mode. */
  //    if (m_dev_ctx.identify_attr.identify_time == ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE) {
  //      NRF_LOG_INFO("Bulb put in identifying mode");
  //      zb_err_code = zb_bdb_finding_binding_target(HA_DIMMABLE_LIGHT_ENDPOINT);
  //      ZB_ERROR_CHECK(zb_err_code);
  //    } else {
  //      NRF_LOG_INFO("Cancel F&B target procedure");
  //      zb_bdb_finding_binding_target_cancel();
  //    }
  //    break;
  case DONGLE_BUTTON_BSP_EVT:
    NRF_LOG_INFO("BUTTON pressed")

    break;

  default:
    NRF_LOG_INFO("Unhandled BSP Event received: %d", evt);
    break;
  }
}

/**@brief Function for initializing LEDs and a single PWM channel.
 */
static void leds_buttons_init(void) {
  ret_code_t err_code;
  app_pwm_config_t pwm_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000L, bsp_board_led_idx_to_pin(BULB_LED));

  /* Initialize all LEDs and buttons. */
  err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, buttons_handler);
  APP_ERROR_CHECK(err_code);
  /* By default the bsp_init attaches BSP_KEY_EVENTS_{0-4} to the PUSH events of the corresponding buttons. */

  /* Initialize PWM running on timer 1 in order to control dimmable light bulb. */
  err_code = app_pwm_init(&BULB_PWM_NAME, &pwm_cfg, NULL);
  APP_ERROR_CHECK(err_code);

  app_pwm_enable(&BULB_PWM_NAME);

  while (app_pwm_channel_duty_set(&BULB_PWM_NAME, 0, 99) == NRF_ERROR_BUSY) {
  }
}

/**@brief Function for initializing all clusters attributes.
 */
static void bulb_clusters_attr_init(void) {
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

  ZB_ZCL_SET_ATTRIBUTE(HA_DIMMABLE_LIGHT_ENDPOINT,
      ZB_ZCL_CLUSTER_ID_ON_OFF,
      ZB_ZCL_CLUSTER_SERVER_ROLE,
      ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
      (zb_uint8_t *)&m_dev_ctx.on_off_attr.on_off,
      ZB_FALSE);

  ZB_ZCL_SET_ATTRIBUTE(HA_DIMMABLE_LIGHT_ENDPOINT,
      ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
      ZB_ZCL_CLUSTER_SERVER_ROLE,
      ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID,
      (zb_uint8_t *)&m_dev_ctx.level_control_attr.current_level,
      ZB_FALSE);
}

/**@brief Function for sending add group request to the local node.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer used to pass received data.
 */
static zb_void_t add_group_id(zb_bufid_t bufid) {

  zb_get_long_address(local_node_ieee_addr);
  local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
  local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);

  NRF_LOG_INFO("Include device 0x%x, ep %d to the group 0x%x", local_node_short_addr, BENCHMARK_SERVER_ENDPOINT, GROUP_ID);

  //  ZB_ZCL_LEVEL_CONTROL_SEND_STEP_REQ(bufid,
  //      m_device_ctx.bulb_params.short_addr,
  //      ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
  //      m_device_ctx.bulb_params.endpoint,
  //      LIGHT_SWITCH_ENDPOINT,
  //      ZB_AF_HA_PROFILE_ID,
  //      ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
  //      NULL,
  //      step_dir,
  //      LIGHT_SWITCH_DIMM_STEP,
  //      LIGHT_SWITCH_DIMM_TRANSACTION_TIME);

  ZB_ZCL_GROUPS_SEND_ADD_GROUP_REQ(bufid,
      local_node_short_addr,
      ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
      BENCHMARK_SERVER_ENDPOINT,
      BENCHMARK_CLIENT_ENDPOINT,
      ZB_AF_HA_PROFILE_ID,
      ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
      NULL,
      GROUP_ID);
}

/**@brief Function for handling Benchmark Save Message Info Command.
 *
 * @param[in]   message   Reference to Benchmark Message.
 */
void bm_save_message_info(bm_message_info message) {
  message_info[bm_message_info_nr] = message;
  bm_message_info_nr++;
}

/**@brief Function for receiving benchmark message.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer used to pass received data.
 */
static void bm_receive_message(zb_bufid_t bufid) {
  zb_zcl_parsed_hdr_t cmd_info;
  ZB_ZCL_COPY_PARSED_HEADER(bufid, &cmd_info);

  NRF_LOG_INFO("Receiving message");
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

  NRF_LOG_INFO("Benchmark Packet received with ID: %d from Src Address: 0x%x to Destination 0x%x with RSSI: %d, LQI: %d, Time: %llu", message.message_id, message.src_addr, message.dst_addr, rssi, lqi, message.net_time);

  bm_save_message_info(message);
}

/**@brief Callback function for handling custom ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
static zb_uint8_t bm_zcl_handler(zb_bufid_t bufid) {
  zb_zcl_parsed_hdr_t cmd_info;

  ZB_ZCL_COPY_PARSED_HEADER(bufid, &cmd_info);
  NRF_LOG_INFO("%s with Endpoint ID: %hd, Cluster ID: %d", __func__, cmd_info.addr_data.common_data.dst_endpoint, cmd_info.cluster_id);

  if (cmd_info.cluster_id == ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) {
    ZB_SCHEDULE_APP_ALARM(bm_receive_message, bufid, 0);
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

  NRF_LOG_INFO("zcl_device_cb id %hd", p_device_cb_param->device_cb_id);

  /* Set default response value. */
  p_device_cb_param->status = RET_OK;

  switch (p_device_cb_param->device_cb_id) {
  case ZB_ZCL_LEVEL_CONTROL_SET_VALUE_CB_ID:
    NRF_LOG_INFO("Level control setting to %d", p_device_cb_param->cb_param.level_control_set_value_param.new_value);
    level_control_set_value(p_device_cb_param->cb_param.level_control_set_value_param.new_value);
    break;

  case ZB_ZCL_SET_ATTR_VALUE_CB_ID:
    cluster_id = p_device_cb_param->cb_param.set_attr_value_param.cluster_id;
    attr_id = p_device_cb_param->cb_param.set_attr_value_param.attr_id;

    if (cluster_id == ZB_ZCL_CLUSTER_ID_ON_OFF) {
      uint8_t value = p_device_cb_param->cb_param.set_attr_value_param.values.data8;

      NRF_LOG_INFO("on/off attribute setting to %hd", value);
      if (attr_id == ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) {
        on_off_set_value((zb_bool_t)value);
      }
    } else if (cluster_id == ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) {
      uint16_t value = p_device_cb_param->cb_param.set_attr_value_param.values.data16;

      NRF_LOG_INFO("level control attribute setting to %hd", value);
      if (attr_id == ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID) {
        level_control_set_value(value);
      }
    } else {
      /* Other clusters can be processed here */
      NRF_LOG_INFO("Unhandled cluster attribute id: %d", cluster_id);
    }
    break;

  default:
    p_device_cb_param->status = RET_ERROR;
    break;
  }

  NRF_LOG_INFO("zcl_device_cb status: %hd", p_device_cb_param->status);
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

  /* Update network status LED */
  zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

  switch (sig) {
  case ZB_BDB_SIGNAL_STEERING:
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
    if (status == RET_OK) {

      /* Schedule Add Group ID request */
      zb_err_code = ZB_SCHEDULE_APP_ALARM(add_group_id, bufid, 2 * ZB_TIME_ONE_SECOND);
      ZB_ERROR_CHECK(zb_err_code);

      /* Read local node address */
      zb_get_long_address(local_node_ieee_addr);
      local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
      local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);
      NRF_LOG_INFO("Network Steering finished with Local Node Address: Short: 0x%x, IEEE/Long: 0x%s", local_node_short_addr, local_nodel_ieee_addr_buf);
      bufid = 0; // Do not free buffer - it will be reused by find_light_bulb callback.
    }
  case ZB_BDB_SIGNAL_DEVICE_REBOOT:
    if (status == RET_OK) {

      /* Read local node address */
      zb_get_long_address(local_node_ieee_addr);
      local_node_short_addr = zb_address_short_by_ieee(local_node_ieee_addr);
      local_node_addr_len = ieee_addr_to_str(local_nodel_ieee_addr_buf, sizeof(local_nodel_ieee_addr_buf), local_node_ieee_addr);
      NRF_LOG_INFO("Node restarted with Local Node Address: Short: 0x%x, IEEE/Long: 0x%s", local_node_short_addr, local_nodel_ieee_addr_buf);
    }
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
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;

  /* Initialize timer, logging system and GPIOs. */
  timer_init();
  log_init();
  leds_buttons_init();

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

  /* Register callback for handling ZCL commands. */
  ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

  /* Register dimmer switch device context (endpoints). */
  ZB_AF_REGISTER_DEVICE_CTX(&dimmable_light_ctx);

  bulb_clusters_attr_init();
  level_control_set_value(m_dev_ctx.level_control_attr.current_level);
}

void bm_zigbee_enable(void) {
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;

  /** Start Zigbee Stack. */
  zb_err_code = zboss_start_no_autostart();
  ZB_ERROR_CHECK(zb_err_code);
}