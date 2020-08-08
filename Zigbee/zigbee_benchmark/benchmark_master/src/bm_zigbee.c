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
#include "bm_simple_buttons_and_leds.h"
#include "bm_timesync.h"
#include "bm_zigbee.h"

/************************************ General Init Functions ***********************************************/

/**@brief Callback used in order to visualise network steering period.
 *
 * @param[in]   param   Not used. Required by callback type definition.
 */
static zb_void_t steering_finished(zb_uint8_t param) {
  UNUSED_PARAMETER(param);
  bm_cli_log("Network steering finished\n");
  bm_led2_set(true);
}

/************************************ Button Handler Functions ***********************************************/

/**@brief Callback for button events.
 *
 * @param[in]   evt      Incoming event from the BSP subsystem.
 */
static void buttons_handler(bsp_event_t evt) {
  zb_bool_t comm_status;
  zb_ret_t zb_err_code;
  zb_uint32_t button;

  switch (evt) {
  case BSP_EVENT_KEY_0:
    UNUSED_RETURN_VALUE(ZB_SCHEDULE_APP_ALARM_CANCEL(steering_finished, ZB_ALARM_ANY_PARAM));

    comm_status = bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
    if (comm_status) {
      bm_cli_log("Top level comissioning restated\n");
    } else {
      bm_cli_log("Top level comissioning hasn't finished yet!\n");
    }
    break;

  default:
    bm_cli_log("Unhandled BSP Event received: %d\n", evt);
    break;
  }
}

/**************************************** Zigbee event handler ***********************************************/

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid) {
  /* Read signal description out of memory buffer. */
  zb_zdo_app_signal_hdr_t *p_sg_p = NULL;
  zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &p_sg_p);
  zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);
  zb_ret_t zb_err_code;
  zb_bool_t comm_status;
  zb_time_t timeout_bi;

  switch (sig) {
  case ZB_BDB_SIGNAL_DEVICE_REBOOT:
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
    // BDB initialization completed after device reboot, use NVRAM contents during initialization. Device joined/rejoined and started.
    if (status == RET_OK) {
      if (ZIGBEE_MANUAL_STEERING == ZB_FALSE) {
        bm_cli_log("Start network steering\n");
        comm_status = bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
        ZB_COMM_STATUS_CHECK(comm_status);
      } else {
        bm_cli_log("Coordinator restarted successfully\n");
      }
    } else {
      NRF_LOG_ERROR("Failed to initialize Zigbee stack using NVRAM data (status: %d)", status);
    }
    break;
  case ZB_BDB_SIGNAL_FORMATION:
    if (status == RET_OK) {
      ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
      bm_cli_log("Network Formation completed\n");
      bm_cli_log("Active channel %d\n", nrf_802154_channel_get());
    }
    break;

  case ZB_BDB_SIGNAL_STEERING:
    if (status == RET_OK) {
      /* Schedule an alarm to notify about the end of steering period */
      bm_cli_log("Network steering started\n");
      zb_err_code = ZB_SCHEDULE_APP_ALARM(steering_finished, 0, ZB_TIME_ONE_SECOND * ZB_ZGP_DEFAULT_COMMISSIONING_WINDOW);
      ZB_ERROR_CHECK(zb_err_code);
    }
    zb_enable_auto_pan_id_conflict_resolution(ZB_FALSE);
    break;

  default:
    /* Call default signal handler. */
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
    break;
  }

  /* Update network status LED */
  if (ZB_JOINED() && (ZB_SCHEDULE_GET_ALARM_TIME(steering_finished, ZB_ALARM_ANY_PARAM, &timeout_bi) == RET_OK)) {
    bm_led2_set(true);

  } else {
    bm_led2_set(false);
  }

  /* All callbacks should either reuse or free passed buffers. If bufid == 0, the buffer is invalid (not passed) */
  if (bufid) {
    zb_buf_free(bufid);
  }
}

/* Get device address of the local device.*/
void bm_get_ieee_eui64(zb_ieee_addr_t ieee_eui64) {
  uint64_t factoryAddress;
  factoryAddress = NRF_FICR->DEVICEADDR[0];
  memcpy(ieee_eui64, &factoryAddress, sizeof(factoryAddress));
}

/**************************************** Zigbee Stack Init and Enable ***********************************************/

void bm_zigbee_init(void) {
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;
//  uint64_t ext_pan_id_64 = DEFAULT_PAN_ID_EXT;
//  zb_ext_pan_id_t ext_pan_id;
//  memcpy(ext_pan_id, &ext_pan_id_64, sizeof(ext_pan_id_64));

  /* Set Zigbee stack logging level and traffic dump subsystem. */
  ZB_SET_TRACE_LEVEL(ZIGBEE_TRACE_LEVEL);
  ZB_SET_TRACE_MASK(ZIGBEE_TRACE_MASK);
  ZB_SET_TRAF_DUMP_OFF();

  /* Initialize Zigbee stack. */
  ZB_INIT("zc");

  /* Set device address to the value read from FICR registers. */
  bm_get_ieee_eui64(ieee_addr);
  zb_set_long_address(ieee_addr);

  /* Set short and extended pan id to the default value. */
//  zb_set_pan_id((zb_uint16_t)DEFAULT_PAN_ID_SHORT);
//  zb_set_extended_pan_id(ext_pan_id);

  /* Set channels on which the coordinator will try to create a new network. */
  zb_set_network_coordinator_role(IEEE_CHANNEL_MASK);
  zb_set_max_children(MAX_CHILDREN);

  /* Keep or erase NVRAM to save the network parameters after device reboot or power-off. */
  zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);
}

void bm_zigbee_enable(void) {
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;

  /** Start Zigbee Stack. */
  zb_err_code = zboss_start_no_autostart();
  ZB_ERROR_CHECK(zb_err_code);

  bm_cli_log("BENCHMARK Master ready\n");
}