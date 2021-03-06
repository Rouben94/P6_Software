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
#include "zb_ha_dimmable_light.h"
#include "zb_nrf52_internal.h"
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

/* Main application customizable context. Stores all settings and static values. */

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

zb_uint8_t seq_num = 0;
zb_uint16_t msg_receive_cnt = 0;

zb_uint32_t stack_enable_max_delay_ms = STACK_STARTUP_MAX_DELAY;
zb_uint32_t network_formation_delay = NETWORK_FORMATION_DELAY;
zb_uint16_t def_payload_size = DEFAULT_PAYLOAD;

void bm_receive_message(zb_bufid_t bufid);

/************************************ Benchmark Client Cluster Attribute Init ***********************************************/

/**@brief Function for initializing all clusters attributes.
 */
static void bm_server_clusters_attr_init(void) {
  /* Basic cluster attributes data */
  m_dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
  m_dev_ctx.basic_attr.app_version = BULB_INIT_BASIC_APP_VERSION;
  m_dev_ctx.basic_attr.stack_version = BULB_INIT_BASIC_STACK_VERSION;
  m_dev_ctx.basic_attr.hw_version = BULB_INIT_BASIC_HW_VERSION;

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

/* Get device address of the local device.*/
void bm_get_ieee_eui64(zb_ieee_addr_t ieee_eui64) {
  uint64_t factoryAddress;
  factoryAddress = NRF_FICR->DEVICEADDR[0];
  memcpy(ieee_eui64, &factoryAddress, sizeof(factoryAddress));
}

/**@brief Function for turning ON/OFF the light bulb.
 *
 * @param[in]   on   Boolean light bulb state.
 */
static void bm_on_off_set_value(zb_bool_t on) {
  bm_cli_log("Set ON/OFF value: %i\n", on);

  ZB_ZCL_SET_ATTRIBUTE(BENCHMARK_SERVER_ENDPOINT,
      ZB_ZCL_CLUSTER_ID_ON_OFF,
      ZB_ZCL_CLUSTER_SERVER_ROLE,
      ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
      (zb_uint8_t *)&on,
      ZB_FALSE);

  if (on) {
    bm_led3_set(true);
  } else {
    bm_led3_set(false);
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
  zb_int8_t rssi = 0;
  zb_ieee_addr_t ieee_dst_addr;
  zb_zcl_parsed_hdr_t *zcl_info = ZB_BUF_GET_PARAM(bufid, zb_zcl_parsed_hdr_t);

  message.net_time = synctimer_getSyncTime();

  message.number_of_hops = 0;                                             /* TODO: Number of hops is not yet available from the ZBOSS API */
  message.data_size = bm_params.AdditionalPayloadSize + def_payload_size; /* Payload Size is default payload size plus additional payload size from config message. */
  message.ack_net_time = 0;

  message.src_addr = zcl_info->addr_data.common_data.source.u.short_addr;
  zb_get_long_address(ieee_dst_addr);
  message.dst_addr = zb_address_short_by_ieee(ieee_dst_addr);
  message.group_addr = bm_params.GroupAddress + GROUP_ID;

  /* Manufacturer specific field in ZCL Header is used to transmit Message ID.
  First check ZCL header if manufacturer specific field is available. */
  if (zcl_info->is_manuf_specific) {
    message.message_id = zcl_info->manuf_specific;
  } else {
    message.message_id = 0;
  }

  zb_zdo_get_diag_data(message.src_addr, &lqi, &rssi);
  message.rssi = rssi;

  bm_cli_log("Benchmark Packet received with ID: %d from Src Address: 0x%x to Destination 0x%x with RSSI: %d (dB), Time: %llu, Data size: %d\n", message.message_id, message.src_addr, message.dst_addr, rssi, message.net_time, message.data_size);

  bm_log_append_ram(message);
  bm_on_off_set_value((zb_bool_t)message.message_id % 2);
}

/************************************ Zigbee event handler ***********************************************/

/**@brief Callback function for handling ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer used to pass received data.
 */
zb_uint8_t bm_ep_handler(zb_bufid_t bufid) {
  zb_zcl_parsed_hdr_t *zcl_info = ZB_BUF_GET_PARAM(bufid, zb_zcl_parsed_hdr_t);
  zb_uint16_t src_addr;
  msg_receive_cnt++;
  src_addr = zcl_info->addr_data.common_data.source.u.short_addr;

  bm_cli_log("Message received in bm_ep_handler from source: 0x%x, cnt: %d, cmd_id: %d\n", src_addr, msg_receive_cnt, zcl_info->cmd_id);

  switch (zcl_info->cluster_id) {
  case ZB_ZCL_CLUSTER_ID_GROUPS:
    bm_cli_log("ZCL Groups command received\n");
    return ZB_FALSE;
    break;

  case ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL:

    bm_receive_message(bufid); /* Receive message and read benchmark values. */

    if (bufid) {
      zb_buf_free(bufid);
    }
    return ZB_TRUE;

  default:
    return ZB_FALSE;
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

  /* Update network status LED */
  zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

  switch (sig) {
  case ZB_BDB_SIGNAL_STEERING:
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid)); /* Call default signal handler. */
    bm_cli_log("Zigbee Network Steering\n");
    bm_cli_log("Active channel %d\n", nrf_802154_channel_get());

    if (status == RET_OK) {
      /* Schedule Add Group ID request */
      zb_err_code = ZB_SCHEDULE_APP_ALARM(add_group_id, bufid, 2 * ZB_TIME_ONE_SECOND);
      ZB_ERROR_CHECK(zb_err_code);

      bufid = 0;
    }
    break;

  case ZB_BDB_SIGNAL_DEVICE_REBOOT:
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid)); /* Call default signal handler. */
    bm_cli_log("Zigbee Network Steering\n");
    bm_cli_log("Active channel %d\n", nrf_802154_channel_get());

    if (status == RET_OK) {
      /* Schedule Add Group ID request */
      zb_err_code = ZB_SCHEDULE_APP_ALARM(add_group_id, bufid, 2 * ZB_TIME_ONE_SECOND);
      ZB_ERROR_CHECK(zb_err_code);

      bufid = 0;
    }
    break;

  case ZB_NWK_SIGNAL_NO_ACTIVE_LINKS_LEFT:
    /* Fall through */
    bufid = 0;
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

/**************************************** Zigbee Stack Init and Enable ***********************************************/

/** Init Zigbee Stack. */
void bm_zigbee_init(void) {
  zb_ieee_addr_t ieee_addr;
  uint64_t long_address;

  /* Initialize timer, logging system and GPIOs. */
  timer_init();

  /* Set Zigbee stack logging level and traffic dump subsystem. */
  ZB_SET_TRACE_LEVEL(ZIGBEE_TRACE_LEVEL);
  ZB_SET_TRACE_MASK(ZIGBEE_TRACE_MASK);
  ZB_SET_TRAF_DUMP_OFF();

  /* Initialize Zigbee stack. */
  ZB_INIT("Benchmark Server");

  /* Set device address to the value read from FICR registers. */
  bm_get_ieee_eui64(ieee_addr);
  zb_set_long_address(ieee_addr);

  /* Set static long IEEE address. */
  zb_set_network_router_role(IEEE_CHANNEL_MASK);
  zb_set_max_children(MAX_CHILDREN);

  /* Erase persistent config of the device if ACK Param in config message is set true. Else delete persistent storage at startup. */
  zigbee_erase_persistent_storage(bm_params.Ack);

  zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(3000));

  /* Initialize application context structure. */
  UNUSED_RETURN_VALUE(ZB_MEMSET(&m_dev_ctx, 0, sizeof(m_dev_ctx)));

  /* Register dimmer switch device context (endpoints). */
  ZB_AF_REGISTER_DEVICE_CTX(&bm_server_ctx);

  /* Register callback for handling ZCL commands. */
  ZB_AF_SET_ENDPOINT_HANDLER(BENCHMARK_SERVER_ENDPOINT, bm_ep_handler);

  /* Initialize Cluster Attributes */
  bm_server_clusters_attr_init();
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
  bm_led3_set(true);
  bm_cli_log("BENCHMARK Server ready\n");
}