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
#include "bm_board_support_thread.h"
#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "nrf_log_default_backends.h"

#include "bm_coap.h"
#include "thread_utils.h"
#include "bm_statemachine.h"
#include "bm_board_support_config.h"

#include <openthread/thread.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/network_time.h>
#include <openthread/platform/time.h>


#define SCHED_QUEUE_SIZE 32                                   /**< Maximum number of events in the scheduler queue. */
#define SCHED_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE /**< Maximum app_scheduler event size. */

#define BM_CLIENT
//#define BM_SERVER

bool toggle_data_size = true;


/***************************************************************************************************
 * @section Buttons
 **************************************************************************************************/

static void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_0:
            NRF_LOG_INFO("Button short press");
            bm_increment_group_address();
            break;

        case BSP_EVENT_KEY_0_LONG:
        {
            NRF_LOG_INFO("Button long press");
            bm_decrement_group_address();

#ifdef BM_CLIENT
            if (toggle_data_size)
            {
                bm_set_data_size(BM_1024Bytes);
                toggle_data_size = false;
            } else
            {
                bm_set_data_size(BM_1bit);
                toggle_data_size = true;
            }
#endif BM_CLIENT

            break;
        }

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
                break;
        }
    }

    NRF_LOG_INFO("State changed! Flags: 0x%08x Current role: %d\r\n", flags,
        otThreadGetDeviceRole(p_context));
}

static void thread_time_sync_callback(void * p_context) { NRF_LOG_INFO("time sync callback"); }

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
    thread_configuration_t thread_configuration = 
    {
        .radio_mode = THREAD_RADIO_MODE_RX_ON_WHEN_IDLE,
        .autocommissioning = true,
        .autostart_disable = false
    };

    thread_init(&thread_configuration);
    thread_state_changed_callback_set(thread_state_changed_callback);
}

/**@brief Function for initializing the Constrained Application Protocol Module
 */
static void thread_coap_init(void)
{
#ifdef BM_CLIENT
    thread_coap_utils_configuration_t thread_coap_configuration = {
        .coap_server_enabled = false,
        .coap_client_enabled = true,
    };
#endif

#ifdef BM_SERVER
    thread_coap_utils_configuration_t thread_coap_configuration = {
        .coap_server_enabled = true,
        .coap_client_enabled = false,
    };
#endif

    thread_coap_utils_init(&thread_coap_configuration);
}

/**@brief Function for initializing the Thread Time Synchronizationt Package
 */
static void thread_time_sync_init(void)
{
    otNetworkTimeSetSyncPeriod(thread_ot_instance_get(), 60);
    otNetworkTimeSetXtalThreshold(thread_ot_instance_get(), otPlatTimeGetXtalAccuracy());
    otNetworkTimeSyncSetCallback(thread_ot_instance_get(), thread_time_sync_callback, NULL);
}

/**@brief Function for initializing scheduler module.
 */
static void scheduler_init(void) { APP_SCHED_INIT(SCHED_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE); }

/***************************************************************************************************
 * @section Main
 **************************************************************************************************/

int main(int argc, char * argv[])
{
    log_init();
    scheduler_init();
    timer_init();

    NRF_LOG_INFO("Start APP");

    thread_instance_init();
    thread_coap_init();
    thread_bsp_init();
    thread_time_sync_init();
    bm_statemachine_init();

    while (true)
    {
        thread_process();
        bm_sm_process();
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