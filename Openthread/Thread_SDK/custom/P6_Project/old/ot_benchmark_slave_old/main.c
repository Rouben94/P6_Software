/** @file
 *
 * @defgroup ot_benchmark main.c
 * @{
 * @ingroup ot_benchmark
 * @brief OpenThread benchmark Application main file.
 *
 * @details This application tests a openthread network.
 *
 */

#include "app_scheduler.h"
#include "app_timer.h"
#include "board_support_thread.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

//#include "flash_save.h"

#include "bm_coap.h"
#include "thread_utils.h"

#include "board_support_config.h"

#include <openthread/instance.h>
#include <openthread/network_time.h>
#include <openthread/platform/time.h>
#include <openthread/thread.h>

#define SCHED_QUEUE_SIZE 32 /**< Maximum number of events in the scheduler queue. */
#define SCHED_EVENT_DATA_SIZE                                                                      \
    APP_TIMER_SCHED_EVENT_DATA_SIZE /**< Maximum app_scheduler event size. */


//Measurement measure_1;

static thread_coap_utils_light_command_t m_command =
    THREAD_COAP_UTILS_LIGHT_CMD_OFF; /**< This variable stores command that has been most recently
                                        used. */


/***************************************************************************************************
 * @section Buttons
 **************************************************************************************************/

static void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_0:
            NRF_LOG_INFO("Button short");
            //flash_write(measure_1);
            //measure_1.MAC++;
            break;

        case BSP_EVENT_KEY_0_LONG:
            NRF_LOG_INFO("Button long");
            //flash_read();
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

//static void my_event_cb(Measurement * data) { NRF_LOG_INFO("Item: %d", data->MAC); }

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


/***************************************************************************************************
 * @section Main
 **************************************************************************************************/

int main(int argc, char * argv[])
{
    log_init();
    scheduler_init();
    timer_init();

    NRF_LOG_INFO("Start APP");

    //flash_save_init(my_event_cb);
    thread_instance_init();
    thread_bsp_init();

    //measure_1.MAC = 0;

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