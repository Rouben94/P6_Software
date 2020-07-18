/** @file
 *
 * @defgroup ot_benchmark main.c
 * @{
 * @ingroup ot_benchmark
 * @brief OpenThread benchmark Application coap file.
 *
 * @details This files handels the coap settings.
 *
 */

#include "bm_coap.h"

#include "app_timer.h"
#include "bm_board_support_thread.h"
#include "nrf_assert.h"
#include "nrf_log.h"
#include "sdk_config.h"
#include "thread_utils.h"
#include "bm_statemachine.h"
#include "bm_master_cli.h"

#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/random_noncrypto.h>
#include <openthread/network_time.h>

#define NUMBER_OF_IP6_GROUPS 25
#define DATA_SIZE 1024
#define HOP_LIMIT_DEFAULT 64


/**@brief Structure holding CoAP status information. */
typedef struct
{
    bool         provisioning_enabled; /**< Information if provisioning is enabled. */
    uint32_t     provisioning_expiry;  /**< Provisioning timeout time. */
    bool         led_blinking_is_on;   /**< Information if leds are blinking */
    otIp6Address peer_address;         /**< An address of a related server node. */
} state_t;

otIp6Address bm_master_address, bm_group_address;

uint8_t bm_group_nr = 0;
const char *bm_group_address_array[NUMBER_OF_IP6_GROUPS] = {"ff02::3", 
                                                            "ff02::4",
                                                            "ff02::5",
                                                            "ff02::6",
                                                            "ff02::7",
                                                            "ff02::8",
                                                            "ff02::9",
                                                            "ff02::10",
                                                            "ff02::11",
                                                            "ff02::12",
                                                            "ff02::13", 
                                                            "ff02::14",
                                                            "ff02::15",
                                                            "ff02::16",
                                                            "ff02::17",
                                                            "ff02::18",
                                                            "ff02::19",
                                                            "ff02::20",
                                                            "ff02::21",
                                                            "ff02::22",
                                                            "ff02::23",
                                                            "ff02::24",
                                                            "ff02::25",
                                                            "ff02::26",
                                                            "ff02::27"};

static thread_coap_utils_configuration_t          m_config;

/**@brief Forward declarations of CoAP resources handlers. */
static void bm_start_handler(void *, otMessage *, const otMessageInfo *);
static void bm_probe_message_handler(void *, otMessage *, const otMessageInfo *);
static void bm_result_handler(void *, otMessage *, const otMessageInfo *);

/**@brief Definition of CoAP resources for benchmark start. */
static otCoapResource m_bm_start_resource =
{
    .mUriPath = "bm_start",
    .mHandler = bm_start_handler,
    .mContext = NULL,
    .mNext    = NULL
};

/**@brief Definition of CoAP resources for benchmark test message. */
static otCoapResource m_bm_test_resource =
{
    .mUriPath = "bm_test",
    .mHandler = bm_probe_message_handler,
    .mContext = NULL,
    .mNext    = NULL
};

/**@brief Definition of CoAP resources for benchmark test message. */
static otCoapResource m_bm_result_resource =
{
    .mUriPath = "bm_result",
    .mHandler = bm_result_handler,
    .mContext = NULL,
    .mNext    = NULL
};

/***************************************************************************************************
 * @section Benchmark Coap IPv6 Groups
 **************************************************************************************************/
void bm_increment_group_address(void)
 {
    otError error = OT_ERROR_NONE;

    if (m_config.coap_server_enabled)
    {
        error = otIp6UnsubscribeMulticastAddress(thread_ot_instance_get(), &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);

        bm_group_nr++;

        if (bm_group_nr>=NUMBER_OF_IP6_GROUPS)
        {
            bm_group_nr = 0;
        }

        error = otIp6AddressFromString(bm_group_address_array[bm_group_nr], &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);

        error = otIp6SubscribeMulticastAddress(thread_ot_instance_get(), &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);
    }

    if (m_config.coap_client_enabled)
    {
        bm_group_nr++;

        if (bm_group_nr>=NUMBER_OF_IP6_GROUPS)
        {
            bm_group_nr = 0;
        }

        error = otIp6AddressFromString(bm_group_address_array[bm_group_nr], &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);
    }
    NRF_LOG_INFO(bm_group_address_array[bm_group_nr]);
 }

void bm_decrement_group_address(void)
 {
    otError error = OT_ERROR_NONE;

    if (m_config.coap_server_enabled)
    {
        error = otIp6UnsubscribeMulticastAddress(thread_ot_instance_get(), &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);

        if (bm_group_nr==0)
        {
            bm_group_nr = (NUMBER_OF_IP6_GROUPS-1);
        } else
        {
            bm_group_nr--;
        }

        error = otIp6AddressFromString(bm_group_address_array[bm_group_nr], &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);

        error = otIp6SubscribeMulticastAddress(thread_ot_instance_get(), &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);
    }

    if (m_config.coap_client_enabled)
    {
        if (bm_group_nr==0)
        {
            bm_group_nr = (NUMBER_OF_IP6_GROUPS-1);
        } else
        {
            bm_group_nr--;
        }

        error = otIp6AddressFromString(bm_group_address_array[bm_group_nr], &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);
    }
    NRF_LOG_INFO(bm_group_address_array[bm_group_nr]);
 }

/***************************************************************************************************
 * @section Benchmark Coap handler for default messages
 **************************************************************************************************/
static void coap_default_handler(void                * p_context,
                                 otMessage           * p_message,
                                 const otMessageInfo * p_message_info)
{
    UNUSED_PARAMETER(p_context);
    UNUSED_PARAMETER(p_message);
    UNUSED_PARAMETER(p_message_info);

    NRF_LOG_INFO("Received CoAP message that does not match any request or resource\r\n");
}
/***************************************************************************************************
 * @section Benchmark Coap Unicast time result response to master
 **************************************************************************************************/
static void bm_result_handler(void                 * p_context,
                                   otMessage            * p_message,
                                   const otMessageInfo  * p_message_info)
{
    bm_message_info message_info;

    do
    {
        if (otCoapMessageGetType(p_message) != OT_COAP_TYPE_CONFIRMABLE &&
            otCoapMessageGetType(p_message) != OT_COAP_TYPE_NON_CONFIRMABLE)
        {
            break;
        }

        if (otCoapMessageGetCode(p_message) != OT_COAP_CODE_PUT)
        {
            break;
        }

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &message_info, sizeof(message_info)) != 16)
        {
            NRF_LOG_INFO("test message handler - missing results");
        }

        NRF_LOG_INFO("Got result message");
        bm_cli_write_result(message_info);

    } while (false);
}

void bm_coap_results_send(bm_message_info message_info)
{
    otError         error = OT_ERROR_NONE;
    otMessage     * p_request;
    otMessageInfo   messafe_info;
    otInstance    * p_instance = thread_ot_instance_get();

    do
    {
        if (otIp6IsAddressUnspecified(&bm_master_address))
        {
            NRF_LOG_INFO("Failed to send coap test message: No master address found")
            break;
        }

        p_request = otCoapNewMessage(p_instance, NULL);
        if (p_request == NULL)
        {
            NRF_LOG_INFO("Failed to allocate message for Coap request");
            break;
        }

        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
        otCoapMessageGenerateToken(p_request, 2);
        
        error = otCoapMessageAppendUriPathOptions(p_request, "bm_result");
        ASSERT(error == OT_ERROR_NONE);

        error = otCoapMessageSetPayloadMarker(p_request);
        ASSERT(error == OT_ERROR_NONE);

        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
        otCoapMessageGenerateToken(p_request, 2);
        UNUSED_VARIABLE(otCoapMessageAppendUriPathOptions(p_request, "bm_result"));
        UNUSED_VARIABLE(otCoapMessageSetPayloadMarker(p_request));

        error = otMessageAppend(p_request, &message_info, sizeof(message_info));
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        memset(&messafe_info, 0, sizeof(messafe_info));
        messafe_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        messafe_info.mAllowZeroHopLimit = false;
        messafe_info.mHopLimit = HOP_LIMIT_DEFAULT;
        memcpy(&messafe_info.mPeerAddr, &bm_master_address, sizeof(messafe_info.mPeerAddr));
        
        error = otCoapSendRequest(p_instance, p_request, &messafe_info, NULL, p_instance);
    } while (false);

    if (error != OT_ERROR_NONE && p_request != NULL)
    {
        NRF_LOG_INFO("Failed to send Coap request: %d", error);
        otMessageFree(p_request);
    }
}

/***************************************************************************************************
 * @section Benchmark Coap Multicast Benchmark start message
 **************************************************************************************************/
static void bm_start_handler(void                 * p_context,
                             otMessage            * p_message,
                             const otMessageInfo  * p_message_info)
{
    bm_master_message message;

    do
    {
        if (otCoapMessageGetType(p_message) != OT_COAP_TYPE_CONFIRMABLE &&
            otCoapMessageGetType(p_message) != OT_COAP_TYPE_NON_CONFIRMABLE)
        {
            break;
        }

        if (otCoapMessageGetCode(p_message) != OT_COAP_CODE_PUT)
        {
            break;
        }

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &message, sizeof(message)) != 24)
        {
            NRF_LOG_INFO("benchmark request handler - missing message")
        }

        
        bm_master_address = message.bm_master_ip6_address;

        if (m_config.coap_client_enabled)
        {
            bm_sm_time_set(message.bm_time);
            message.bm_status ? bm_sm_new_state_set(BM_STATE_1_CLIENT) : bm_sm_new_state_set(BM_STATE_STOP);
        }
              
        if (m_config.coap_server_enabled)
        {
            bm_sm_time_set(message.bm_time);
            message.bm_status ? bm_sm_new_state_set(BM_STATE_1_SERVER) : bm_sm_new_state_set(BM_STATE_STOP);
        }

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            //If there will be a confirmation.
        }
        
    } while(false);
}

void bm_coap_multicast_start_send(bm_master_message message)
{
    otError       error = OT_ERROR_NONE;
    otMessage   * p_request;
    otMessageInfo message_info;
    const char  * p_scope = "ff03::1"; // FTDs and MTDs Mesh-Local
    otInstance  * p_instance = thread_ot_instance_get();

    do
    {
        p_request = otCoapNewMessage(p_instance, NULL);
        if (p_request == NULL)
        {
          NRF_LOG_INFO("Failed to allocate message for Coap request");
          break;
        }

        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);

        error = otCoapMessageAppendUriPathOptions(p_request, "bm_start");
        ASSERT(error == OT_ERROR_NONE);

        error = otCoapMessageSetPayloadMarker(p_request);
        ASSERT(error == OT_ERROR_NONE);

        error = otMessageAppend(p_request, &message, sizeof(message));
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        memset(&message_info, 0, sizeof(message_info));
        message_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        message_info.mAllowZeroHopLimit = false;
        message_info.mHopLimit = HOP_LIMIT_DEFAULT;

        error = otIp6AddressFromString(p_scope, &message_info.mPeerAddr);
        ASSERT(error == OT_ERROR_NONE);

        error = otCoapSendRequest(p_instance, p_request, &message_info, NULL, NULL);
    } while (false);

    if (error != OT_ERROR_NONE && p_request != NULL)
    {
        NRF_LOG_INFO("Failed to send Coap request: %d\r\n", error);
        otMessageFree(p_request);
    }
}

/***************************************************************************************************
 * @section Benchmark Coap probe message.
 **************************************************************************************************/
static void bm_probe_message_response_send(otMessage           * p_request_message,
                                           const otMessageInfo * p_message_info)
{
    otError      error = OT_ERROR_NO_BUFS;
    otMessage  * p_response;
    otInstance * p_instance = thread_ot_instance_get();

    do
    {
        p_response = otCoapNewMessage(p_instance, NULL);
        if (p_response == NULL)
        {
            break;
        }

        error = otCoapMessageInitResponse(p_response,
                                          p_request_message,
                                          OT_COAP_TYPE_ACKNOWLEDGMENT,
                                          OT_COAP_CODE_CHANGED);

        if (error != OT_ERROR_NONE)
        {
            break;
        }

        error = otCoapSendResponse(p_instance, p_response, p_message_info);

    } while (false);

    if ((error != OT_ERROR_NONE) && (p_response != NULL))
    {
        otMessageFree(p_response);
    }
}

static void bm_probe_message_handler(void                 * p_context,
                                     otMessage            * p_message,
                                     const otMessageInfo  * p_message_info)
{
    uint16_t  bm_probe;
    bm_message_info message;

    do
    {
        if (otCoapMessageGetType(p_message) != OT_COAP_TYPE_CONFIRMABLE &&
            otCoapMessageGetType(p_message) != OT_COAP_TYPE_NON_CONFIRMABLE)
        {
            break;
        }

        if (otCoapMessageGetCode(p_message) != OT_COAP_CODE_PUT)
        {
            break;
        }

        message.RSSI = otMessageGetRss(p_message);
        message.message_id = otCoapMessageGetMessageId(p_message);
        message.number_of_hops = (HOP_LIMIT_DEFAULT - p_message_info->mHopLimit);

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &bm_probe, sizeof(bm_probe)) == 1)
        {
            NRF_LOG_INFO("Server: Got 1 bit message");
            bsp_board_led_invert(BSP_BOARD_LED_2);
            message.data_size = 0;
        } else
        {
            NRF_LOG_INFO("Server: Got 1024 byte message");
            bsp_board_led_invert(BSP_BOARD_LED_2);
            message.data_size = 1;
        }

        otNetworkTimeGet(thread_ot_instance_get(), &message.net_time);
        bm_save_message_info(message);

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            NRF_LOG_INFO("Server: OT_COAP_TYPE_CONFIRMABLE");
            bm_probe_message_response_send(p_message, p_message_info);
        }
    } while (false);
}

void bm_coap_probe_message_send(uint8_t state)
{
    otError           error = OT_ERROR_NONE;
    otMessage       * p_request;
    otMessageInfo     messafe_info;
    otInstance      * p_instance = thread_ot_instance_get();
    bm_message_info   message;

    uint8_t bm_big_probe[DATA_SIZE];
    bool bm_small_probe;
    uint64_t time;

    otNetworkTimeGet(thread_ot_instance_get(), &message.net_time);
    message.RSSI = 0;
    message.number_of_hops = 0;

    do
    {
        if (otIp6IsAddressUnspecified(&bm_group_address))
        {
            NRF_LOG_INFO("Failed to send coap test message: No group address found")
            break;
        }

        p_request = otCoapNewMessage(p_instance, NULL);
        if (p_request == NULL)
        {
            NRF_LOG_INFO("Failed to allocate message for Coap request");
            break;
        }

        //ToDo: If Bedingung für Ack

        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT); //Hier für ACK
        otCoapMessageGenerateToken(p_request, 2);
        
        error = otCoapMessageAppendUriPathOptions(p_request, "bm_test");
        ASSERT(error == OT_ERROR_NONE);

        error = otCoapMessageSetPayloadMarker(p_request);
        ASSERT(error == OT_ERROR_NONE);
        
        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT); //Hier für ACK
        otCoapMessageGenerateToken(p_request, 2);
        UNUSED_VARIABLE(otCoapMessageAppendUriPathOptions(p_request, "bm_test"));
        UNUSED_VARIABLE(otCoapMessageSetPayloadMarker(p_request));

        if (state == BM_1bit)
        {
            error = otMessageAppend(p_request, &bm_small_probe, sizeof(bool));
            message.data_size = 0;
        } else if (state == BM_1024Bytes)
        {
            otRandomNonCryptoFillBuffer(bm_big_probe, DATA_SIZE);
            error = otMessageAppend(p_request, bm_big_probe, (DATA_SIZE*sizeof(uint8_t)));
            message.data_size = 1;
        }

        if (error != OT_ERROR_NONE)
        {
            break;
        }

        memset(&messafe_info, 0, sizeof(messafe_info));
        messafe_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        messafe_info.mAllowZeroHopLimit = false;
        messafe_info.mHopLimit = HOP_LIMIT_DEFAULT;
        memcpy(&messafe_info.mPeerAddr, &bm_group_address, sizeof(messafe_info.mPeerAddr));
                 
        error = otCoapSendRequest(p_instance, p_request, &messafe_info, NULL, p_instance);

        message.message_id = otCoapMessageGetMessageId(p_request);
        bm_save_message_info(message);
    } while (false);

    

    if (error != OT_ERROR_NONE && p_request != NULL)
    {
        NRF_LOG_INFO("Failed to send Coap request: %d", error);
        otMessageFree(p_request);
    }
}

/***************************************************************************************************
 * @section Benchmark Coap Initialization
 **************************************************************************************************/
void thread_coap_utils_init(const thread_coap_utils_configuration_t * p_config)
{
    otInstance * p_instance = thread_ot_instance_get();

    otError error = otCoapStart(p_instance, OT_DEFAULT_COAP_PORT);
    ASSERT(error == OT_ERROR_NONE);

    otCoapSetDefaultHandler(p_instance, coap_default_handler, NULL);

    m_config = *p_config;

    error = otIp6AddressFromString(bm_group_address_array[0], &bm_group_address);
    ASSERT(error == OT_ERROR_NONE);

    if (m_config.coap_server_enabled)
    {
        m_bm_test_resource.mContext = p_instance;
        otCoapAddResource(p_instance, &m_bm_test_resource);
        ASSERT(error == OT_ERROR_NONE);

        error = otIp6SubscribeMulticastAddress(thread_ot_instance_get(), &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);
    }

    if (m_config.coap_client_enabled || m_config.coap_server_enabled)
    {
        m_bm_start_resource.mContext    = p_instance;
        otCoapAddResource(p_instance, &m_bm_start_resource);
        ASSERT(error == OT_ERROR_NONE);
    }

    if (!m_config.coap_client_enabled || !m_config.coap_server_enabled)
    {
        m_bm_result_resource.mContext   = p_instance;
        otCoapAddResource(p_instance, &m_bm_result_resource);
        ASSERT(error == OT_ERROR_NONE);
    }
}

void thread_coap_utils_deinit(void)
{
    otInstance * p_instance = thread_ot_instance_get();

    otError error = otCoapStop(p_instance);
    ASSERT(error == OT_ERROR_NONE);

    otCoapSetDefaultHandler(p_instance, NULL, NULL);

    if (m_config.coap_server_enabled)
    {
        m_bm_test_resource.mContext      = NULL;
        otCoapRemoveResource(p_instance, &m_bm_test_resource);

        error = otIp6UnsubscribeMulticastAddress(thread_ot_instance_get(), &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);
    }

    if (m_config.coap_client_enabled || m_config.coap_server_enabled)
    {
        m_bm_start_resource.mContext    = NULL;
        otCoapRemoveResource(p_instance, &m_bm_start_resource);
    }
    
    if (!m_config.coap_client_enabled || !m_config.coap_server_enabled)
    {
        m_bm_result_resource.mContext   = NULL;
        otCoapRemoveResource(p_instance, &m_bm_result_resource);
    }

    bm_group_nr = 0;
    error = otIp6AddressFromString("0", &bm_group_address);
    ASSERT(error == OT_ERROR_NONE);
}