#include "bm_log.h"
//#include "bm_cli.h"
#include "bm_config.h"
#include "bm_flash_save.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include <string.h>

// Save the Loged Data to
bm_message_info message_info[NUMBER_OF_BENCHMARK_REPORT_MESSAGES] = {0};

uint16_t bm_message_cnt; // Count the Log Entry

void bm_log_clear_ram() {
  bm_message_cnt = 0;
  memset(message_info, 0, sizeof(message_info));
}

void bm_log_append_ram(bm_message_info message) {
  if (bm_message_cnt > sizeof(message_info) - 1) {
    NRF_LOG_INFO("Log Buffer Overflow... Ignoring Data\n");
    return;
  }
  message_info[bm_message_cnt] = message;
  bm_message_cnt++;
}

void bm_log_save_to_flash() {
  uint16_t bm_message_cnt_flash = 0;

  while (bm_message_cnt_flash < bm_message_cnt) {

    flash_write(*((Measurement *)&message_info[bm_message_cnt_flash]));
    NRF_LOG_INFO("Message write to Flash Message Number: %d", bm_message_cnt_flash);
    bm_message_cnt_flash++;
  }
  NRF_LOG_INFO("Write to Flash finished. %d data packets saved", bm_message_cnt_flash);
}

/* Callback function to read Benchmark Message Info data from Flash. */
void bm_log_load_from_flash_cb(Measurement *data) {

  message_info[bm_message_cnt] = *((bm_message_info *)data);
  bm_message_cnt++;
}

uint16_t bm_log_load_from_flash() {
  bm_message_cnt = 0;
  flash_read();
  NRF_LOG_INFO("Read data from Flash done");

  return bm_message_cnt;
}

void bm_log_clear_flash() {
  flash_delete();
  NRF_LOG_INFO("Flash Data deleted");
}

void bm_log_init() {
  flash_save_init(bm_log_load_from_flash_cb);
}