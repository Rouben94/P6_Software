#include "bm_statemachine.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_timer.h"
#include "bm_coap.h"
#include "bm_master_cli.h"
#include "boards.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include <openthread/cli.h>
#include <openthread/network_time.h>
#include <openthread/random_noncrypto.h>

#define NUMBER_OF_NETWORK_TIME_ELEMENTS 1000
#define NUMBER_OF_NODES 60
#define BM_START_DELAY 10000000
#define BM_SECONDS 1000000

APP_TIMER_DEF(m_request_timer);
APP_TIMER_DEF(m_probe_timer);
APP_TIMER_DEF(m_result_timer);

bm_message_info message_info[NUMBER_OF_NETWORK_TIME_ELEMENTS] = {0};
uint32_t bm_time_stamps[NUMBER_OF_NETWORK_TIME_ELEMENTS];
bm_message_info result;

bm_master_message * bm_start_params;
otInstance * p_instance;

static uint32_t LSB_MAC_Address[52] = {0xFDBD, 0xE5BF, 0xC5A8, 0x0972, 0x3CE7, 0x4711, 0xD6F4,
    0xEB69, 0xCD5B, 0x1458, 0x0C96, 0x1E13, 0x3E8E, 0x3E8A, 0xE907, 0xD775, 0x7E84, 0x5B66, 0x496D,
    0x29D5, 0xBF27, 0x87C1, 0x7CC4, 0xA564, 0x7935, 0x4551, 0xFADA, 0xAD40, 0xC37B, 0xD38A, 0xCC89,
    0x263F, 0x8236, 0xDCA0, 0x3C69, 0x14CB, 0xBBA6, 0xF1BB, 0x0FAF, 0xCCB8, 0x30B7, 0x9ED9, 0x50C1,
    0xD85A, 0x7965, 0xA00B, 0xAF03, 0x2713, 0x8169, 0x9D49, 0x7795, 0x067B};
uint8_t BM_NODE_ID;

uint64_t last_net_time_seen = 0xffffffffffffff;
uint16_t bm_message_info_nr = 0;
uint16_t payload = 0;
uint8_t bm_new_state = 0;
uint8_t bm_actual_state = 0;
uint8_t ack_received = 1;
bool additional_payload = false;

/***************************************************************************************************
 * @section State machine - External Functions
 **************************************************************************************************/

void bm_save_message_info(bm_message_info message)
{
    message_info[bm_message_info_nr] = message;
    bm_message_info_nr++;
}

void bm_save_result(bm_message_info message[], uint16_t size)
{
    if (last_net_time_seen != message[0].net_time)
    {
        for (int i = 0; i < size; i++)
        {
            if (message[i].net_time != 0)
            {
                bm_cli_write_result(message[i]);
            }
        }
    }
}

void bm_start_param_set(bm_master_message * start_param) { bm_start_params = start_param; }

void bm_sm_new_state_set(uint8_t state) { bm_new_state = state; }

void bm_set_additional_payload(bool state)
{
    additional_payload = state;
    (state) ? bsp_board_led_on(BSP_BOARD_LED_1) : bsp_board_led_off(BSP_BOARD_LED_1);
}

void bm_set_ack_received(uint8_t received) { ack_received = received; }

uint8_t bm_get_node_id(void) { return BM_NODE_ID; }

/***************************************************************************************************
 * @section State machine - Internal Functions
 **************************************************************************************************/
static int compare(const void * a, const void * b) { return (*(int *)a - *(int *)b); }

static void m_request_handler(void * p_context) { bm_coap_multicast_start_send(bm_start_params); }

static void m_probe_handler(void * p_context)
{
    if (bm_start_params->bm_msg_size <= 3)
    {
        payload = 3;
    }
    else if (bm_start_params->bm_msg_size >= 1024)
    {
        payload = 1024;
    }
    else
    {
        payload = bm_start_params->bm_msg_size;
    }

    if (!additional_payload)
    {
        bm_coap_probe_message_send(3);
    }
    else if (additional_payload)
    {
        bm_coap_probe_message_send(payload);
    }

    bsp_board_led_invert(BSP_BOARD_LED_3);
}

static void m_result_handler(void * p_context) { bm_coap_results_send(&result); }

static void wait_for_event(uint64_t end_time)
{
    uint64_t netTime = 0;
    end_time += bm_start_params->bm_master_time_stamp;

    do
    {
        otNetworkTimeGet(p_instance, &netTime);
        if (end_time < netTime)
        {
            break;
        }

    } while (true);
}

/***************************************************************************************************
 * @section State machine Slave
 **************************************************************************************************/
static void state_1_server(void)
{
    wait_for_event(BM_START_DELAY);
    bm_new_state = BM_STATE_2_SERVER;
}

static void state_2_server(void)
{
    NRF_LOG_INFO("Slave: Start Benchmark");
    bsp_board_led_on(BSP_BOARD_LED_2);

    wait_for_event(BM_START_DELAY + bm_start_params->bm_time * BM_SECONDS);
    bm_new_state = BM_STATE_3_SLAVE;
}

static void state_1_client(void)
{
    memset(bm_time_stamps, 0, NUMBER_OF_NETWORK_TIME_ELEMENTS * sizeof(*bm_time_stamps));
    for (int i = 0; i < bm_start_params->bm_nbr_of_msg; i++)
    {
        bm_time_stamps[i] =
            otRandomNonCryptoGetUint32InRange(10, (bm_start_params->bm_time * 1000) - 10);
    }
    qsort(bm_time_stamps, bm_start_params->bm_nbr_of_msg, sizeof(uint32_t), compare);
    wait_for_event(BM_START_DELAY);
    bm_new_state = BM_STATE_2_CLIENT;
}

static void state_2_client(void)
{
    NRF_LOG_INFO("Slave: Start Benchmark");
    bsp_board_led_on(BSP_BOARD_LED_2);
    uint32_t error;

    for (int i = 0; i < bm_start_params->bm_nbr_of_msg; i++)
    {
        wait_for_event(BM_START_DELAY + bm_time_stamps[i] * 1000);
        error = app_timer_start(m_probe_timer, 5, NULL);
        ASSERT(error == NRF_SUCCESS);
    }

    wait_for_event(BM_START_DELAY + bm_start_params->bm_time * BM_SECONDS);
    bm_new_state = BM_STATE_3_SLAVE;
}

static void state_3_slave(void)
{
    NRF_LOG_INFO("Slave: End Benchmark");
    bsp_board_led_off(BSP_BOARD_LED_2);
    bsp_board_led_off(BSP_BOARD_LED_3);
    wait_for_event(BM_START_DELAY + bm_start_params->bm_time * BM_SECONDS +
                   otRandomNonCryptoGetUint32InRange(1 * BM_SECONDS, 120 * BM_SECONDS));

    bm_new_state = BM_STATE_4_SLAVE;
}

static void state_4_slave(void)
{
    NRF_LOG_INFO("Slave: Send Results");
    bsp_board_led_on(BSP_BOARD_LED_3);
    uint32_t error;
    error = app_timer_start(m_result_timer, 5, NULL);
    ASSERT(error == NRF_SUCCESS);

    do
    {
        if (bm_message_info_nr == 0 && ack_received == 1)
        {
            break;
        }
        if (ack_received == 1)
        {
            bm_message_info_nr--;
            result = message_info[bm_message_info_nr];
            error = app_timer_start(m_result_timer, 5, NULL);
            ASSERT(error == NRF_SUCCESS);
            ack_received = 0;
        }
        else if (ack_received == 2)
        {
            result = message_info[bm_message_info_nr];
            error = app_timer_start(m_result_timer, 5, NULL);
            ASSERT(error == NRF_SUCCESS);
            ack_received = 0;
        }
    } while (true);

    bm_message_info_nr = 0;
    memset(message_info, 0, sizeof(message_info));
    bsp_board_led_off(BSP_BOARD_LED_3);

    bm_new_state = BM_EMPTY_STATE;
}

/***************************************************************************************************
 * @section State machine Master
 **************************************************************************************************/
static void state_1_master(void)
{
    uint32_t error;

    for (int i = 1; i < 9; i++)
    {
        error = app_timer_start(m_request_timer, 5, NULL);
        ASSERT(error == NRF_SUCCESS)
        wait_for_event(BM_SECONDS * i);
    }

    wait_for_event(BM_START_DELAY);
    bm_new_state = BM_STATE_2_MASTER;
}

static void state_2_master(void)
{
    otCliOutput("<BENCHMARK_START> \r\n", sizeof("<BENCHMARK_START> \r\n"));
    wait_for_event(BM_START_DELAY + bm_start_params->bm_time * BM_SECONDS);
    bm_new_state = BM_STATE_3_MASTER;
}

static void state_3_master(void)
{
    otCliOutput("<BENCHMARK_END> \r\n", sizeof("<BENCHMARK_END> \r\n"));
    bm_new_state = BM_EMPTY_STATE;
}

/***************************************************************************************************
 * @section State machine Process
 **************************************************************************************************/
void bm_sm_process(void)
{
    bm_actual_state = bm_new_state;

    switch (bm_actual_state)
    {
        case BM_STATE_1_CLIENT:
            state_1_client();
            break;

        case BM_STATE_2_CLIENT:
            state_2_client();
            break;

        case BM_STATE_1_SERVER:
            state_1_server();
            break;

        case BM_STATE_2_SERVER:
            state_2_server();
            break;

        case BM_STATE_3_SLAVE:
            state_3_slave();
            break;

        case BM_STATE_4_SLAVE:
            state_4_slave();
            break;

        case BM_STATE_1_MASTER:
            state_1_master();
            break;

        case BM_STATE_2_MASTER:
            state_2_master();
            break;

        case BM_STATE_3_MASTER:
            state_3_master();
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
    p_instance = thread_ot_instance_get();
    uint32_t error;
    error = app_timer_create(&m_request_timer, APP_TIMER_MODE_SINGLE_SHOT, m_request_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_probe_timer, APP_TIMER_MODE_SINGLE_SHOT, m_probe_handler);
    ASSERT(error == NRF_SUCCESS);
    error = app_timer_create(&m_result_timer, APP_TIMER_MODE_SINGLE_SHOT, m_result_handler);
    ASSERT(error == NRF_SUCCESS);

    for (int i = 0; i < 52; i++)
    {
        if (LSB_MAC_Address[i] == NRF_FICR->DEVICEADDR[0])
        {
            BM_NODE_ID = i + 1;
        }
    }
}