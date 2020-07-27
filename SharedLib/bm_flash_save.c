/** @file
 *
 * @defgroup ot_benchmark flash_save.c
 * @{
 * @ingroup ot_benchmark
 * @brief OpenThread benchmark Application flash_save file.
 *
 * @details This files handels the flash save settings.
 *
 */
#include "bm_flash_save.h"
#include "bm_cli.h"
#include "bm_config.h"
#ifdef NRF_SDK_ZIGBEE
#include "fds.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define FILE_ID 0x0001 /* The ID of the file to write the records into. */
#define RECORD_KEY 0x1111

flash_cb_t cb;

static void fds_evt_handler(fds_evt_t const *p_fds_evt) {
  switch (p_fds_evt->id) {
  case FDS_EVT_INIT:
    if (p_fds_evt->result == NRF_SUCCESS) {

      bm_cli_log("Succesfully initialized FDS module\n");
    }
    break;

  case FDS_EVT_WRITE:
    if (p_fds_evt->result == NRF_SUCCESS) {
//      bm_cli_log("FDS record write was successful\n");
    }
    break;

  case FDS_EVT_UPDATE:
    if (p_fds_evt->result == NRF_SUCCESS) {
//      bm_cli_log("FDS record read was successful\n");
    }
    break;

  case FDS_EVT_DEL_RECORD:
    if (p_fds_evt->result == NRF_SUCCESS) {
//      bm_cli_log("FDS record delete was successful\n");
    }
    break;

  default:
    break;
  }
}

void flash_read(void) {
  fds_record_desc_t record_desc;
  fds_flash_record_t flash_record;
  fds_find_token_t ftok;
  Measurement *measure;

  memset(&ftok, 0x00, sizeof(fds_find_token_t));

  while (fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok) == NRF_SUCCESS) {
    ret_code_t rc = fds_record_open(&record_desc, &flash_record);
    if (rc != NRF_SUCCESS) {
      bm_cli_log("FDS open record error: 0x%x\n", rc);
      if (rc != FDS_ERR_NO_SPACE_IN_FLASH) {
        fds_gc();
      }
    }

    measure = (Measurement *)flash_record.p_data;
    cb(measure);

    rc = fds_record_close(&record_desc);
    if (rc != NRF_SUCCESS) {
      bm_cli_log("FDS close record error: 0x%x\n", rc);
      if (rc != FDS_ERR_NO_SPACE_IN_FLASH) {
        fds_gc();
      }
    }
  }
}

void flash_write(Measurement measure) {
  fds_record_desc_t record_desc;
  fds_record_t record;
  ret_code_t rc;

  record.file_id = FILE_ID;
  record.key = RECORD_KEY;
  record.data.p_data = &measure;

  record.data.length_words = (sizeof(measure) + 3) / sizeof(uint32_t);
  rc = fds_record_write(&record_desc, &record);
  if (rc != NRF_SUCCESS) {
    bm_cli_log("FDS write error: 0x%x\n", rc);
    if (rc != FDS_ERR_NO_SPACE_IN_FLASH) {
      fds_gc();
    }
  }
}

void flash_delete(void) {
  fds_record_desc_t record_desc;
  fds_flash_record_t flash_record;
  fds_find_token_t ftok;
  Measurement *measure;

  memset(&ftok, 0x00, sizeof(fds_find_token_t));

  while (fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok) == NRF_SUCCESS) {
    ret_code_t rc = fds_record_delete(&record_desc);
    if (rc != NRF_SUCCESS) {
      bm_cli_log("FDS delete error: 0x%x\n", rc);
      if (rc != FDS_ERR_NO_SPACE_IN_FLASH) {
        fds_gc();
      }
    }
  }
}

void flash_save_init(flash_cb_t evt_handler) {
  ret_code_t rc;
  rc = fds_register(fds_evt_handler);
  APP_ERROR_CHECK(rc);

  rc = fds_init();
  APP_ERROR_CHECK(rc);

  cb = evt_handler;
}

#endif