/*
This file is part of Benchamrk-Shared-Library.

Benchamrk-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchamrk-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchamrk-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR 	   :    Cyrill Horath       */
/* Co-AUTHOR 	 :    Raffael Anklin       */



#include "bm_cli.h"
#include "bm_config.h"

#ifdef ZEPHYR_BLE_MESH
#include <shell/shell.h>
#include <stdlib.h>
#include <zephyr.h>
#elif defined NRF_SDK_ZIGBEE
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
//#include "fds.h"

#include "nrf_cli.h"
#include "nrf_cli_rtt.h"
#include "nrf_cli_types.h"

#include "nrf_fstorage_nvmc.h"
#include "nrf_log.h"
#include "nrf_log_backend_flash.h"
#include "nrf_log_ctrl.h"

#include "nrf_mpu_lib.h"
#include "nrf_stack_guard.h"

#include "nrf_cli_libuarte.h"
#include "nrf_cli_uart.h"
#endif
#endif

/* Init the Parameters */
bm_params_t bm_params = {BENCHMARK_DEFAULT_TIME_S, BENCHMARK_DEFAULT_PACKETS_CNT,0,0,0,0,0,0,0};
bm_params_t bm_params_buf = {BENCHMARK_DEFAULT_TIME_S, BENCHMARK_DEFAULT_PACKETS_CNT,0,0,0,0,0,0,0};

#ifdef ZEPHYR_BLE_MESH
void bm_cli_log(const char *fmt, ...) {
  // Zephyr way to Log info
  printk(fmt);
}
#elif defined NRF_SDK_ZIGBEE
#ifdef BENCHMARK_MASTER

#if NRF_LOG_BACKEND_CRASHLOG_ENABLED
NRF_LOG_BACKEND_CRASHLOG_DEF(m_crash_log_backend);
#endif

/* Counter timer. */
APP_TIMER_DEF(m_timer_0);

/**
 * @brief Command line interface instance
 * */
#define CLI_EXAMPLE_LOG_QUEUE_SIZE (100)

NRF_CLI_LIBUARTE_DEF(m_cli_libuarte_transport, 256, 256);
NRF_CLI_DEF(m_cli_libuarte,
    "libuarte_cli:~$ ",
    &m_cli_libuarte_transport.transport,
    '\r',
    CLI_EXAMPLE_LOG_QUEUE_SIZE);

static void timer_handle(void *p_context) {
  UNUSED_PARAMETER(p_context);
}

static void cli_start(void) {
  ret_code_t ret;

  ret = nrf_cli_start(&m_cli_libuarte);
  APP_ERROR_CHECK(ret);
}

static void cli_init(void) {
  ret_code_t ret;

  cli_libuarte_config_t libuarte_config;
  libuarte_config.tx_pin = TX_PIN_NUMBER;
  libuarte_config.rx_pin = RX_PIN_NUMBER;
  libuarte_config.baudrate = NRF_UARTE_BAUDRATE_115200;
  libuarte_config.parity = NRF_UARTE_PARITY_EXCLUDED;
  libuarte_config.hwfc = NRF_UARTE_HWFC_DISABLED;
  ret = nrf_cli_init(&m_cli_libuarte, &libuarte_config, true, true, NRF_LOG_SEVERITY_INFO);
  APP_ERROR_CHECK(ret);

}
#endif
void bm_cli_process(void) {
#ifdef BENCHMARK_MASTER
  nrf_cli_process(&m_cli_libuarte);
//  nrf_cli_process(&m_cli_uart);
#endif
}
void bm_cli_init(void) {
#ifdef BENCHMARK_MASTER
  ret_code_t ret;

  ret = nrf_drv_clock_init();
  APP_ERROR_CHECK(ret);
  nrf_drv_clock_lfclk_request(NULL);

//  ret = app_timer_init();
//  APP_ERROR_CHECK(ret);
//
//  ret = app_timer_create(&m_timer_0, APP_TIMER_MODE_REPEATED, timer_handle);
//  APP_ERROR_CHECK(ret);
//
//  ret = app_timer_start(m_timer_0, APP_TIMER_TICKS(1000), NULL);
//  APP_ERROR_CHECK(ret);

  cli_init();
  cli_start();

  NRF_LOG_RAW_INFO("Please press the Tab key to see all available commands.\n");
#endif
}

void bm_cli_log_init(void) {
//#ifdef BENCHMARK_MASTER
//  APP_ERROR_CHECK(NRF_LOG_INIT(app_timer_cnt_get));
//#else
  APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
//#endif
  NRF_LOG_DEFAULT_BACKENDS_INIT();
}
#endif