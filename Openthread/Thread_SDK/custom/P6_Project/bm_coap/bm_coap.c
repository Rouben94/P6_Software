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
#include <openthread/random_noncrypto.h>

#define NUMBER_OF_IP6_GROUPS 25
#define DATA_SIZE 1024
#define HOP_LIMIT_DEFAULT 64
#define max_number_of_nodes 100

APP_TIMER_DEF(m_slave_timer);

uint8_t bm_message_ID = 0;

// TID OVerflow Handler -> Allows an Overflow of of the TID when the next TID is dropping by 250... (uint8 overflows at 255 so there is a margin of 5)
// To Handle such case for all possibel Src. Adresses create the following struct and a array of the struct to save the last seen TID of each Addr.
typedef struct
{
  uint16_t src_addr;
  uint8_t last_TID_seen;
  uint8_t TID_OverflowCnt;
} __attribute__((packed)) bm_tid_overflow_handler_t;

bm_tid_overflow_handler_t bm_tid_overflow_handler[max_number_of_nodes]; // Excpect not more than max_number_of_nodes Different Adresses

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
const char *bm_group_address_array[NUMBER_OF_IP6_GROUPS] = {"ff03::3", 
                                                            "ff03::4",
                                                            "ff03::5",
                                                            "ff03::6",
                                                            "ff03::7",
                                                            "ff03::8",
                                                            "ff03::9",
                                                            "ff03::10",
                                                            "ff03::11",
                                                            "ff03::12",
                                                            "ff03::13", 
                                                            "ff03::14",
                                                            "ff03::15",
                                                            "ff03::16",
                                                            "ff03::17",
                                                            "ff03::18",
                                                            "ff03::19",
                                                            "ff03::20",
                                                            "ff03::21",
                                                            "ff03::22",
                                                            "ff03::23",
                                                            "ff03::24",
                                                            "ff03::25",
                                                            "ff03::26",
                                                            "ff03::27"};

static thread_coap_utils_configuration_t          m_config;

/**@brief Forward declarations of CoAP resources handlers. */
static void bm_start_handler(void *, otMessage *, const otMessageInfo *);
static void bm_start_request_handler(void *, otMessage *, const otMessageInfo *);
static void bm_probe_message_handler(void *, otMessage *, const otMessageInfo *);
static void bm_result_handler(void *, otMessage *, const otMessageInfo *);
static void bm_result_request_handler(void *, otMessage *, const otMessageInfo *);

/**@brief Definition of CoAP resources for benchmark start. */
static otCoapResource m_bm_start_resource =
{
    .mUriPath = "bm_start",
    .mHandler = bm_start_handler,
    .mContext = NULL,
    .mNext    = NULL
};

/**@brief Definition of CoAP resources for benchmark start. */
static otCoapResource m_bm_start_request_resource =
{
    .mUriPath = "bm_start_request",
    .mHandler = bm_start_request_handler,
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

/**@brief Definition of CoAP resources for benchmark test message. */
static otCoapResource m_bm_result_request_resource =
{
    .mUriPath = "test",
    .mHandler = bm_result_request_handler,
    .mContext = NULL,
    .mNext    = NULL
};

// Insert the tid and src address to get the merged tid with the tid overlfow cnt -> resulting in a uint16_t
uint16_t bm_get_overflow_tid_from_overflow_handler(uint8_t tid, uint16_t src_addr){
	// Get the TID in array
	for (int i = 0; i < max_number_of_nodes; i++)
	{
		if(bm_tid_overflow_handler[i].src_addr == src_addr){
			// Check if Overflow happend
			if((bm_tid_overflow_handler[i].last_TID_seen - tid) > 250){
				bm_tid_overflow_handler[i].TID_OverflowCnt++;
			}
			// Add the last seen TID
			bm_tid_overflow_handler[i].last_TID_seen = tid;	
			return (uint16_t) (bm_tid_overflow_handler[i].TID_OverflowCnt << 8 ) | (tid & 0xff);		
			//return bm_tid_overflow_handler[i].TID_OverflowCnt;
		} else if(bm_tid_overflow_handler[i].src_addr == 0){
			// Add the Src Adress
			bm_tid_overflow_handler[i].src_addr = src_addr;
			bm_tid_overflow_handler[i].last_TID_seen = tid;
			bm_tid_overflow_handler[i].TID_OverflowCnt = 0;
			return (uint16_t) (bm_tid_overflow_handler[i].TID_OverflowCnt << 8 ) | (tid & 0xff);
		}
	}
	return 0; // Default return 0	
}

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
 * @section Benchmark Coap Unicast result response to master
 **************************************************************************************************/
static void bm_coap_result_response_handler(void                * p_context,
                                            otMessage           * p_message,
                                            const otMessageInfo * p_message_info,
                                            otError               result)
{
    if (result == OT_ERROR_NONE)
    {
      if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_ACKNOWLEDGMENT)
      {
          NRF_LOG_INFO("Slave: Result succesfull transmitted");
          bm_sm_new_state_set(BM_STATE_3_SLAVE);
      }      
    }
    else 
    {
      if (result == OT_ERROR_RESPONSE_TIMEOUT)  // Coap response or acknowledgment or DNS response not received
      {
          NRF_LOG_INFO("Slave: Result response not received");
          bm_sm_new_state_set(BM_EMPTY_STATE);
      }    
    }
}

static void bm_coap_result_response_send(otMessage           * p_request_message,
                                        const otMessageInfo * p_message_info)
{
    otError      error = OT_ERROR_NO_BUFS;
    otMessage  * p_response;
    otInstance * p_instance = thread_ot_instance_get();

    NRF_LOG_INFO("Master: result response send")

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

static void bm_result_handler(void                 * p_context,
                                   otMessage            * p_message,
                                   const otMessageInfo  * p_message_info)
{
    NRF_LOG_INFO("Master: Got result message");
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

        uint16_t size = (otMessageGetLength(p_message) - otMessageGetOffset(p_message))/sizeof(bm_message_info);
        bm_message_info message_info[size];
   
        otMessageRead(p_message, otMessageGetOffset(p_message), message_info, sizeof(message_info));
        bm_save_result(message_info, size);

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            bm_coap_result_response_send(p_message, p_message_info);
        }
    } while(false);
}

void bm_coap_results_send(bm_message_info message_info[], uint16_t size)
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

        otCoapMessageInit(p_request, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT);
        otCoapMessageGenerateToken(p_request, 2);
        
        error = otCoapMessageAppendUriPathOptions(p_request, "bm_result");
        ASSERT(error == OT_ERROR_NONE);

        error = otCoapMessageSetPayloadMarker(p_request);
        ASSERT(error == OT_ERROR_NONE);

        otCoapMessageInit(p_request, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT);
        otCoapMessageGenerateToken(p_request, 2);
        UNUSED_VARIABLE(otCoapMessageAppendUriPathOptions(p_request, "bm_result"));
        UNUSED_VARIABLE(otCoapMessageSetPayloadMarker(p_request));

        error = otMessageAppend(p_request, message_info, size);
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        memset(&messafe_info, 0, sizeof(messafe_info));
        messafe_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        messafe_info.mAllowZeroHopLimit = false;
        messafe_info.mHopLimit = HOP_LIMIT_DEFAULT;
        messafe_info.mPeerAddr = bm_master_address;
        
        error = otCoapSendRequest(p_instance, p_request, &messafe_info, bm_coap_result_response_handler, p_instance);
    } while (false);

    if (error != OT_ERROR_NONE && p_request != NULL)
    {
        NRF_LOG_INFO("Failed to send Coap request: %d", error);
        otMessageFree(p_request);
    }
}

static void bm_result_request_handler(void                 * p_context,
                                      otMessage            * p_message,
                                      const otMessageInfo  * p_message_info)
{
    bool result_request;
    NRF_LOG_INFO("Slave: got result request")
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

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &result_request, sizeof(bool)) != 1)
        {
            NRF_LOG_INFO("benchmark request handler - missing message")
        }
        
        bm_sm_new_state_set(BM_STATE_2_SLAVE);
    } while(false);
}

void bm_coap_result_request_send(otIp6Address address)
{
    otError       error = OT_ERROR_NONE;
    otMessage   * p_request;
    otMessageInfo message_info;
    otInstance  * p_instance = thread_ot_instance_get();

    NRF_LOG_INFO("Master:  Send result request");
    bool result_request = true;

    do
    {
        p_request = otCoapNewMessage(p_instance, NULL);
        if (p_request == NULL)
        {
          NRF_LOG_INFO("Failed to allocate message for Coap request");
          break;
        }

        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);

        error = otCoapMessageAppendUriPathOptions(p_request, "test");
        ASSERT(error == OT_ERROR_NONE);

        error = otCoapMessageSetPayloadMarker(p_request);
        ASSERT(error == OT_ERROR_NONE);

        error = otMessageAppend(p_request, &result_request, sizeof(bool));
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        memset(&message_info, 0, sizeof(message_info));
        message_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        message_info.mPeerAddr = address;
        message_info.mAllowZeroHopLimit = false;
        message_info.mHopLimit = HOP_LIMIT_DEFAULT;

        error = otCoapSendRequest(p_instance, p_request, &message_info, NULL, p_instance);
    } while (false);

    if (error != OT_ERROR_NONE && p_request != NULL)
    {
        NRF_LOG_INFO("Failed to send Coap request: %d\r\n", error);
        otMessageFree(p_request);
    }
}

/***************************************************************************************************
 * @section Benchmark Coap Multicast Benchmark start message
 **************************************************************************************************/
//static void bm_coap_start_response_handler(void                * p_context,
//                                           otMessage           * p_message,
//                                           const otMessageInfo * p_message_info,
//                                           otError               result)
//{
//    if (result == OT_ERROR_NONE)
//    {
//      if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_ACKNOWLEDGMENT)
//      {
//          NRF_LOG_INFO("Slave: Start Benchmark");
//          bsp_board_led_on(BSP_BOARD_LED_2);
//          if (m_config.coap_client_enabled)
//          {
//            bm_sm_new_state_set(BM_STATE_1_SLAVE);
//          }
//      }      
//    }
//    else 
//    {
//      if (result == OT_ERROR_RESPONSE_TIMEOUT)  // Coap response or acknowledgment or DNS response not received
//      {
//          NRF_LOG_INFO("Slave: Response not received");
//      }    
//    }
//}

//static void bm_coap_start_response_send(otMessage           * p_request_message,
//                                        const otMessageInfo * p_message_info)
//{
//    otError      error = OT_ERROR_NO_BUFS;
//    otMessage  * p_response;
//    otInstance * p_instance = thread_ot_instance_get();
//
//    NRF_LOG_INFO("Master: start response send")
//
//    do
//    {
//        p_response = otCoapNewMessage(p_instance, NULL);
//        if (p_response == NULL)
//        {
//            break;
//        }
//
//        error = otCoapMessageInitResponse(p_response,
//                                          p_request_message,
//                                          OT_COAP_TYPE_ACKNOWLEDGMENT,
//                                          OT_COAP_CODE_CHANGED);
//
//        if (error != OT_ERROR_NONE)
//        {
//            break;
//        }
//
//        error = otCoapSendResponse(p_instance, p_response, p_message_info);
//
//    } while (false);
//
//    if ((error != OT_ERROR_NONE) && (p_response != NULL))
//    {
//        otMessageFree(p_response);
//    }
//}

static void bm_start_request_handler(void                 * p_context,
                                     otMessage            * p_message,
                                     const otMessageInfo  * p_message_info)
{
    NRF_LOG_INFO("Master: got start request")
    otIp6Address bm_slave_address;

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

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &bm_slave_address, sizeof(bm_slave_address)) != 16)
        {
            NRF_LOG_INFO("benchmark request handler - missing message")
        }

        bm_save_slave_address(bm_slave_address);

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            //bm_coap_start_response_send(p_message, p_message_info);
        }
    } while(false);
}

void bm_coap_master_start_request_send()
{
    otError       error = OT_ERROR_NONE;
    otMessage   * p_request;
    otMessageInfo message_info;
    otInstance  * p_instance = thread_ot_instance_get();

    NRF_LOG_INFO("Slave: Start request send");
    otIp6Address slave_address = *otThreadGetMeshLocalEid(thread_ot_instance_get());

    do
    {
        p_request = otCoapNewMessage(p_instance, NULL);
        if (p_request == NULL)
        {
          NRF_LOG_INFO("Failed to allocate message for Coap request");
          break;
        }

        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);

        error = otCoapMessageAppendUriPathOptions(p_request, "bm_start_request");
        ASSERT(error == OT_ERROR_NONE);

        error = otCoapMessageSetPayloadMarker(p_request);
        ASSERT(error == OT_ERROR_NONE);

        error = otMessageAppend(p_request, &slave_address, sizeof(otIp6Address));
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        memset(&message_info, 0, sizeof(message_info));
        message_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        message_info.mPeerAddr = bm_master_address;
        message_info.mAllowZeroHopLimit = false;
        message_info.mHopLimit = HOP_LIMIT_DEFAULT;

        error = otCoapSendRequest(p_instance, p_request, &message_info, NULL, p_instance);

//        error = otCoapSendRequest(p_instance, p_request, &message_info, bm_coap_start_response_handler, p_instance);
    } while (false);

    if (error != OT_ERROR_NONE && p_request != NULL)
    {
        NRF_LOG_INFO("Failed to send Coap request: %d\r\n", error);
        otMessageFree(p_request);
    }
}

static void m_slave_timer_handler(void * p_context)
{
    bm_coap_master_start_request_send();
}

static void bm_start_handler(void                 * p_context,
                             otMessage            * p_message,
                             const otMessageInfo  * p_message_info)
{
    bm_master_message message;
    uint32_t error;

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

        bm_sm_time_set(message.bm_time);

        NRF_LOG_INFO("Slave: Start Benchmark");
        bsp_board_led_on(BSP_BOARD_LED_2);
        if (m_config.coap_client_enabled)
        {
          bm_sm_new_state_set(BM_STATE_1_SLAVE);
        }

        error = app_timer_start(m_slave_timer, APP_TIMER_TICKS(otRandomNonCryptoGetUint16InRange(1, 100)), NULL);
        ASSERT(error == NRF_SUCCESS);

        //bm_coap_master_start_request_send(); // delay this
    } while(false);
}

void bm_coap_multicast_start_send(bm_master_message message)
{
    otError       error = OT_ERROR_NONE;
    otMessage   * p_request;
    otMessageInfo message_info;
    const char  * p_scope = "ff03::1"; // FTDs and MTDs Mesh-Local
    otInstance  * p_instance = thread_ot_instance_get();

    NRF_LOG_INFO("Master:  Send multicast start")

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

        error = otCoapSendRequest(p_instance, p_request, &message_info, NULL, p_instance);
        bm_sm_new_state_set(BM_STATE_1_MASTER);
        bm_sm_time_set(message.bm_time);
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
//static void bm_probe_message_response_send(otMessage           * p_request_message,
//                                           const otMessageInfo * p_message_info)
//{
//    otError      error = OT_ERROR_NO_BUFS;
//    otMessage  * p_response;
//    otInstance * p_instance = thread_ot_instance_get();
//
//    do
//    {
//        p_response = otCoapNewMessage(p_instance, NULL);
//        if (p_response == NULL)
//        {
//            break;
//        }
//
//        error = otCoapMessageInitResponse(p_response,
//                                          p_request_message,
//                                          OT_COAP_TYPE_ACKNOWLEDGMENT,
//                                          OT_COAP_CODE_CHANGED);
//
//        if (error != OT_ERROR_NONE)
//        {
//            break;
//        }
//
//        error = otCoapSendResponse(p_instance, p_response, p_message_info);
//
//    } while (false);
//
//    if ((error != OT_ERROR_NONE) && (p_response != NULL))
//    {
//        otMessageFree(p_response);
//    }
//}

static void bm_probe_message_handler(void                 * p_context,
                                     otMessage            * p_message,
                                     const otMessageInfo  * p_message_info)
{
    uint8_t bm_probe[3];
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

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &bm_probe, 3*sizeof(bm_probe)) == 3)
        {
            NRF_LOG_INFO("Server: Got 1 bit message");
            message.message_id = bm_get_overflow_tid_from_overflow_handler(bm_probe[0], ((bm_probe[1] & 0xff) | (bm_probe[2] << 8)));
            bsp_board_led_invert(BSP_BOARD_LED_3);
            message.data_size = 3;
        } else
        {
            NRF_LOG_INFO("Server: Got 1024 byte message");
            message.message_id = bm_get_overflow_tid_from_overflow_handler(bm_probe[0], ((bm_probe[1] & 0xff) | (bm_probe[2] << 8)));
            bsp_board_led_invert(BSP_BOARD_LED_3);
            message.data_size = 1024;
        }

        otNetworkTimeGet(thread_ot_instance_get(), &message.net_time);
        message.RSSI = otMessageGetRss(p_message);
        message.number_of_hops = (HOP_LIMIT_DEFAULT - p_message_info->mHopLimit);
        message.source_address = ((bm_probe[1] & 0xff) | (bm_probe[2] << 8));
        message.dest_address = (*otThreadGetMeshLocalEid(thread_ot_instance_get())).mFields.m16[7];
        message.grp_address = bm_group_address.mFields.m16[7];
        message.net_time_ack = 0;
        bm_save_message_info(message);

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            NRF_LOG_INFO("Server: OT_COAP_TYPE_CONFIRMABLE");
            //bm_probe_message_response_send(p_message, p_message_info);
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

    bm_message_ID++;

    message.RSSI = 0;
    message.number_of_hops = 0;
    message.source_address = (*otThreadGetMeshLocalEid(thread_ot_instance_get())).mFields.m16[7];
    message.dest_address = bm_group_address.mFields.m16[7];
    message.grp_address = bm_group_address.mFields.m16[7];
    message.net_time_ack = 0;
    message.message_id = bm_get_overflow_tid_from_overflow_handler(bm_message_ID, message.source_address);
    otNetworkTimeGet(thread_ot_instance_get(), &message.net_time);

    uint8_t bm_big_probe[DATA_SIZE];
    uint8_t bm_small_probe[3] = {bm_message_ID, message.source_address & 0xff, message.source_address >> 8 };

    do
    {
        if (otIp6IsAddressUnspecified(&bm_group_address))
        {
            NRF_LOG_INFO("Failed to send coap test message: No group address found");
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
            error = otMessageAppend(p_request, &bm_small_probe, 3*sizeof(uint8_t));
            message.data_size = 3;
        } else if (state == BM_1024Bytes)
        {
            otRandomNonCryptoFillBuffer(bm_big_probe, DATA_SIZE);
            bm_big_probe[0] = bm_small_probe[0];
            bm_big_probe[1] = bm_small_probe[1];
            bm_big_probe[2] = bm_small_probe[2];
            error = otMessageAppend(p_request, &bm_big_probe, (DATA_SIZE*sizeof(uint8_t)));
            message.data_size = 1024;
        }

        if (error != OT_ERROR_NONE)
        {
            break;
        }

        bm_save_message_info(message);
        memset(&messafe_info, 0, sizeof(messafe_info));
        messafe_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        messafe_info.mAllowZeroHopLimit = false;
        messafe_info.mHopLimit = HOP_LIMIT_DEFAULT;
        memcpy(&messafe_info.mPeerAddr, &bm_group_address, sizeof(messafe_info.mPeerAddr));
                 
        error = otCoapSendRequest(p_instance, p_request, &messafe_info, NULL, p_instance);
        
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

    otIp6AddressFromString("ff00::28", &bm_master_address);

    otError error = otCoapStart(p_instance, OT_DEFAULT_COAP_PORT);
    ASSERT(error == OT_ERROR_NONE);

    otCoapSetDefaultHandler(p_instance, coap_default_handler, NULL);

    m_config = *p_config;

    error = otIp6AddressFromString(bm_group_address_array[0], &bm_group_address);
    ASSERT(error == OT_ERROR_NONE);

    error = app_timer_create(&m_slave_timer, APP_TIMER_MODE_SINGLE_SHOT, m_slave_timer_handler);
    ASSERT(error == NRF_SUCCESS);

    if (m_config.coap_server_enabled)
    {
        m_bm_test_resource.mContext = p_instance;
        error = otCoapAddResource(p_instance, &m_bm_test_resource);
        ASSERT(error == OT_ERROR_NONE);

        error = otIp6SubscribeMulticastAddress(thread_ot_instance_get(), &bm_group_address);
        ASSERT(error == OT_ERROR_NONE);
    }

    if (m_config.coap_client_enabled || m_config.coap_server_enabled)
    {
        m_bm_start_resource.mContext    = p_instance;
        error = otCoapAddResource(p_instance, &m_bm_start_resource);
        ASSERT(error == OT_ERROR_NONE);

        m_bm_result_request_resource.mContext    = p_instance;
        error = otCoapAddResource(p_instance, &m_bm_result_request_resource);
        ASSERT(error == OT_ERROR_NONE);
    }

    if (!m_config.coap_client_enabled || !m_config.coap_server_enabled)
    {
        error = otIp6SubscribeMulticastAddress(p_instance, &bm_master_address);
        ASSERT(error == OT_ERROR_NONE);

        m_bm_result_resource.mContext   = p_instance;
        error = otCoapAddResource(p_instance, &m_bm_result_resource);
        ASSERT(error == OT_ERROR_NONE);

        m_bm_start_request_resource.mContext = p_instance;
        error = otCoapAddResource(p_instance, &m_bm_start_request_resource);
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

        m_bm_result_request_resource.mContext    = NULL;
        otCoapRemoveResource(p_instance, &m_bm_result_request_resource);
    }
    
    if (!m_config.coap_client_enabled || !m_config.coap_server_enabled)
    {
        m_bm_result_resource.mContext   = NULL;
        otCoapRemoveResource(p_instance, &m_bm_result_resource);

        m_bm_start_request_resource.mContext   = NULL;
        otCoapRemoveResource(p_instance, &m_bm_start_request_resource);
    }

    bm_group_nr = 0;
    error = otIp6AddressFromString("0", &bm_group_address);
    ASSERT(error == OT_ERROR_NONE);
}