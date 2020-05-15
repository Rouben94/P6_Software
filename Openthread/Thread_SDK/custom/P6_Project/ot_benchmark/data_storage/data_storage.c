
#include "data_storage.h"

#include "fds.h"
#include "nrf_log.h"


#define FILE_ID       0x0001 /* The ID of the file to write the records into. */
#define RECORD_KEY_1  0x1111 /* A key for the first record. */
#define RECORD_KEY_2  0x2222 /* A key for the second record. */

static uint32_t const m_deadbeef = 0xDEADBEEF;
static char const m_hello[] = "Hello, world!";

fds_record_t record;
fds_record_desc_t record_desc;

// Simple event handler to handle errors during initialization.
static void fds_evt_handler(fds_evt_t const *p_fds_evt) {
  switch (p_fds_evt->id) {
  case FDS_EVT_INIT:
    if (p_fds_evt->result != NRF_SUCCESS) {
      NRF_LOG_INFO("FDS init has failed");
    }
    break;
  default:
    break;
  }
}

void set_record(void) {
  // Set up record.
  record.file_id = FILE_ID;
  record.key = RECORD_KEY_1;
  record.data.p_data = &m_deadbeef;
  record.data.length_words = 1; /* one word is four bytes. */

  ret_code_t rc;
  rc = fds_record_write(&record_desc, &record);
  if (rc != NRF_SUCCESS) {
    NRF_LOG_INFO("FDS write has failed");
  }


  // Set up record.
  record.file_id = FILE_ID;
  record.key = RECORD_KEY_2;
  record.data.p_data = &m_hello;

  /* The following calculation takes into account any eventual remainder of the division. */
  record.data.length_words = (sizeof(m_hello) + 3) / 4;
  rc = fds_record_write(&record_desc, &record);
  if (rc != NRF_SUCCESS) {
    NRF_LOG_INFO("FDS write has failed");
  }

  NRF_LOG_INFO("FDS write was succesfull");
}

/**@brief Function for initializing flash data storage module.
 */
void data_storage_init(void) {
  ret_code_t ret;
  ret = fds_register(fds_evt_handler);
  if (ret != NRF_SUCCESS) {
    NRF_LOG_INFO("Registering of the FDS event handler has failed");
  }

  ret = fds_init();
  if (ret != NRF_SUCCESS) {
    NRF_LOG_INFO("FDS init has failed");
  }
}


