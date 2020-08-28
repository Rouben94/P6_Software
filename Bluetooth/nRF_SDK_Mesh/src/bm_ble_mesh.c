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

#include <stdint.h>
#include <string.h>

#include "bm_config.h"
#include "bm_cli.h"
#include "bm_timesync.h"
#include "bm_simple_buttons_and_leds.h"

/* HAL */
#include "boards.h"
#include "simple_hal.h"
#include "app_timer.h"

/* Core */
#include "nrf_mesh_config_core.h"
#include "nrf_mesh_gatt.h"
#include "nrf_mesh_configure.h"
#include "nrf_mesh.h"
#include "bm_mesh_stack.h"
#include "device_state_manager.h"
#include "access_config.h"

/* Provisioning and configuration */
#include "mesh_provisionee.h"
#include "mesh_app_utils.h"
#include "net_state.h"

/* Models */
#include "bm_generic_onoff_common.h"
#include "bm_generic_onoff_messages.h"
#include "config_server.h"
#include "bm_generic_onoff_client.h"
#include "bm_generic_onoff_server.h"

/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"

/* Example specific includes */
#include "app_config.h"
#include "nrf_mesh_config_examples.h"
#include "light_switch_example_common.h"
#include "example_common.h"
#include "app_onoff.h"
#include "ble_softdevice_support.h"

/*****************************************************************************
 * Definitions
 *****************************************************************************/

#define BENCHMARK_SERVER
#define BENCHMARK_CLIENT

#define ONOFF_SERVER_0_LED          (BSP_LED_0)

#define APP_ONOFF_ELEMENT_INDEX     (0)

#define APP_UNACK_MSG_REPEAT_COUNT   (2)

/* Controls if the model instance should force all mesh messages to be segmented messages. */
#define APP_FORCE_SEGMENTATION       (true)
/* Controls the MIC size used by the model instance for sending the mesh messages. */
#define APP_MIC_SIZE                 (NRF_MESH_TRANSMIC_SIZE_SMALL)


/*****************************************************************************
 * Forward declaration of static functions
 *****************************************************************************/
static void app_gen_onoff_client_publish_interval_cb(access_model_handle_t handle, void * p_self);
static void app_generic_onoff_client_status_cb(const generic_onoff_client_t * p_self,
                                               const access_message_rx_meta_t * p_meta,
                                               const generic_onoff_status_params_t * p_in);
static void app_gen_onoff_client_transaction_status_cb(access_model_handle_t model_handle,
                                                       void * p_args,
                                                       access_reliable_status_t status);
                                                                                           
static void app_gen_onoff_server_state_set_cb(const generic_onoff_server_t * p_self,
                                             const access_message_rx_meta_t * p_meta,
                                             const generic_onoff_set_params_t * p_in,
                                             const model_transition_t * p_in_transition,
                                             generic_onoff_status_params_t * p_out);

static void app_gen_onoff_server_state_get_cb(const generic_onoff_server_t * p_self,
                                             const access_message_rx_meta_t * p_meta,
                                             generic_onoff_status_params_t * p_out);



/*****************************************************************************
 * Static variables
 *****************************************************************************/
static generic_onoff_client_t m_client;
static generic_onoff_server_t m_server;
static bool                   m_device_provisioned;

const generic_onoff_client_callbacks_t client_cbs =
{
    .onoff_status_cb = app_generic_onoff_client_status_cb,
    .ack_transaction_status_cb = app_gen_onoff_client_transaction_status_cb,
    .periodic_publish_cb = app_gen_onoff_client_publish_interval_cb
};

const generic_onoff_server_callbacks_t server_cbs =
{
    .onoff_cbs.set_cb = app_gen_onoff_server_state_set_cb,
    .onoff_cbs.get_cb = app_gen_onoff_server_state_get_cb
};


static void app_gen_onoff_server_state_set_cb(const generic_onoff_server_t * p_self,
                                             const access_message_rx_meta_t * p_meta,
                                             const generic_onoff_set_params_t * p_in,
                                             const model_transition_t * p_in_transition,
                                             generic_onoff_status_params_t * p_out)
{

    bm_cli_log("Setting GPIO value: %d\n", p_in->on_off % 2)

    //hal_led_pin_set(ONOFF_SERVER_0_LED, p_in->on_off % 2);
    bm_led3_set(p_in->on_off % 2);
    

    bm_cli_log("DST Addr: %x\n",p_meta->dst.value);
    bm_cli_log("GRP Addr: %x\n",p_meta->dst.value);
    bm_cli_log("SRC Addr: %x\n",p_meta->src.value);
    bm_cli_log("TID: %d\n",p_in->tid);
    bm_cli_log("TTL: %d\n",p_meta->ttl);
    bm_cli_log("RSSI: %d\n",(int16_t)p_meta->p_core_metadata->params.scanner.rssi);
    bm_cli_log("SIZE: 2\n");

}


static void app_gen_onoff_server_state_get_cb(const generic_onoff_server_t * p_self,
                                             const access_message_rx_meta_t * p_meta,
                                             generic_onoff_status_params_t * p_out)
{

}




/* Acknowledged transaction status callback, if acknowledged transfer fails, application can
* determine suitable course of action (e.g. re-initiate previous transaction) by using this
* callback.
*/
static void app_gen_onoff_client_transaction_status_cb(access_model_handle_t model_handle,
                                                       void * p_args,
                                                       access_reliable_status_t status)
{
}

/* Generic OnOff client model interface: Process the received status message in this callback */
static void app_generic_onoff_client_status_cb(const generic_onoff_client_t * p_self,
                                               const access_message_rx_meta_t * p_meta,
                                               const generic_onoff_status_params_t * p_in)
{
}

/* This callback is called periodically if model is configured for periodic publishing */
static void app_gen_onoff_client_publish_interval_cb(access_model_handle_t handle, void * p_self)
{
}

bool last_state;
static uint16_t tid = 0;

void bm_send_message()
{

    uint32_t status = NRF_SUCCESS;
    generic_onoff_set_params_t set_params;
    model_transition_t transition_params;
    
    
    tid++;
    
    set_params.on_off = (uint8_t) (tid & 0x00FF); // LSB of TID
    set_params.tid = (uint8_t) (tid >> 8); // MSB of TID
    transition_params.delay_ms = 0;
    transition_params.transition_time_ms = 0;
    bm_cli_log("Sending msg: ONOFF SET %d\n", set_params.on_off % 2);
    (void)access_model_reliable_cancel(m_client.model_handle);
    //status = generic_onoff_client_set(&m_client, &set_params, &transition_params);
    status = generic_onoff_client_set_unack(&m_client, &set_params, NULL, APP_UNACK_MSG_REPEAT_COUNT);
    //hal_led_pin_set(BSP_LED_0, set_params.on_off);
    bm_led2_set(set_params.on_off % 2);

    bm_cli_log("DST Addr: bm_params\n");
    bm_cli_log("GRP Addr: bm_params\n");
    bm_cli_log("SRC Addr: addr\n");
    bm_cli_log("TID: %d\n",((uint16_t)set_params.tid << 8) | set_params.on_off);
    bm_cli_log("TTL: %d\n",access_default_ttl_get());
    bm_cli_log("RSSI: 0\n");
    bm_cli_log("SIZE: %d\n",m_client.access_message.message.length); // Static Payload is 2 Bytes + Additional Siize

}




static void models_init_cb(void)
{
      bm_cli_log("Initializing, and adding models\n");
      m_client.settings.p_callbacks = &client_cbs;
      m_client.settings.timeout = 0;
      m_client.settings.force_segmented = APP_FORCE_SEGMENTATION;
      m_client.settings.transmic_size = APP_MIC_SIZE;
      ERROR_CHECK(generic_onoff_client_init(&m_client, 0));

      m_server.settings.p_callbacks = &server_cbs;
      m_server.settings.force_segmented = APP_FORCE_SEGMENTATION;
      m_server.settings.transmic_size = APP_MIC_SIZE;
      ERROR_CHECK(generic_onoff_server_init(&m_server, 0));

      bm_cli_log("Do Self Provisioning\n");
      uint32_t status;
      
      dsm_local_unicast_address_t local_address;
      local_address.address_start = 0x99; // Replace with MAC or Random Value
      local_address.count = ACCESS_ELEMENT_COUNT;
      status = dsm_local_unicast_addresses_set(&local_address);

      dsm_handle_t netkey_handle;
      uint8_t netkey[16]={0x79,0xF5,0xF1,0x6A,0x43,0x22,0x17,0xDF,0x1D,0xC1,0x59,0x10,0x13,0x0A,0x31,0x6F};
      status = dsm_subnet_add(0, netkey, &netkey_handle);
      dsm_handle_t network_handle = dsm_net_key_index_to_subnet_handle(0);
 
      dsm_handle_t appkey_handle;
      uint8_t appkey[16]={0x5B,0xD8,0xDE,0xB4,0x49,0xAB,0x15,0x1B,0x72,0x7F,0xD4,0x56,0xF5,0x49,0x81,0x9B};
      status = dsm_appkey_add(0, network_handle, appkey, &appkey_handle);

      dsm_handle_t devkey_handle;
      uint8_t devkey[16]={0x78,0xF5,0xF1,0x6A,0x43,0x22,0x17,0xDF,0x1D,0xC1,0x59,0x10,0x13,0x0A,0x31,0x6F};
      status = dsm_devkey_add(local_address.address_start, netkey_handle, devkey, &devkey_handle);

      /* Bind config server to the device key */
      status = config_server_bind(devkey_handle);

      NRF_MESH_ERROR_CHECK(net_state_iv_index_set(0,0));

      access_default_ttl_set(7);

      bm_cli_log("Add Subscription and Publish Addresses\n");
      dsm_handle_t subscription_address_handle;
      status = dsm_address_subscription_add(0xC001, &subscription_address_handle);
      dsm_handle_t publish_address_handle;
      status = dsm_address_publish_add(0xC001,&publish_address_handle);
      
      bm_cli_log("Configurate Models\n");
      NRF_MESH_ERROR_CHECK(access_model_application_bind(m_client.model_handle,appkey_handle));
      NRF_MESH_ERROR_CHECK(access_model_publish_application_set(m_client.model_handle,appkey_handle));
      NRF_MESH_ERROR_CHECK(access_model_publish_address_set(m_client.model_handle,publish_address_handle));    

      NRF_MESH_ERROR_CHECK(access_model_application_bind(m_server.model_handle,appkey_handle));
      NRF_MESH_ERROR_CHECK(access_model_subscription_add(m_server.model_handle,subscription_address_handle));  
      
}


static void config_server_evt_cb(const config_server_evt_t * p_evt)
{

}


static void mesh_init(void)
{
    mesh_stack_init_params_t init_params =
    {
        .core.irq_priority       = NRF_MESH_IRQ_PRIORITY_LOWEST,
        .core.lfclksrc           = DEV_BOARD_LF_CLK_CFG,
        .core.p_uuid             = NULL,
        .models.models_init_cb   = models_init_cb,
        .models.config_server_cb = config_server_evt_cb
    };

    uint32_t status = mesh_stack_init(&init_params, &m_device_provisioned);
    switch (status)
    {
        case NRF_ERROR_INVALID_DATA:
            bm_cli_log("Data in the persistent memory was corrupted. Device starts as unprovisioned.\n");
			bm_cli_log("Reset device before start provisioning.\n");
            break;
        case NRF_SUCCESS:
            break;
        default:
            ERROR_CHECK(status);
    }
}

static void bm_ble_mesh_initialize(void)
{


    //__LOG_INIT(LOG_SRC_APP | LOG_SRC_ACCESS | LOG_SRC_BEARER, LOG_LEVEL_INFO, LOG_CALLBACK_DEFAULT);
    bm_cli_log("----- BLE Mesh Light Switch Client Demo -----\n");



    //ERROR_CHECK(app_timer_init());
    //hal_leds_init();

#if BUTTON_BOARD
    //ERROR_CHECK(hal_buttons_init(button_event_handler));
#endif

    ble_stack_init();

#if MESH_FEATURE_GATT_ENABLED
    gap_params_init();
    conn_params_init();
#endif

    mesh_init();
}



static void unicast_address_print(void)
{
    dsm_local_unicast_address_t node_address;
    dsm_local_unicast_addresses_get(&node_address);
    bm_cli_log("Node Address: 0x%04x \n", node_address.address_start);
}


static void provisioning_complete_cb(void)
{
    bm_cli_log("Successfully provisioned\n");

#if MESH_FEATURE_GATT_ENABLED
    /* Restores the application parameters after switching from the Provisioning
     * service to the Proxy  */
    gap_params_init();
    conn_params_init();
#endif

    unicast_address_print();
    //hal_led_blink_stop();
    //hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
    //hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_PROV);
}


static void bm_ble_mesh_start(void)
{    
    if (!m_device_provisioned)
    {      
        static const uint8_t static_auth_data[NRF_MESH_KEY_SIZE] = STATIC_AUTH_DATA;
        mesh_provisionee_start_params_t prov_start_params =
        {
            .p_static_data    = static_auth_data,
            .prov_sd_ble_opt_set_cb = NULL,
            .prov_complete_cb = provisioning_complete_cb,
            .prov_device_identification_start_cb =  NULL,
            .prov_device_identification_stop_cb = NULL,
            .prov_abort_cb = NULL,
            .p_device_uri = EX_URI_LS_CLIENT
        };
        ERROR_CHECK(mesh_provisionee_prov_start(&prov_start_params));
    }
    else
    {
        unicast_address_print();
    }

    provisioning_complete_cb();

    mesh_app_uuid_print(nrf_mesh_configure_device_uuid_get());

    ERROR_CHECK(mesh_stack_start());

    //hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
    //hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_START);
}

void bm_ble_mesh_init()
{
    bm_ble_mesh_initialize();
    bm_ble_mesh_start();    
    sd_clock_hfclk_request();
}

void bm_ble_mesh_deinit()
{
    retval = proxy_stop();
    if (retval == NRF_SUCCESS)
    {
        retval = nrf_mesh_disable();
    }
    sd_softdevice_disable();
}
