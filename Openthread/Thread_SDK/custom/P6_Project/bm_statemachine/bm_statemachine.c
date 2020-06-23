#include "bm_statemachine.h"

#include "app_timer.h"
#include "boards.h"
#include "bm_coap.h"

#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "nrf_log_default_backends.h"

#include <openthread/network_time.h>

#define NUMBER_OF_NETWORK_TIME_ELEMENTS 512

APP_TIMER_DEF(m_benchmark_timer);
APP_TIMER_DEF(m_led_timer);

struct bm_message_info
{
    uint64_t net_time;
    uint16_t message_id;
} bm_message_info[NUMBER_OF_NETWORK_TIME_ELEMENTS];

uint16_t bm_message_info_nr = 0;

uint32_t  bm_time = 0;
uint8_t   bm_new_state = 0;
uint8_t   bm_actual_state = 0;

/***************************************************************************************************
 * @section State machine - Functions
 **************************************************************************************************/
void bm_save_message_info(uint16_t id)
{
    uint64_t time;
    otNetworkTimeGet(thread_ot_instance_get(), &time);
    bm_message_info[bm_message_info_nr].net_time = time;
    bm_message_info[bm_message_info_nr].message_id = id;
    bm_message_info_nr++;
}

void bm_sm_time_set(uint32_t time)
{
    bm_time = time;
}

void bm_sm_new_state_set(uint8_t state)
{
    bm_new_state = state;
}

/***************************************************************************************************
 * @section State machine - Timer handlers
 **************************************************************************************************/
static void led_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_unicast_test_message_send(1);
}

static void benchmark_handler(void * p_context)
{
    bm_new_state = BM_STATE_2;
}

/***************************************************************************************************
 * @section State machine
 **************************************************************************************************/
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

    uint32_t error;

    error = app_timer_start(m_led_timer, APP_TIMER_TICKS(500), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_benchmark_timer, APP_TIMER_TICKS(bm_time), NULL);
    ASSERT(error == NRF_SUCCESS);
    bm_new_state = BM_EMPTY_STATE;
}

static void state_2(void)
{
    NRF_LOG_INFO("state two done");

    for (int i=0; i<bm_message_info_nr; i++)
    {
        //NRF_LOG_INFO("Time %d: %d", i+1, bm_netTime[i]);
    }
    bm_message_info_nr = 0;
    memset(bm_message_info, 0, NUMBER_OF_NETWORK_TIME_ELEMENTS * sizeof(bm_message_info));
    bm_coap_unicast_test_message_send(0);
    bm_new_state = BM_DEFAULT_STATE;
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

        case BM_STATE_2:
            state_2();
            break;

        case BM_EMPTY_STATE:
            break;
        
        default:
            break;
    }
}

/***************************************************************************************************
 * @section State machine - Init
 **************************************************************************************************/
void bm_statemachine_init(void)
{
    uint32_t error;

    error = app_timer_create(&m_led_timer, APP_TIMER_MODE_REPEATED, led_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_benchmark_timer, APP_TIMER_MODE_REPEATED, benchmark_handler);
    ASSERT(error == NRF_SUCCESS);
}
