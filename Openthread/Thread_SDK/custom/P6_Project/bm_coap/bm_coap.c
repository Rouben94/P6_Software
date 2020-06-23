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

#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>
#include <openthread/platform/alarm-milli.h>

#define RESPONSE_POLL_PERIOD     100
#define PROVISIONING_EXPIRY_TIME 5000
#define LED_INTERVAL             100

APP_TIMER_DEF(m_provisioning_timer);
APP_TIMER_DEF(m_led_timer);

/**@brief Structure holding CoAP status information. */
typedef struct
{
    bool         provisioning_enabled; /**< Information if provisioning is enabled. */
    uint32_t     provisioning_expiry;  /**< Provisioning timeout time. */
    bool         led_blinking_is_on;   /**< Information if leds are blinking */
    otIp6Address peer_address;         /**< An address of a related server node. */
} state_t;

otIp6Address bm_master_address;

static uint32_t                                  m_poll_period;
static state_t                                   m_state;
static thread_coap_utils_configuration_t          m_config;
static thread_coap_utils_light_command_handler_t m_light_command_handler;

/**@brief Forward declarations of CoAP resources handlers. */
static void provisioning_request_handler(void *, otMessage *, const otMessageInfo *);
static void bm_start_handler(void *, otMessage *, const otMessageInfo *);
static void bm_test_message_handler(void *, otMessage *, const otMessageInfo *);
static void bm_time_result_handler(void *, otMessage *, const otMessageInfo *);

/**@brief Definition of CoAP resources for provisioning. */
static otCoapResource m_provisioning_resource =
{
    .mUriPath = "provisioning",
    .mHandler = provisioning_request_handler,
    .mContext = NULL,
    .mNext    = NULL
};

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
    .mHandler = bm_test_message_handler,
    .mContext = NULL,
    .mNext    = NULL
};

/**@brief Definition of CoAP resources for benchmark test message. */
static otCoapResource m_bm_result_resource =
{
    .mUriPath = "bm_result",
    .mHandler = bm_time_result_handler,
    .mContext = NULL,
    .mNext    = NULL
};

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
 static void bm_time_result_handler(void                 * p_context,
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

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &message_info, sizeof(message_info)) != 1)
        {
            NRF_LOG_INFO("test message handler - missing command");
        }

        //RESULTS CLI OUT

    } while (false);
}

void bm_coap_unicast_time_results_send(bm_message_info message_info)
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

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &message, sizeof(message)) != 12)
        {
            NRF_LOG_INFO("benchmark request handler - missing message")
        }

        
        bm_master_address = *message.bm_master_ip6_address;

        if (m_config.coap_client_enabled)
        {
            bm_sm_time_set(message.bm_time);
            message.bm_status ? bm_sm_new_state_set(BM_STATE_1) : bm_sm_new_state_set(BM_STATE_2);
        }      

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            //If there will be a confirmation.
        }
        
    } while(false);
}

void bm_coap_multicast_start_send(bm_master_message message, thread_coap_utils_multicast_scope_t scope)
{
    otError       error = OT_ERROR_NONE;
    otMessage   * p_request;
    otMessageInfo message_info;
    const char  * p_scope = NULL;
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

        switch (scope)
        {
            case THREAD_COAP_UTILS_MULTICAST_LINK_LOCAL:
                p_scope = "ff02::1";
                break;
            case THREAD_COAP_UTILS_MULTICAST_REALM_LOCAL:
                p_scope = "ff03::1";
                break;
            default:
                ASSERT(false);
        }

        memset(&message_info, 0, sizeof(message_info));
        message_info.mPeerPort = OT_DEFAULT_COAP_PORT;

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
 * @section Benchmark Coap test message.
 **************************************************************************************************/
static void bm_test_message_response_handler(void                 * p_context,
                                             otMessage            * p_message,
                                             const otMessageInfo  * p_message_info,
                                             otError              * result)
{
    // test message ACK handler
}

static void bm_test_message_response_send(otMessage           * p_request_message,
                                          const otMessageInfo * p_message_info)
{
    otError       error = OT_ERROR_NONE;
    otMessage   * p_response;
    otInstance  * p_instance = thread_ot_instance_get();

    do
    {
        p_response = otCoapNewMessage(p_instance, NULL);
        if (p_response == NULL)
        {
            break;
        }

        error = otCoapMessageInitResponse(p_response, p_request_message, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        error = otCoapSendResponse(p_instance, p_request_message, p_message_info);
    } while (false);

    if (error != OT_ERROR_NONE && p_response != NULL)
    {
        otMessageFree(p_response);
    }
}

static void bm_test_message_handler(void                 * p_context,
                                    otMessage            * p_message,
                                    const otMessageInfo  * p_message_info)
{
    bool state;

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

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &state, sizeof(state)) != 1)
        {
            NRF_LOG_INFO("test message handler - missing command");
        }

        if (state)
        {
            NRF_LOG_INFO("Server: Got test message");
            bm_save_message_info(otCoapMessageGetMessageId(p_message));
        } else if (!state)
        {
            bm_sm_new_state_set(BM_STATE_3);
        }
        

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            bm_test_message_response_send(p_message, p_message_info);
        }
    } while (false);
}

void bm_coap_unicast_test_message_send(bool state)
{
    otError         error = OT_ERROR_NONE;
    otMessage     * p_request;
    otMessageInfo   messafe_info;
    otInstance    * p_instance = thread_ot_instance_get();

    do
    {
        if (otIp6IsAddressUnspecified(&m_state.peer_address))
        {
            NRF_LOG_INFO("Failed to send coap test message: No peer address found")
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
        
        error = otCoapMessageAppendUriPathOptions(p_request, "bm_test");
        ASSERT(error == OT_ERROR_NONE);

        error = otCoapMessageSetPayloadMarker(p_request);
        ASSERT(error == OT_ERROR_NONE);

        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
        otCoapMessageGenerateToken(p_request, 2);
        UNUSED_VARIABLE(otCoapMessageAppendUriPathOptions(p_request, "bm_test"));
        UNUSED_VARIABLE(otCoapMessageSetPayloadMarker(p_request));

        error = otMessageAppend(p_request, &state, sizeof(state));
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        memset(&messafe_info, 0, sizeof(messafe_info));
        messafe_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        memcpy(&messafe_info.mPeerAddr, &m_state.peer_address, sizeof(messafe_info.mPeerAddr));
        
        error = otCoapSendRequest(p_instance, p_request, &messafe_info, NULL, p_instance);

        bm_save_message_info(otCoapMessageGetMessageId(p_request));
    } while (false);

    if (error != OT_ERROR_NONE && p_request != NULL)
    {
        NRF_LOG_INFO("Failed to send Coap request: %d", error);
        otMessageFree(p_request);
    }
}

/***************************************************************************************************
 * @section Benchmark Coap provisioning
 **************************************************************************************************/
static uint32_t poll_period_response_set(void)
{
    uint32_t     error;
    otError      ot_error;
    otInstance * p_instance = thread_ot_instance_get();

    do
    {
        if (otThreadGetLinkMode(p_instance).mRxOnWhenIdle)
        {
            error = NRF_ERROR_INVALID_STATE;
            break;
        }

        if (!m_poll_period)
        {
            m_poll_period = otLinkGetPollPeriod(p_instance);

            NRF_LOG_INFO("Poll Period: %dms set\r\n", RESPONSE_POLL_PERIOD);

            ot_error = otLinkSetPollPeriod(p_instance, RESPONSE_POLL_PERIOD);
            ASSERT(ot_error == OT_ERROR_NONE);

            error =  NRF_SUCCESS;
        }
        else
        {
            error = NRF_ERROR_BUSY;
        }
    } while (false);

    return error;
}

static void poll_period_restore(void)
{
    otError      error;
    otInstance * p_instance = thread_ot_instance_get();

    do
    {
        if (otThreadGetLinkMode(p_instance).mRxOnWhenIdle)
        {
            break;
        }

        if (m_poll_period)
        {
            error = otLinkSetPollPeriod(p_instance, m_poll_period);
            ASSERT(error == OT_ERROR_NONE);

            NRF_LOG_INFO("Poll Period: %dms restored\r\n", m_poll_period);
            m_poll_period = 0;
        }
    } while (false);
}

static void light_changed_default(thread_coap_utils_light_command_t light_command)
{
    switch (light_command)
    {
        case THREAD_COAP_UTILS_LIGHT_CMD_ON:
            LEDS_ON(BSP_LED_3_MASK);
            break;

        case THREAD_COAP_UTILS_LIGHT_CMD_OFF:
            LEDS_OFF(BSP_LED_3_MASK);
            break;

        case THREAD_COAP_UTILS_LIGHT_CMD_TOGGLE:
            LEDS_INVERT(BSP_LED_3_MASK);
            break;

        default:
            break;
    }
}

static bool provisioning_is_enabled(void)
{
    return m_state.provisioning_enabled;
}

static void provisioning_enable(bool enable)
{
    uint32_t error;

    m_state.provisioning_enabled = enable;

    if (enable)
    {
        m_state.provisioning_expiry = otPlatAlarmMilliGetNow() + PROVISIONING_EXPIRY_TIME;
        error = app_timer_start(m_provisioning_timer,
                                APP_TIMER_TICKS(PROVISIONING_EXPIRY_TIME),
                                NULL);
        ASSERT(error == NRF_SUCCESS);

        error = app_timer_start(m_led_timer, APP_TIMER_TICKS(LED_INTERVAL), NULL);
        ASSERT(error == NRF_SUCCESS);

        if (m_config.configurable_led_blinking_enabled)
        {
            m_light_command_handler(THREAD_COAP_UTILS_LIGHT_CMD_ON);
        }
    }
    else
    {
        m_state.provisioning_expiry = 0;

        error = app_timer_stop(m_provisioning_timer);
        ASSERT(error == NRF_SUCCESS);

        error = app_timer_stop(m_led_timer);
        ASSERT(error == NRF_SUCCESS);

        LEDS_OFF(BSP_LED_2_MASK);

        if (m_config.configurable_led_blinking_enabled)
        {
            error = app_timer_stop(m_led_timer);
            ASSERT(error == NRF_SUCCESS);

            if (m_state.led_blinking_is_on)
            {
                m_light_command_handler(THREAD_COAP_UTILS_LIGHT_CMD_ON);
            }
            else
            {
                m_light_command_handler(THREAD_COAP_UTILS_LIGHT_CMD_OFF);
            }
        }
    }
}

static void provisioning_timer_handler(void * p_context)
{
    provisioning_enable(false);
}

static otError provisioning_response_send(otMessage           * p_request_message,
                                          const otMessageInfo * p_message_info)
{
    otError        error = OT_ERROR_NO_BUFS;
    otMessage    * p_response;
    otInstance   * p_instance = thread_ot_instance_get();

    do
    {
        p_response = otCoapNewMessage(p_instance, NULL);
        if (p_response == NULL)
        {
            break;
        }

        otCoapMessageInit(p_response, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_CONTENT);

        error = otCoapMessageSetToken(p_response,
                                      otCoapMessageGetToken(p_request_message),
                                      otCoapMessageGetTokenLength(p_request_message));
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        error = otCoapMessageSetPayloadMarker(p_response);
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        error = otMessageAppend(
            p_response, otThreadGetMeshLocalEid(p_instance), sizeof(otIp6Address));
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        error = otCoapSendResponse(p_instance, p_response, p_message_info);

    } while (false);

    if (error != OT_ERROR_NONE && p_response != NULL)
    {
        otMessageFree(p_response);
    }

    return error;
}

static void provisioning_request_handler(void                * p_context,
                                         otMessage           * p_message,
                                         const otMessageInfo * p_message_info)
{
    UNUSED_PARAMETER(p_message);

    otError       error;
    otMessageInfo message_info;

    if (!provisioning_is_enabled())
    {
        return;
    }

    if ((otCoapMessageGetType(p_message) == OT_COAP_TYPE_NON_CONFIRMABLE) &&
        (otCoapMessageGetCode(p_message) == OT_COAP_CODE_GET))
    {
        message_info = *p_message_info;
        memset(&message_info.mSockAddr, 0, sizeof(message_info.mSockAddr));

        error = provisioning_response_send(p_message, &message_info);
        if (error == OT_ERROR_NONE)
        {
            provisioning_enable(false);
        }
    }
}

static void led_timer_handler(void * p_context)
{
    // This handler may be called after app_timer_stop due to app_shceduler.
    if (m_state.provisioning_enabled)
    {
        LEDS_INVERT(BSP_LED_2_MASK);
    }
}

static void provisioning_response_handler(void                * p_context,
                                          otMessage           * p_message,
                                          const otMessageInfo * p_message_info,
                                          otError               result)
{
    //result = OT_ERROR_NONE;
    UNUSED_PARAMETER(p_context);

    // Restore the polling period back to initial slow value.
    poll_period_restore();

    if (result == OT_ERROR_NONE)
    {
        UNUSED_RETURN_VALUE(otMessageRead(p_message,
                                          otMessageGetOffset(p_message),
                                          &m_state.peer_address,
                                          sizeof(m_state.peer_address)));
    }
    else
    {
        NRF_LOG_INFO("Provisioning failed: %d\r\n", result);
    }
}

void thread_coap_utils_provisioning_request_send(void)
{
    otError       error = OT_ERROR_NO_BUFS;
    otMessage   * p_request;
    otMessageInfo message_info;
    otInstance  * p_instance = thread_ot_instance_get();

    do
    {
        p_request = otCoapNewMessage(p_instance, NULL);
        if (p_request == NULL)
        {
            break;
        }

        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_GET);
        otCoapMessageGenerateToken(p_request, 2);

        error = otCoapMessageAppendUriPathOptions(p_request, "provisioning");
        ASSERT(error == OT_ERROR_NONE);

        // decrease the polling period for higher responsiveness
        uint32_t err_code = poll_period_response_set();
        if (err_code == NRF_ERROR_BUSY)
        {
            break;
        }

        memset(&message_info, 0, sizeof(message_info));
        message_info.mPeerPort = OT_DEFAULT_COAP_PORT;

        error = otIp6AddressFromString("ff03::1", &message_info.mPeerAddr);
        ASSERT(error == OT_ERROR_NONE);

        error = otCoapSendRequest(
            p_instance, p_request, &message_info, provisioning_response_handler, p_instance);

    } while (false);

    if (error != OT_ERROR_NONE && p_request != NULL)
    {
        otMessageFree(p_request);
    }

}

/***************************************************************************************************
 * @section Benchmark Coap external functions
 **************************************************************************************************/
void thread_coap_utils_provisioning_enable_set(bool value)
{
    provisioning_enable(value);
}

void thread_coap_utils_light_command_handler_set(thread_coap_utils_light_command_handler_t handler)
{
    ASSERT(handler != NULL);

    m_light_command_handler = handler;
}

bool thread_coap_utils_light_is_led_blinking(void)
{
    return m_state.led_blinking_is_on;
}

void thread_coap_utils_peer_addr_clear(void)
{
    memset(&m_state.peer_address, 0, sizeof(m_state.peer_address));
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

    m_light_command_handler = light_changed_default;

    if (m_config.coap_server_enabled)
    {
        LEDS_CONFIGURE(LEDS_MASK);
        LEDS_OFF(LEDS_MASK);

        uint32_t retval = app_timer_create(&m_led_timer,
                                           APP_TIMER_MODE_REPEATED,
                                           led_timer_handler);
        ASSERT(retval == NRF_SUCCESS);

        retval = app_timer_create(&m_provisioning_timer,
                                  APP_TIMER_MODE_SINGLE_SHOT,
                                  provisioning_timer_handler);
        ASSERT(retval == NRF_SUCCESS);

        m_provisioning_resource.mContext = p_instance;

        error = otCoapAddResource(p_instance, &m_provisioning_resource);
        ASSERT(error == OT_ERROR_NONE);

        m_bm_test_resource.mContext = p_instance;

        error = otCoapAddResource(p_instance, &m_bm_test_resource);
        ASSERT(error == OT_ERROR_NONE);
    }

    if (m_config.coap_client_enabled || m_config.coap_server_enabled)
    {
        m_bm_start_resource.mContext    = p_instance;

        error = otCoapAddResource(p_instance, &m_bm_start_resource);
        ASSERT(error == OT_ERROR_NONE);
    }

    if (!m_config.coap_client_enabled || !m_config.coap_server_enabled)
    {
        m_bm_result_resource.mContext   = p_instance;

        error = otCoapAddResource(p_instance, &m_bm_result_resource);
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
        m_provisioning_resource.mContext = NULL;
        m_bm_test_resource.mContext      = NULL;
        
        otCoapRemoveResource(p_instance, &m_provisioning_resource);
        otCoapRemoveResource(p_instance, &m_bm_test_resource);
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
}