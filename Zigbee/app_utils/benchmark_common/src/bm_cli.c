#include "bm_cli.h"
#include "bm_config.h"

#ifdef ZEPHYR_BLE_MESH

#elif defined NRF_SDK_Zigbee
// function is Define in header file
#include "nrf.h"
#include "nrf_cli.h"
#include "nrf_cli_types.h"
#include "nrf_drv_clock.h"

#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_backend_flash.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_instance.h"

#include "app_usbd.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_core.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_string_desc.h"
#include "nrf_cli_cdc_acm.h"
#include "nrf_drv_usbd.h"

#define CLI_OVER_USB_CDC_ACM 1

#endif

/* Init the Parameters */
bm_params_t bm_params = {BENCHMARK_DEFAULT_TIME_S, BENCHMARK_DEFAULT_PACKETS_CNT};
bm_params_t bm_params_buf = {BENCHMARK_DEFAULT_TIME_S, BENCHMARK_DEFAULT_PACKETS_CNT};

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
void bm_cli_log(const char *fmt, ...) {
  // Zephyr way to Log info
  printk(fmt);
}
#elif defined NRF_SDK_Zigbee

// function is Define in header file
NRF_LOG_INSTANCE_REGISTER(ZIGBEE_CLI_LOG_NAME, NRF_LOG_SUBMODULE_NAME,
    ZIGBEE_CLI_CONFIG_INFO_COLOR,
    ZIGBEE_CLI_CONFIG_DEBUG_COLOR,
    ZIGBEE_CLI_CONFIG_LOG_INIT_FILTER_LEVEL,
    ZIGBEE_CLI_CONFIG_LOG_ENABLED ? ZIGBEE_CLI_CONFIG_LOG_LEVEL : NRF_LOG_SEVERITY_NONE);

// This structure keeps reference to the logger instance used by this module.
typedef struct {
  NRF_LOG_INSTANCE_PTR_DECLARE(p_log)
} log_ctx_t;

// Logger instance used by this module.
static log_ctx_t m_log = {
    NRF_LOG_INSTANCE_PTR_INIT(p_log, ZIGBEE_CLI_LOG_NAME, NRF_LOG_SUBMODULE_NAME)};

NRF_CLI_CDC_ACM_DEF(m_cli_cdc_acm_transport);
NRF_CLI_DEF(m_cli_cdc_acm,
    "> ",
    &m_cli_cdc_acm_transport.transport,
    '\r',
    CLI_EXAMPLE_LOG_QUEUE_SIZE);

static void usbd_user_ev_handler(app_usbd_event_type_t event) {
  switch (event) {
  case APP_USBD_EVT_STOPPED:
    app_usbd_disable();
    break;
  case APP_USBD_EVT_POWER_DETECTED:
    if (!nrf_drv_usbd_is_enabled()) {
      app_usbd_enable();
    }
    break;
  case APP_USBD_EVT_POWER_REMOVED:
    app_usbd_stop();
    break;
  case APP_USBD_EVT_POWER_READY:
    app_usbd_start();
    break;
  default:
    break;
  }
}

static void bm_usbd_init(void) {
  ret_code_t ret;
  static const app_usbd_config_t usbd_config = {
      .ev_handler = app_usbd_event_execute,
      .ev_state_proc = usbd_user_ev_handler};

  app_usbd_serial_num_generate();

  ret = app_usbd_init(&usbd_config);
  APP_ERROR_CHECK(ret);

  app_usbd_class_inst_t const *class_cdc_acm =
      app_usbd_cdc_acm_class_inst_get(&nrf_cli_cdc_acm);
  ret = app_usbd_class_append(class_cdc_acm);
  APP_ERROR_CHECK(ret);
}
static void bm_usbd_enable(void) {
  ret_code_t ret;

  if (USBD_POWER_DETECTION) {
    ret = app_usbd_power_events_enable();
    APP_ERROR_CHECK(ret);
  } else {
    NRF_LOG_INST_INFO(m_log.p_log, "No USB power detection enabled\r\nStarting USB now");

    app_usbd_enable();
    app_usbd_start();
  }

  /* Give some time for the host to enumerate and connect to the USB CDC port */
  nrf_delay_ms(1000);
}

void bm_cli_start(void) {
  ret_code_t ret;

  bm_usbd_enable();

  ret = nrf_cli_start(&m_cli_cdc_acm);
  APP_ERROR_CHECK(ret);
}

void bm_cli_init(void) {
  ret_code_t ret;

  ret = nrf_cli_init(&m_cli_cdc_acm, NULL, true, true, NRF_LOG_SEVERITY_INFO);
  APP_ERROR_CHECK(ret);

  bm_usbd_init();
}

void bm_cli_process(void) {
  nrf_cli_process(&m_cli_cdc_acm);
}

/* ======================== CLI Commands ===================== */

static void cmd_nordic(nrf_cli_t const *p_cli, size_t argc, char **argv) {
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);

  if (nrf_cli_help_requested(p_cli)) {
    nrf_cli_help_print(p_cli, NULL, 0);
    return;
  }

  nrf_cli_fprintf(p_cli, NRF_CLI_OPTION,
      "\n"
      "            .co:.                   'xo,          \n"
      "         .,collllc,.             'ckOOo::,..      \n"
      "      .:ooooollllllll:'.     .;dOOOOOOo:::;;;'.   \n"
      "   'okxddoooollllllllllll;'ckOOOOOOOOOo:::;;;,,,' \n"
      "   OOOkxdoooolllllllllllllllldxOOOOOOOo:::;;;,,,'.\n"
      "   OOOOOOkdoolllllllllllllllllllldxOOOo:::;;;,,,'.\n"
      "   OOOOOOOOOkxollllllllllllllllllcccldl:::;;;,,,'.\n"
      "   OOOOOOOOOOOOOxdollllllllllllllccccc::::;;;,,,'.\n"
      "   OOOOOOOOOOOOOOOOkxdlllllllllllccccc::::;;;,,,'.\n"
      "   kOOOOOOOOOOOOOOOOOOOkdolllllllccccc::::;;;,,,'.\n"
      "   kOOOOOOOOOOOOOOOOOOOOOOOxdllllccccc::::;;;,,,'.\n"
      "   kOOOOOOOOOOOOOOOOOOOOOOOOOOkxolcccc::::;;;,,,'.\n"
      "   kOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOkdlc::::;;;,,,'.\n"
      "   xOOOOOOOOOOOxdkOOOOOOOOOOOOOOOOOOOOxoc:;;;,,,'.\n"
      "   xOOOOOOOOOOOdc::ldkOOOOOOOOOOOOOOOOOOOkdc;,,,''\n"
      "   xOOOOOOOOOOOdc::;;,;cdkOOOOOOOOOOOOOOOOOOOxl;''\n"
      "   .lkOOOOOOOOOdc::;;,,''..;oOOOOOOOOOOOOOOOOOOOx'\n"
      "      .;oOOOOOOdc::;;,.       .:xOOOOOOOOOOOOd;.  \n"
      "          .:xOOdc:,.              'ckOOOOkl'      \n"
      "             .od'                    'xk,         \n"
      "\n");

  nrf_cli_print(p_cli, "                Nordic Semiconductor              \n");
}

NRF_CLI_CMD_REGISTER(nordic, NULL, "Print Nordic Semiconductor logo.", cmd_nordic);

#endif