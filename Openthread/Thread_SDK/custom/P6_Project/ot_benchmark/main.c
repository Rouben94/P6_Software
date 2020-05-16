/**
 * Copyright (c) 2017 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup simple_coap_client_example_main main.c
 * @{
 * @ingroup simple_coap_client_example_example
 * @brief Simple CoAP Client Example Application main file.
 *
 * @details This example demonstrates a CoAP client application that enables to control BSP_LED_0
 *          on a board with related Simple CoAP Server application via CoAP messages.
 *
 */

#include "app_scheduler.h"
#include "app_timer.h"
#include "bsp_thread.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "fds.h"

#include "thread_coap_utils.h"
#include "thread_utils.h"

#include "board_support_config.h"

#include <openthread/instance.h>
#include <openthread/network_time.h>
#include <openthread/platform/time.h>
#include <openthread/thread.h>

#define SCHED_QUEUE_SIZE 32 /**< Maximum number of events in the scheduler queue. */
#define SCHED_EVENT_DATA_SIZE                                                                      \
    APP_TIMER_SCHED_EVENT_DATA_SIZE /**< Maximum app_scheduler event size. */


/* Flag to check fds initialization. */
static bool volatile m_fds_initialized;

#define FILE_ID 0x0001      /* The ID of the file to write the records into. */
#define RECORD_KEY_1 0x1111 /* A key for the first record. */
#define RECORD_KEY_2 0x2222 /* A key for the second record. */
#define RECORD_KEY  0x2222

static uint32_t const m_deadbeef = 0xDEADBEEF;
static char const m_hello[] = "Hello, world!";

fds_record_t record;
fds_flash_record_t flash_record;
fds_record_desc_t record_desc;
fds_find_token_t ftok;


static thread_coap_utils_light_command_t m_command =
    THREAD_COAP_UTILS_LIGHT_CMD_OFF; /**< This variable stores command that has been most recently
                                        used. */
void test2(void)
{
    /* It is required to zero the token before first use. */
    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    /* Loop until all records with the given key and file ID have been found. */
    while (fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok) == NRF_SUCCESS)
    {
        if (fds_record_open(&record_desc, &flash_record) != NRF_SUCCESS)
        {
            /* Handle error. */
        }
        /* Access the record through the flash_record structure. */
        NRF_LOG_INFO("Item: %s", flash_record.p_data);
        /* Close the record when done. */
        if (fds_record_close(&record_desc) != NRF_SUCCESS)
        {
            /* Handle error. */
        }
        
    }
}

void test(void)
{
    // Set up record.
    record.file_id = FILE_ID;
    record.key = RECORD_KEY_1;
    record.data.p_data = &m_deadbeef;
    record.data.length_words = 1; /* one word is four bytes. */
    ret_code_t rc;
    rc = fds_record_write(&record_desc, &record);
    if (rc != NRF_SUCCESS)
    {
        NRF_LOG_INFO("Error: 0x%x", rc);
        NRF_LOG_INFO("write fail");
    }
    // Set up record.
    record.file_id = FILE_ID;
    record.key = RECORD_KEY_2;
    record.data.p_data = &m_hello;
    /* The following calculation takes into account any eventual remainder of the division. */
    record.data.length_words = (sizeof(m_hello) + 3) / 4;
    rc = fds_record_write(&record_desc, &record);
    if (rc != NRF_SUCCESS)
    {
        NRF_LOG_INFO("Error: 0x%x", rc);
        NRF_LOG_INFO("write fail");
    }
    NRF_LOG_INFO("write success");
}


/***************************************************************************************************
 * @section Buttons
 **************************************************************************************************/

static void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_0:
            NRF_LOG_INFO("Button short");
            test();
            break;

        case BSP_EVENT_KEY_0_LONG:
            NRF_LOG_INFO("Button long");
            test2();
            break;

        default:
            return; // no implementation needed
    }
}

/***************************************************************************************************
 * @section Callbacks
 **************************************************************************************************/

static void thread_state_changed_callback(uint32_t flags, void * p_context)
{
    if (flags & OT_CHANGED_THREAD_ROLE)
    {
        switch (otThreadGetDeviceRole(p_context))
        {
            case OT_DEVICE_ROLE_CHILD:
            case OT_DEVICE_ROLE_ROUTER:
            case OT_DEVICE_ROLE_LEADER:
                break;

            case OT_DEVICE_ROLE_DISABLED:
            case OT_DEVICE_ROLE_DETACHED:
            default:
                thread_coap_utils_peer_addr_clear();
                break;
        }
    }

    NRF_LOG_INFO("State changed! Flags: 0x%08x Current role: %d\r\n", flags,
        otThreadGetDeviceRole(p_context));
}

// Simple event handler to handle errors during initialization.
static void fds_evt_handler(fds_evt_t const * p_fds_evt)
{
    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result == NRF_SUCCESS)
            {
                m_fds_initialized = true;
            }
            break;
        default:
            break;
    }
}

/***************************************************************************************************
 * @section Initialization
 **************************************************************************************************/

/**@brief Function for initializing the Thread Board Support Package
 */
static void thread_bsp_init(void)
{
    uint32_t error_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(error_code);

    error_code = bsp_thread_init(thread_ot_instance_get());
    APP_ERROR_CHECK(error_code);
}

/**@brief Function for initializing the Application Timer Module
 */
static void timer_init(void)
{
    uint32_t error_code = app_timer_init();
    APP_ERROR_CHECK(error_code);
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing the Thread Stack
 */
static void thread_instance_init(void)
{
    thread_configuration_t thread_configuration = {
        .radio_mode = THREAD_RADIO_MODE_RX_ON_WHEN_IDLE,
        .autocommissioning = true,
    };

    thread_init(&thread_configuration);
    thread_state_changed_callback_set(thread_state_changed_callback);
}

/**@brief Function for initializing scheduler module.
 */
static void scheduler_init(void) { APP_SCHED_INIT(SCHED_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE); }


/**@brief   Wait for fds to initialize. */
static void wait_for_fds_ready(void)
{
    while (!m_fds_initialized)
    {
        __WFE();
    }
}

/***************************************************************************************************
 * @section Main
 **************************************************************************************************/

int main(int argc, char * argv[])
{
    log_init();
    scheduler_init();
    timer_init();

    NRF_LOG_INFO("Start APP");

    ret_code_t ret = fds_register(fds_evt_handler);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("FDS Callback register failed");
    }
    ret = fds_init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("Error: 0x%x", ret);
        NRF_LOG_INFO("FDS Init failed");
        APP_ERROR_CHECK(ret);
    }

    thread_instance_init();
    thread_bsp_init();

        


    /* Wait for fds to initialize. */
    //wait_for_fds_ready();


    while (true)
    {
        thread_process();
        app_sched_execute();

        if (NRF_LOG_PROCESS() == false)
        {
            thread_sleep();
        }
    }
}

/**
 *@}
 **/