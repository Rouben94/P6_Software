
#include "zb_error_handler.h"
#include "zb_mem_config_max.h"
#include "zboss_api.h"
#include "zigbee_helpers.h"

#include "app_timer.h"
#include "boards.h"
#include "bsp.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bm_config.h"
#include "bm_zigbee.h"

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

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

/**@brief Callback used in order to visualise network steering period.
 *
 * @param[in]   param   Not used. Required by callback type definition.
 */
static zb_void_t steering_finished(zb_uint8_t param) {
  UNUSED_PARAMETER(param);
  NRF_LOG_INFO("Network steering finished");
  bsp_board_led_off(ZIGBEE_NETWORK_STATE_LED);
}

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
      NRF_LOG_INFO("Top level comissioning restated");
    } else {
      NRF_LOG_INFO("Top level comissioning hasn't finished yet!");
    }
    break;
  case BSP_EVENT_KEY_3:
    button = BENCHMARK_BUTTON;
    NRF_LOG_INFO("BENCHMARK Button pressed");
    //     zb_err_code = ZB_SCHEDULE_APP_ALARM(coordinator_button_handler, button, 0);
    //     if (zb_err_code == RET_OVERFLOW) {
    //      NRF_LOG_WARNING("Can not schedule another alarm, queue is full.");
    //    } else {
    //      ZB_ERROR_CHECK(zb_err_code);
    //    }
    break;

  default:
    NRF_LOG_INFO("Unhandled BSP Event received: %d", evt);
    break;
  }
}

/**@brief Function for initializing LEDs and Buttons.
 */
static void leds_buttons_init(void) {
  uint32_t err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, buttons_handler);
  APP_ERROR_CHECK(err_code);
  /* By default the bsp_init attaches BSP_KEY_EVENTS_{0-4} to the PUSH events of the corresponding buttons. */

  bsp_board_leds_off();
}

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
    // BDB initialization completed after device reboot, use NVRAM contents during initialization. Device joined/rejoined and started.
    if (status == RET_OK) {
      if (ZIGBEE_MANUAL_STEERING == ZB_FALSE) {
        NRF_LOG_INFO("Start network steering");
        comm_status = bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
        ZB_COMM_STATUS_CHECK(comm_status);
      } else {
        NRF_LOG_INFO("Coordinator restarted successfully");
      }
    } else {
      NRF_LOG_ERROR("Failed to initialize Zigbee stack using NVRAM data (status: %d)", status);
    }
    break;

  case ZB_BDB_SIGNAL_STEERING:
    if (status == RET_OK) {
      if (ZIGBEE_PERMIT_LEGACY_DEVICES == ZB_TRUE) {
        NRF_LOG_INFO("Allow pre-Zigbee 3.0 devices to join the network");
        zb_bdb_set_legacy_device_support(1);
      }

      /* Schedule an alarm to notify about the end of steering period */
      NRF_LOG_INFO("Network steering started");
      zb_err_code = ZB_SCHEDULE_APP_ALARM(steering_finished, 0, ZB_TIME_ONE_SECOND * ZB_ZGP_DEFAULT_COMMISSIONING_WINDOW);
      ZB_ERROR_CHECK(zb_err_code);
    }
#ifndef ZB_ED_ROLE
    zb_enable_auto_pan_id_conflict_resolution(ZB_FALSE);
#endif
    break;

  case ZB_ZDO_SIGNAL_DEVICE_ANNCE: {
    zb_zdo_signal_device_annce_params_t *dev_annce_params = ZB_ZDO_SIGNAL_GET_PARAMS(p_sg_p, zb_zdo_signal_device_annce_params_t);
    NRF_LOG_INFO("New device commissioned or rejoined (short: 0x%04hx)", dev_annce_params->device_short_addr);

    zb_err_code = ZB_SCHEDULE_APP_ALARM_CANCEL(steering_finished, ZB_ALARM_ANY_PARAM);
    if (zb_err_code == RET_OK) {
      NRF_LOG_INFO("Joining period extended.");
      zb_err_code = ZB_SCHEDULE_APP_ALARM(steering_finished, 0, ZB_TIME_ONE_SECOND * ZB_ZGP_DEFAULT_COMMISSIONING_WINDOW);
      ZB_ERROR_CHECK(zb_err_code);
    }
  } break;

  default:
    /* Call default signal handler. */
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
    break;
  }

  /* Update network status LED */
  if (ZB_JOINED() && (ZB_SCHEDULE_GET_ALARM_TIME(steering_finished, ZB_ALARM_ANY_PARAM, &timeout_bi) == RET_OK)) {
    bsp_board_led_on(ZIGBEE_NETWORK_STATE_LED);
  } else {
    bsp_board_led_off(ZIGBEE_NETWORK_STATE_LED);
  }

  /* All callbacks should either reuse or free passed buffers. If bufid == 0, the buffer is invalid (not passed) */
  if (bufid) {
    zb_buf_free(bufid);
  }
}

void bm_zigbee_init(void) {
  zb_ret_t zb_err_code;
  zb_ieee_addr_t ieee_addr;
  // Initialize.
  timers_init();
  log_init();
  leds_buttons_init();

  /* Set Zigbee stack logging level and traffic dump subsystem. */
  ZB_SET_TRACE_LEVEL(ZIGBEE_TRACE_LEVEL);
  ZB_SET_TRACE_MASK(ZIGBEE_TRACE_MASK);
  ZB_SET_TRAF_DUMP_OFF();

  /* Initialize Zigbee stack. */
  ZB_INIT("zc");

  /* Set device address to the value read from FICR registers. */
  zb_osif_get_ieee_eui64(ieee_addr);
  zb_set_long_address(ieee_addr);

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

  NRF_LOG_INFO("BENCHMARK Master ready");
}