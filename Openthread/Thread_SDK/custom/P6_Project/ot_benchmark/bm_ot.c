
#include "bm_ot.h"
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_log.h"
#include "bm_simple_buttons_and_leds.h"
#include "bm_timesync.h"
#include "bm_rand.h"
#include "bm_board_support_thread.h"
#include "bm_board_support_config.h"
#include "thread_utils.h"

#include <openthread/thread.h>
#include <openthread/instance.h>
#include <openthread/coap.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/random_noncrypto.h>

#define NUMBER_OF_IP6_GROUPS 25
#define MAX_DATA_SIZE 1024
#define HOP_LIMIT_DEFAULT 64
#define MAX_NUMBER_OF_NODES 100
#define BM_OT_START_DELAY_MAX_MS 2000

//Define data here for lower ram usage in handler
bm_message_info message;
otMessageInfo coap_message_info;
otMessage * p_request;
otInstance * p_instance;
otError error;

// TID OVerflow Handler -> Allows an Overflow of of the TID when the next TID is dropping by 250...
// (uint8 overflows at 255 so there is a margin of 5) To Handle such case for all possibel Src.
// Adresses create the following struct and a array of the struct to save the last seen TID of each
// Addr.
typedef struct
{
    uint16_t src_addr;
    uint8_t last_TID_seen;
    uint8_t TID_OverflowCnt;
} __attribute__((packed)) bm_tid_overflow_handler_t;

/**@brief Forward declarations of CoAP resources handler from probe message. */
static void bm_probe_message_handler(void *, otMessage *, const otMessageInfo *);

/**@brief Definition of CoAP resources for benchmark test message. */
static otCoapResource m_bm_probe_resource = {
    .mUriPath = "bm_probe", .mHandler = bm_probe_message_handler, .mContext = NULL, .mNext = NULL};

// Groub address array. There should be 25 groups
const char * bm_group_address_array[NUMBER_OF_IP6_GROUPS] = {"ff03::3", "ff03::4", "ff03::5",
    "ff03::6", "ff03::7", "ff03::8", "ff03::9", "ff03::10", "ff03::11", "ff03::12", "ff03::13",
    "ff03::14", "ff03::15", "ff03::16", "ff03::17", "ff03::18", "ff03::19", "ff03::20", "ff03::21",
    "ff03::22", "ff03::23", "ff03::24", "ff03::25", "ff03::26", "ff03::27"};

// Excpect not more than MAX_NUMBER_OF_NODES Different Adresses
bm_tid_overflow_handler_t bm_tid_overflow_handler[MAX_NUMBER_OF_NODES];

// Global variable for accessing the group IPv6 address
otIp6Address bm_group_address = {0};

// Global variable for the probe message ID
uint8_t bm_message_ID = 0;

//Probemessage array in Byte format initalized for max DATA_SIZE
uint8_t bm_probe[MAX_DATA_SIZE];

/***************************************************************************************************
 * @section Thread Callback
 **************************************************************************************************/
 //Do something if thread stade is changed
static void thread_state_changed_callback(uint32_t flags, void * p_context)
{
    if (flags & OT_CHANGED_THREAD_ROLE)
    {
        switch (otThreadGetDeviceRole(p_context))
        {
            case OT_DEVICE_ROLE_CHILD:
            case OT_DEVICE_ROLE_ROUTER:
            case OT_DEVICE_ROLE_LEADER:
                break;

            case OT_DEVICE_ROLE_DISABLED:
            case OT_DEVICE_ROLE_DETACHED:
            default:
                break;
        }
    }

    bm_cli_log("State changed! Flags: 0x%08x Current role: %d\r\n", flags,
        otThreadGetDeviceRole(p_context));
}

/***************************************************************************************************
 * @section TID overflow handler
 **************************************************************************************************/
// Insert the tid and src address to get the merged tid with the tid overlfow cnt -> resulting in a
uint16_t bm_get_overflow_tid_from_overflow_handler(uint8_t tid, uint16_t src_addr)
{
    // Get the TID in array
    for (int i = 0; i < MAX_NUMBER_OF_NODES; i++)
    {
        if (bm_tid_overflow_handler[i].src_addr == src_addr)
        {
            // Check if Overflow happend
            if ((bm_tid_overflow_handler[i].last_TID_seen - tid) > 250)
            {
                bm_tid_overflow_handler[i].TID_OverflowCnt++;
            }
            // Add the last seen TID
            bm_tid_overflow_handler[i].last_TID_seen = tid;
            return (uint16_t)(bm_tid_overflow_handler[i].TID_OverflowCnt << 8) | (tid & 0xff);
        }
        else if (bm_tid_overflow_handler[i].src_addr == 0)
        {
            // Add the Src Adress
            bm_tid_overflow_handler[i].src_addr = src_addr;
            bm_tid_overflow_handler[i].last_TID_seen = tid;
            bm_tid_overflow_handler[i].TID_OverflowCnt = 0;
            return (uint16_t)(bm_tid_overflow_handler[i].TID_OverflowCnt << 8) | (tid & 0xff);
        }
    }
    return 0; // Default return 0
}

/***************************************************************************************************
 * @section Benchmark Coap handler for default messages
 **************************************************************************************************/
 //Handler if an unknown message is received
static void coap_default_handler(
    void * p_context, otMessage * p_message, const otMessageInfo * p_message_info)
{
    UNUSED_PARAMETER(p_context);
    UNUSED_PARAMETER(p_message);
    UNUSED_PARAMETER(p_message_info);

    bm_cli_log("Received CoAP message that does not match any request or resource\r\n");
}

/***************************************************************************************************
 * @section Benchmark Coap probe message.
 **************************************************************************************************/
 //Handler if a probe message is received
static void bm_probe_message_handler(
    void * p_context, otMessage * p_message, const otMessageInfo * p_message_info)
{
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

        message.data_size = otMessageRead(
            p_message, otMessageGetOffset(p_message), &bm_probe, 3 * sizeof(bm_probe));
        message.message_id = bm_get_overflow_tid_from_overflow_handler(
            bm_probe[0], ((bm_probe[1] & 0xff) | (bm_probe[2] << 8)));
        message.net_time = synctimer_getSyncTime();
        message.rssi = otMessageGetRss(p_message);
        message.number_of_hops = (HOP_LIMIT_DEFAULT - p_message_info->mHopLimit);
        message.src_addr = ((bm_probe[1] & 0xff) | (bm_probe[2] << 8));
        message.group_addr = bm_group_address.mFields.m16[7];
        bm_log_append_ram(message);

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            bm_cli_log("Server: OT_COAP_TYPE_CONFIRMABLE");
        }

        bm_led3_set(message.message_id % 2);

    } while (false);
}

//Function for sending a probe message with the CoAP module
void bm_coap_probe_message_send(uint16_t payload_len)
{
    bm_message_ID++;
    message.dst_addr = bm_group_address.mFields.m16[7];
    message.group_addr = bm_group_address.mFields.m16[7];
    message.message_id = bm_get_overflow_tid_from_overflow_handler(bm_message_ID, message.src_addr);
    message.net_time = synctimer_getSyncTime();

    do
    {
        if (otIp6IsAddressUnspecified(&bm_group_address))
        {
            bm_cli_log("Failed to send coap test message: No group address found");
            break;
        }

        p_request = otCoapNewMessage(p_instance, NULL);
        if (p_request == NULL)
        {
            bm_cli_log("Failed to allocate message for Coap request");
            break;
        }

        otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
        otCoapMessageGenerateToken(p_request, 2);
        error = otCoapMessageAppendUriPathOptions(p_request, "bm_probe");
        ASSERT(error == OT_ERROR_NONE);
        error = otCoapMessageSetPayloadMarker(p_request);
        ASSERT(error == OT_ERROR_NONE);

        otRandomNonCryptoFillBuffer(bm_probe, payload_len);
        bm_probe[0] = bm_message_ID;
        bm_probe[1] = message.src_addr & 0xff;
        bm_probe[2] = message.src_addr >> 8;
        error = otMessageAppend(p_request, &bm_probe, (payload_len * sizeof(uint8_t)));
        message.data_size = payload_len;

        if (error != OT_ERROR_NONE)
        {
            break;
        }
        bm_log_append_ram(message);

        memset(&coap_message_info, 0, sizeof(coap_message_info));
        coap_message_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        coap_message_info.mAllowZeroHopLimit = false;
        coap_message_info.mHopLimit = HOP_LIMIT_DEFAULT;
        memcpy(&coap_message_info.mPeerAddr, &bm_group_address, sizeof(coap_message_info.mPeerAddr));

        error = otCoapSendRequest(p_instance, p_request, &coap_message_info, NULL, p_instance);

    } while (false);

    if (error != OT_ERROR_NONE && p_request != NULL)
    {
        bm_cli_log("Failed to send Coap request: %d", error);
        otMessageFree(p_request);
    }
    bm_led2_set(message.message_id % 2);
}

void bm_send_message() { bm_coap_probe_message_send(bm_params.AdditionalPayloadSize + 3); }

/***************************************************************************************************
 * @section Benchmark Coap Initialization
 **************************************************************************************************/
void thread_coap_utils_init(const thread_coap_utils_configuration_t * p_config)
{
    p_instance = thread_ot_instance_get();

    error = otCoapStart(p_instance, OT_DEFAULT_COAP_PORT);
    ASSERT(error == OT_ERROR_NONE);

    otCoapSetDefaultHandler(p_instance, coap_default_handler, NULL);

    if (p_config->coap_server_enabled)
    {
        m_bm_probe_resource.mContext = p_instance;
        error = otCoapAddResource(p_instance, &m_bm_probe_resource);
        ASSERT(error == OT_ERROR_NONE);
    }
}

/***************************************************************************************************
 * @section Initialization
 **************************************************************************************************/
/**@brief Function for initializing the Thread Board Support Package
 */
static void thread_bsp_init(void)
{
    uint32_t error_code = bsp_thread_init(thread_ot_instance_get());
    APP_ERROR_CHECK(error_code);
}

/**@brief Function for initializing the Thread Stack.
 */
static void thread_instance_init(void)
{
    thread_configuration_t thread_configuration = {
        .radio_mode = THREAD_RADIO_MODE_RX_ON_WHEN_IDLE,
        .autocommissioning = true,
    };

    thread_init(&thread_configuration);
    thread_state_changed_callback_set(thread_state_changed_callback);
}

/**@brief Function for initializing the Constrained Application Protocol Module
 */
static void thread_coap_init(void)
{
#ifdef BENCHMARK_CLIENT
    message.src_addr = (*otThreadGetMeshLocalEid(thread_ot_instance_get())).mFields.m16[7];
    message.rssi = 0;
    message.ack_net_time = 0;
    message.number_of_hops = 0;
    thread_coap_utils_configuration_t thread_coap_configuration = {
        .coap_server_enabled = false,
        .coap_client_enabled = true,
    };
#endif // BM_CLIENT

#ifdef BENCHMARK_SERVER
    message.dst_addr = (*otThreadGetMeshLocalEid(p_instance)).mFields.m16[7];
    message.ack_net_time = 0;
    thread_coap_utils_configuration_t thread_coap_configuration = {
        .coap_server_enabled = true,
        .coap_client_enabled = false,
    };
#endif // BM_SERVER

#ifdef BENCHMARK_MASTER
    thread_coap_utils_configuration_t thread_coap_configuration = {
        .coap_server_enabled = false,
        .coap_client_enabled = false,
    };
#endif // BM_MASTER

    thread_coap_utils_init(&thread_coap_configuration);
}

/***************************************************************************************************
 * @section Init Openthread
 **************************************************************************************************/
void bm_ot_init()
{
#if defined BENCHMARK_CLIENT || defined BENCHMARK_SERVER
    bm_sleep(bm_rand_32%BM_OT_START_DELAY_MAX_MS);
#endif // BM_CLIENT OR BM_SERVER
    // Init Openthread stack
    thread_instance_init();
    thread_bsp_init();
    thread_coap_init();

    // Set Group
    uint32_t error =
        otIp6AddressFromString(bm_group_address_array[bm_params.GroupAddress], &bm_group_address);
    ASSERT(error == OT_ERROR_NONE);
#ifdef BENCHMARK_SERVER
    error = otIp6SubscribeMulticastAddress(thread_ot_instance_get(), &bm_group_address);
    ASSERT(error == OT_ERROR_NONE);
#endif
}