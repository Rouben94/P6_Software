
#include "bm_cli.h"
#include "bm_config.h"

#ifdef ZEPHYR_BLE_MESH
#include <shell/shell.h>
#include <stdlib.h>
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#include "boards.h"
#include "nrf_log_default_backends.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef BENCHMARK_MASTER

// function is Define in header file

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"

#include "app_error.h"
#include "app_timer.h"
#include "app_util.h"
#include "fds.h"

#include "nrf_cli.h"
#include "nrf_cli_rtt.h"
#include "nrf_cli_types.h"

#include "nrf_fstorage_nvmc.h"
#include "nrf_log.h"
#include "nrf_log_backend_flash.h"
#include "nrf_log_ctrl.h"

#include "nrf_mpu_lib.h"
#include "nrf_stack_guard.h"

#include "nrf_cli_uart.h"
#endif
#endif

/* Init the Parameters */
bm_params_t bm_params = {BENCHMARK_DEFAULT_TIME_S, BENCHMARK_DEFAULT_PACKETS_CNT};
bm_params_t bm_params_buf = {BENCHMARK_DEFAULT_TIME_S, BENCHMARK_DEFAULT_PACKETS_CNT};

#ifdef ZEPHYR_BLE_MESH
void bm_cli_log(const char *fmt, ...) {
  // Zephyr way to Log info
  printk(fmt);
}
#elif defined NRF_SDK_Zigbee
#ifdef BENCHMARK_MASTER

#if NRF_LOG_BACKEND_CRASHLOG_ENABLED
NRF_LOG_BACKEND_CRASHLOG_DEF(m_crash_log_backend);
#endif

/* Counter timer. */
APP_TIMER_DEF(m_timer_0);

/**
 * @brief Command line interface instance
 * */
#define CLI_EXAMPLE_LOG_QUEUE_SIZE (4)

NRF_CLI_UART_DEF(m_cli_uart_transport, 0, 64, 16);
NRF_CLI_DEF(m_cli_uart,
    "uart_cli:~$ ",
    &m_cli_uart_transport.transport,
    '\r',
    CLI_EXAMPLE_LOG_QUEUE_SIZE);

static void timer_handle(void *p_context) {
  UNUSED_PARAMETER(p_context);
}

static void cli_start(void) {
  ret_code_t ret;

  ret = nrf_cli_start(&m_cli_uart);
  APP_ERROR_CHECK(ret);
}

static void cli_init(void) {
  ret_code_t ret;

  nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
  uart_config.pseltxd = TX_PIN_NUMBER;
  uart_config.pselrxd = RX_PIN_NUMBER;
  uart_config.hwfc = NRF_UART_HWFC_DISABLED;
  ret = nrf_cli_init(&m_cli_uart, &uart_config, true, true, NRF_LOG_SEVERITY_INFO);
  APP_ERROR_CHECK(ret);
}

void bm_cli_process(void) {

  nrf_cli_process(&m_cli_uart);
}
#endif
void bm_cli_init(void) {
#ifdef BENCHMARK_MASTER
  ret_code_t ret;

  ret = nrf_drv_clock_init();
  APP_ERROR_CHECK(ret);
  nrf_drv_clock_lfclk_request(NULL);

  ret = app_timer_init();
  APP_ERROR_CHECK(ret);

  ret = app_timer_create(&m_timer_0, APP_TIMER_MODE_REPEATED, timer_handle);
  APP_ERROR_CHECK(ret);

  ret = app_timer_start(m_timer_0, APP_TIMER_TICKS(1000), NULL);
  APP_ERROR_CHECK(ret);

  cli_init();

  cli_start();

  NRF_LOG_RAW_INFO("Command Line Interface example started.\n");
  NRF_LOG_RAW_INFO("Please press the Tab key to see all available commands.\n");
#endif
}


void bm_cli_log_init(void) {

  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
}
#endif