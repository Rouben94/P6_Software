#include "bm_statemachine.h"

#include "app_timer.h"
#include "boards.h"
#include "bm_coap.h"

#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "nrf_log_default_backends.h"

#include <openthread/network_time.h>

APP_TIMER_DEF(m_benchmark_timer);
APP_TIMER_DEF(m_led_timer);

uint16_t bm_netTime_nr = 0;
uint64_t bm_netTime[1000];

uint32_t  bm_time = 0;
uint8_t   bm_new_state = 0;
uint8_t   bm_actual_state = 0;

static void save_netTime(uint64_t time)
{
    bm_netTime[bm_netTime_nr] = time;
    bm_netTime_nr++;
}


static void led_handler(void * p_context)
{
    uint64_t time;
    bsp_board_led_invert(BSP_BOARD_LED_2);

    otNetworkTimeGet(thread_ot_instance_get(), &time);
    save_netTime(time);

    bm_coap_unicast_test_message_send(1);
}

static void benchmark_handler(void * p_context)
{
    for (int i=0; i<bm_netTime_nr; i++)
    {
        NRF_LOG_INFO("Time %d: %d", i+1, bm_netTime[i]);
    }
    bm_netTime_nr = 0;
    memset(bm_netTime, 0, 1000 * sizeof(uint64_t));
    bm_new_state = BM_DEFAULT_STATE;
}

void bm_sm_time_set(uint32_t time)
{
    bm_time = time;
}

void bm_sm_new_state_set(uint8_t state)
{
    bm_new_state = state;
}

static void default_state(void)
{
    uint32_t error;
    error = app_timer_stop(m_led_timer);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_stop(m_benchmark_timer);
    ASSERT(error == NRF_SUCCESS);
    bsp_board_led_off(BSP_BOARD_LED_2);
}

static void state_1(void)
{
    NRF_LOG_INFO("state one done");
    NRF_LOG_INFO("Benchmark time is: %d", bm_time);

    uint32_t error;

    error = app_timer_start(m_led_timer, APP_TIMER_TICKS(500), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_benchmark_timer, APP_TIMER_TICKS(bm_time), NULL);
    ASSERT(error == NRF_SUCCESS);
    bm_new_state = BM_EMPTY_STATE;
}

void bm_statemachine_empty(void)
{
    
}

void bm_statemachine_init(void)
{
    uint32_t error;

    error = app_timer_create(&m_led_timer, APP_TIMER_MODE_REPEATED, led_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_benchmark_timer, APP_TIMER_MODE_REPEATED, benchmark_handler);
    ASSERT(error == NRF_SUCCESS);
}

void bm_sm_process(void)
{
    bm_actual_state = bm_new_state;

    switch(bm_actual_state)
    {
        case BM_DEFAULT_STATE:
            default_state();
            break;
        
        case BM_STATE_1:
            state_1();
            break;

        case BM_EMPTY_STATE:
            bm_statemachine_empty();
            break;
        
        default:
            bm_statemachine_empty();
            break;
    }
}
