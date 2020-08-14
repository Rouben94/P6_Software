#include "bm_statemachine.h"

#include "app_timer.h"
#include "boards.h"
#include "bm_coap.h"
#include "bm_master_cli.h"

#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "nrf_log_default_backends.h"

#include <openthread/network_time.h>
#include <openthread/random_noncrypto.h>
#include <openthread/cli.h>

#define NUMBER_OF_NETWORK_TIME_ELEMENTS 3000
#define NUMBER_OF_NODES 60

APP_TIMER_DEF(m_benchmark_timer);
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
bm_message_info result[NUMBER_OF_NODES][NUMBER_OF_NETWORK_TIME_ELEMENTS] = {0};

uint16_t bm_message_info_nr = 0;
uint16_t bm_slave_nr = 0;

otIp6Address bm_slave_address[NUMBER_OF_NODES] = {};
uint32_t  bm_time = 0;
uint64_t  last_net_time_seen = 0xffffffffffffff;
uint8_t   bm_new_state = 0;
uint8_t   bm_actual_state = 0;
uint8_t   data_size = 1;

bool      bm_stop = true;

/***************************************************************************************************
 * @section State machine - Functions
 **************************************************************************************************/
void bm_reset_slave_address(void)
{
    bm_slave_nr = 0;
    memset(bm_slave_address, 0, sizeof(bm_slave_address));
}

void bm_save_message_info(bm_message_info message)
{
    message_info[bm_message_info_nr] = message;
    bm_message_info_nr++;
}

void bm_save_result(bm_message_info message[], uint16_t size)
{
    if(last_net_time_seen != message[0].net_time)
    {
        for(int i=0; i<size; i++)
        {
            if(message[i].net_time != 0)
            {
                bm_cli_write_result(message[i]);
            }
        }

        app_timer_stop(m_result_timer);
        bm_sm_new_state_set(BM_STATE_2_MASTER);
    }  
}

void bm_save_slave_address(otIp6Address slave_address)
{
    bm_slave_address[bm_slave_nr] = slave_address;
    bm_slave_nr++;
}

void bm_stop_set(bool state)
{
    bm_stop = state;
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
        ticks_array[i] = otRandomNonCryptoGetUint16InRange(4000, bm_time);
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

/***************************************************************************************************
 * @section State machine - Timer handlers
 **************************************************************************************************/
static void m_msg_1_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_2_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_3_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_4_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_5_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_6_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_7_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_8_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_9_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_msg_10_handler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
    bm_coap_probe_message_send(data_size);
}

static void m_benchmark_handler(void * p_context)
{
    bm_new_state = BM_STATE_2_MASTER;
}

static void m_result_handler(void * p_context)
{
    bm_new_state = BM_STATE_2_MASTER;
}

/***************************************************************************************************
 * @section State machine
 **************************************************************************************************/
static void state_1_slave(void)
{
    start_timer();
    bm_new_state = BM_EMPTY_STATE;
}

static void state_2_slave(void)
{
    uint32_t error;
    if(bm_message_info_nr==0)
    {
        bm_message_info bm_result_struct[1];
        memset(bm_result_struct, 0, sizeof(bm_result_struct));
        bm_coap_results_send(bm_result_struct, sizeof(bm_result_struct));
    } else
    {
        bm_message_info bm_result_struct[bm_message_info_nr];
    
        for(int i=0; i<bm_message_info_nr; i++)
        {
            bm_result_struct[i] = message_info[i];
        }
        bm_coap_results_send(bm_result_struct, sizeof(bm_result_struct));
    }

    bm_new_state = BM_EMPTY_STATE;
}

static void state_3_slave(void)
{
    bsp_board_led_off(BSP_BOARD_LED_2);
    bsp_board_led_off(BSP_BOARD_LED_3);
    bm_message_info_nr = 0;
    memset(message_info, 0, sizeof(message_info));

    bm_new_state = BM_EMPTY_STATE;
}

static void state_1_master(void)
{    
    uint32_t error;
    error = app_timer_start(m_benchmark_timer, APP_TIMER_TICKS(bm_time+5000), NULL);
    ASSERT(error == NRF_SUCCESS);

    bm_new_state = BM_EMPTY_STATE;
}

static void state_2_master(void)
{   
    uint32_t error;

    if (bm_slave_nr == 0)
    {
        otCliOutput("<REPORT_END> \r\n", sizeof("<REPORT_END> \r\n"));
    }

    if (bm_slave_nr > 0)
    {
        bm_slave_nr--;
        bm_coap_result_request_send(bm_slave_address[bm_slave_nr]);
        error = app_timer_start(m_result_timer, APP_TIMER_TICKS(3000), NULL);
        ASSERT(error == NRF_SUCCESS);
    }
    
    bm_new_state = BM_EMPTY_STATE;
}

void bm_sm_process(void)
{
    bm_actual_state = bm_new_state;

    switch(bm_actual_state)
    {        
        case BM_STATE_1_SLAVE:
            state_1_slave();
            break;

        case BM_STATE_2_SLAVE:
            state_2_slave();
            break;

        case BM_STATE_3_SLAVE:
            state_3_slave();
            break;

        case BM_STATE_1_MASTER:
            state_1_master();
            break;

        case BM_STATE_2_MASTER:
            state_2_master();
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

    error = app_timer_create(&m_benchmark_timer, APP_TIMER_MODE_SINGLE_SHOT, m_benchmark_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_result_timer, APP_TIMER_MODE_SINGLE_SHOT, m_result_handler);
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
