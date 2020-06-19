/* Copyright (c) 2010 - 2020, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
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
 */


#include <stdio.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"
// Board PCA10059 not officialy supported
//#define BOARD_PCA10059
#define BOARD_PCA10056
#if defined(BOARD_PCA10056) && defined(BOARD_PCA10059)
#undef BOARD_PCA10056
#endif
#if defined(BOARD_PCA10059)
#define BUTTON_BOARD
#endif

#include "boards.h"
#include "simple_hal.h"

#include "log.h"
#include "mesh_app_utils.h"
#include "app_timer.h"
#include "example_common.h"
#include "nrf_mesh_config_core.h"
#include "nrf_sdh.h"

#include <time_sync.h>
#include "nrf_gpiote.h"
#include "nrf_ppi.h"
#include "nrf_timer.h"

#if defined(NRF51) && defined(NRF_MESH_STACK_DEPTH)
#include "stack_depth.h"
#endif

/*****************************************************************************
 * Definitions
 *****************************************************************************/


/*****************************************************************************
 * Forward declaration of static functions
 *****************************************************************************/


/*****************************************************************************
 * Static variables
 *****************************************************************************/



static void button_event_handler(uint32_t button_number)
{
    /* Increase button number because the buttons on the board is marked with 1 to 4 */
    button_number++;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Button %u pressed\n", button_number);
    uint32_t err_code;
    switch (button_number)
    {

        case 1:
        {
            static bool m_send_sync_pkt = false;
                
                if (m_send_sync_pkt)
                {
                    m_send_sync_pkt = false;
                    
                    hal_led_pin_set(BSP_LED_0, false);
                    
                    err_code = ts_tx_stop();
                    APP_ERROR_CHECK(err_code);
                    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Stopping sync beacon transmission!\r\n");
                }
                else
                {
                    m_send_sync_pkt = true;
                    
                    hal_led_pin_set(BSP_LED_0, true);
                    
                    err_code = ts_tx_start(200);
                    APP_ERROR_CHECK(err_code);
                    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Starting sync beacon transmission!\r\n");
                }
        }

    }
}


static void sync_timer_button_init(void)
{
    uint32_t       err_code;
    uint8_t        rf_address[5] = {0xDE, 0xAD, 0xBE, 0xEF, 0x19};
    ts_params_t    ts_params;
    
    // Debug pin: 
    // nRF52-DK (PCA10040) Toggle P0.24 from sync timer to allow pin measurement
    // nRF52840-DK (PCA10056) Toggle P1.14 from sync timer to allow pin measurement
#if defined(BOARD_PCA10040)
    nrf_gpiote_task_configure(3, NRF_GPIO_PIN_MAP(0, 24), NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
    nrf_gpiote_task_enable(3);
#elif defined(BOARD_PCA10056)
    nrf_gpiote_task_configure(3, NRF_GPIO_PIN_MAP(0, 16), NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW); // LED4 0.16
    nrf_gpiote_task_enable(3);
#elif defined(BOARD_PCA10059)
    nrf_gpiote_task_configure(3, NRF_GPIO_PIN_MAP(0, 12), NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW); // LED Blau 0.12
    nrf_gpiote_task_enable(3);
#else
#warning Debug pin not set
#endif
    
    nrf_ppi_channel_endpoint_setup(
        NRF_PPI_CHANNEL0, 
        (uint32_t) nrf_timer_event_address_get(NRF_TIMER3, NRF_TIMER_EVENT_COMPARE4),
        nrf_gpiote_task_addr_get(NRF_GPIOTE_TASKS_OUT_3));
    nrf_ppi_channel_enable(NRF_PPI_CHANNEL0);
    
    ts_params.high_freq_timer[0] = NRF_TIMER3;
    ts_params.high_freq_timer[1] = NRF_TIMER2;
    ts_params.rtc             = NRF_RTC1;
    ts_params.egu             = NRF_EGU3;
    ts_params.egu_irq_type    = SWI3_EGU3_IRQn;
    ts_params.ppi_chg         = 0;
    ts_params.ppi_chns[0]     = 1;
    ts_params.ppi_chns[1]     = 2;
    ts_params.ppi_chns[2]     = 3;
    ts_params.ppi_chns[3]     = 4;
    ts_params.rf_chn          = 0; /* For testing purposes */
    memcpy(ts_params.rf_addr, rf_address, sizeof(rf_address));
    
    err_code = ts_init(&ts_params);
    APP_ERROR_CHECK(err_code);
    
    err_code = ts_enable();
    APP_ERROR_CHECK(err_code);
    
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Started listening for beacons.\r\n");
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Press Button 1 to start sending sync beacons\r\n");
}

int main(void)
{
    //initialize();
    //start();

    // initialization
    __LOG_INIT(LOG_MSK_DEFAULT | LOG_SRC_DFU | LOG_SRC_APP | LOG_SRC_SERIAL, LOG_LEVEL_INFO, log_callback_rtt);
    app_timer_init();
    hal_leds_init();
#if BUTTON_BOARD
    hal_buttons_init(button_event_handler);
#endif

    sync_timer_button_init();


    for (;;)
    {
        (void)sd_app_evt_wait();
    }
}
