#include "bm_statemachine.h"

#include "app_timer.h"
#include "boards.h"
#include "bm_coap.h"

#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "nrf_log_default_backends.h"

#include <openthread/network_time.h>
#include <openthread/random_noncrypto.h>

#define NUMBER_OF_NETWORK_TIME_ELEMENTS 1000
#define NUMBER_OF_SENDING_MESSAGES 10

APP_TIMER_DEF(m_result_timer);
APP_TIMER_DEF(m_msg_1_timer);
APP_TIMER_DEF(m_msg_2_timer);
APP_TIMER_DEF(m_msg_3_timer);
APP_TIMER_DEF(m_msg_4_timer);
APP_TIMER_DEF(m_msg_5_timer);
APP_TIMER_DEF(m_msg_6_timer);
APP_TIMER_DEF(m_msg_7_timer);
APP_TIMER_DEF(m_msg_8_timer);
APP_TIMER_DEF(m_msg_9_timer);
APP_TIMER_DEF(m_msg_10_timer);

bm_message_info message_info[NUMBER_OF_NETWORK_TIME_ELEMENTS] = {0};

uint16_t bm_message_info_nr = 0;

uint32_t  bm_time = 0;
uint8_t   bm_new_state = 0;
uint8_t   bm_actual_state = 0;
uint8_t   data_size = 1;
uint8_t   s_timer = 0;

/***************************************************************************************************
 * @section State machine - Functions
 **************************************************************************************************/
void bm_save_message_info(bm_message_info message)
{
    message_info[bm_message_info_nr] = message;
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

void bm_set_data_size(uint8_t size)
{
    data_size = size;
    (data_size == 2) ?  bsp_board_led_on(BSP_BOARD_LED_3) : bsp_board_led_off(BSP_BOARD_LED_3);
}

static void start_timer(void)
{
    uint32_t error;
    uint16_t ticks_array[10];

    for (int i=0; i<10; i++)
    {
        ticks_array[i] = otRandomNonCryptoGetUint16InRange(1000, bm_time-1000);
    }
    
    error = app_timer_start(m_msg_1_timer, APP_TIMER_TICKS(ticks_array[0]), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_msg_2_timer, APP_TIMER_TICKS(ticks_array[1]), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_msg_3_timer, APP_TIMER_TICKS(ticks_array[2]), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_msg_4_timer, APP_TIMER_TICKS(ticks_array[3]), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_msg_5_timer, APP_TIMER_TICKS(ticks_array[4]), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_msg_6_timer, APP_TIMER_TICKS(ticks_array[5]), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_msg_7_timer, APP_TIMER_TICKS(ticks_array[6]), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_msg_8_timer, APP_TIMER_TICKS(ticks_array[7]), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_msg_9_timer, APP_TIMER_TICKS(ticks_array[8]), NULL);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_start(m_msg_10_timer, APP_TIMER_TICKS(ticks_array[9]), NULL);
    ASSERT(error == NRF_SUCCESS);
}

static void stop_timer(void)
{
    uint32_t error;
    error = app_timer_stop(m_msg_1_timer);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_stop(m_msg_2_timer);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_stop(m_msg_3_timer);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_stop(m_msg_4_timer);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_stop(m_msg_5_timer);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_stop(m_msg_6_timer);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_stop(m_msg_7_timer);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_stop(m_msg_8_timer);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_stop(m_msg_9_timer);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_stop(m_msg_10_timer);
    ASSERT(error == NRF_SUCCESS);    
}

/***************************************************************************************************
 * @section State machine - Timer handlers
 **************************************************************************************************/
static void m_msg_1_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_2_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_3_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_4_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_5_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_6_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_7_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_8_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_9_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_10_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
    bm_coap_probe_message_send(data_size);
}

static void result_handler(void * p_context)
{
    bm_message_info_nr--;
    bm_coap_results_send(message_info[bm_message_info_nr]);
    bm_new_state = BM_STATE_2;
}

/***************************************************************************************************
 * @section State machine
 **************************************************************************************************/
static void state_1(void)
{
    start_timer();       
    bm_new_state = BM_EMPTY_STATE;
}

static void state_2(void)
{
    uint32_t error;
    if(bm_message_info_nr == 0)
    {
        bsp_board_led_off(BSP_BOARD_LED_2);
        bm_message_info_nr = 0;
        memset(message_info, 0, sizeof(message_info));

        bm_new_state = BM_EMPTY_STATE;
    } else
    {
        error = app_timer_start(m_result_timer, APP_TIMER_TICKS(otRandomNonCryptoGetUint16InRange(20, 30)), NULL);
        ASSERT(error == NRF_SUCCESS);

        bm_new_state = BM_EMPTY_STATE;
    }
}

static void state_stop(void)
{
    stop_timer();
    bsp_board_led_off(BSP_BOARD_LED_2);

    bm_message_info_nr = 0;
    memset(message_info, 0, sizeof(message_info));

    bm_new_state = BM_EMPTY_STATE;
}

void bm_sm_process(void)
{
    bm_actual_state = bm_new_state;

    switch(bm_actual_state)
    {        
        case BM_STATE_1:
            state_1();
            break;

        case BM_STATE_2:
            state_2();
            break;

        case BM_STATE_STOP:
            state_stop();
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

    error = app_timer_create(&m_result_timer, APP_TIMER_MODE_SINGLE_SHOT, result_handler);
    ASSERT(error == NRF_SUCCESS);

    error = app_timer_create(&m_msg_1_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_1_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_msg_2_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_2_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_msg_3_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_3_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_msg_4_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_4_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_msg_5_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_5_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_msg_6_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_6_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_msg_7_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_7_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_msg_8_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_8_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_msg_9_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_9_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_msg_10_timer, APP_TIMER_MODE_SINGLE_SHOT, m_msg_10_handler);
    ASSERT(error == NRF_SUCCESS);
}
