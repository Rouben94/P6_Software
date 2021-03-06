/*
This file is part of Benchmark-Shared-Library.

Benchmark-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchmark-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchmark-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR	   :   Robin Bobst        */
/* AUTHOR	   :   Cyrill Horath      */

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
#include "bm_simple_buttons_and_leds.h"
#include "bm_timesync.h"
#if defined NRF_SDK_ZIGBEE || defined NRF_SDK_THREAD || defined NRF_SDK_MESH
#include "fds.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define FILE_ID 0x0001 /* The ID of the file to write the records into. */
#define RECORD_KEY 0x1111

flash_cb_t cb;
bool blocked = false;

static void fds_evt_handler(fds_evt_t const * p_fds_evt)
{
    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result == NRF_SUCCESS)
            {

                bm_cli_log("Succesfully initialized FDS module\n");
            }
            break;

        case FDS_EVT_WRITE:
            blocked = false;
            if (p_fds_evt->result == NRF_SUCCESS)
            {
            }
            break;

        case FDS_EVT_UPDATE:
            blocked = false;
            if (p_fds_evt->result == NRF_SUCCESS)
            {
            }
            break;

        case FDS_EVT_DEL_RECORD:
            blocked = false;
            if (p_fds_evt->result == NRF_SUCCESS)
            {
            }
            break;

        case FDS_EVT_GC:
            blocked = false;
            if (p_fds_evt->result == NRF_SUCCESS)
            {
            }
            break;

        default:
            break;
    }
}

void flash_read(void)
{
    fds_record_desc_t record_desc;
    fds_flash_record_t flash_record;
    fds_find_token_t ftok;
    bm_message_info * msg;

    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    while (fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok) == NRF_SUCCESS)
    {
        ret_code_t rc = fds_record_open(&record_desc, &flash_record);
        if (rc != NRF_SUCCESS)
        {
            bm_cli_log("FDS open record error: 0x%x\n", rc);
        }

        msg = (bm_message_info *)flash_record.p_data;
        cb(msg);

        rc = fds_record_close(&record_desc);
        if (rc != NRF_SUCCESS)
        {
            bm_cli_log("FDS close record error: 0x%x\n", rc);
        }
    }
}

void flash_write(bm_message_info msg)
{
    fds_record_desc_t record_desc;
    fds_record_t record;
    ret_code_t rc;

    record.file_id = FILE_ID;
    record.key = RECORD_KEY;
    record.data.p_data = &msg;
    record.data.length_words = (sizeof(bm_message_info)) / sizeof(uint32_t);

    blocked = true;
    rc = fds_record_write(&record_desc, &record);
    if (rc != NRF_SUCCESS)
    {
        bm_cli_log("FDS delete error: 0x%x\n", rc);
        bm_led1_set(true);
    }
    else
    {
        while (blocked)
        {
        };
    }
}

void flash_delete(void)
{
    fds_record_desc_t record_desc;
    fds_flash_record_t flash_record;
    fds_find_token_t ftok;
    bm_message_info * msg;

    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    while (fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok) == NRF_SUCCESS)
    {
        blocked = true;
        ret_code_t rc = fds_record_delete(&record_desc);
        if (rc != NRF_SUCCESS)
        {
            bm_cli_log("FDS delete error: 0x%x\n", rc);
            bm_led1_set(true);
        }
        else
        {
            while (blocked)
            {
            };
        }
    }

    blocked = true;
    ret_code_t rc = fds_gc();
    if (rc != NRF_SUCCESS)
    {
        bm_cli_log("FDS delete error: 0x%x\n", rc);
        bm_led1_set(true);
    }
    else
    {
        while (blocked)
        {
        };
    }
}

void flash_save_init(flash_cb_t evt_handler)
{
    ret_code_t rc;
    rc = fds_register(fds_evt_handler);
    APP_ERROR_CHECK(rc);
    

    fds_evt_t test_evt;
    test_evt.id = FDS_EVT_INIT;
    fds_evt_handler(&test_evt);

    rc = fds_init();
    APP_ERROR_CHECK(rc);

    cb = evt_handler;
}

#endif