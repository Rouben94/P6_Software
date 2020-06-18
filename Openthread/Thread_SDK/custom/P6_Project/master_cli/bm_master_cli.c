#include "bm_master_cli.h"
#include "bm_coap.h"

#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "nrf_log_default_backends.h"

#include <openthread/cli.h>


void bm_cli_benchmark_start(uint8_t aArgsLength, char *aArgs[]);
void bm_cli_benchmark_stop(uint8_t aArgsLength, char *aArgs[]);

otCliCommand bm_cli_usercommands[2] = {
  {"benchmark_start", bm_cli_benchmark_start},
  {"benchmark_stop" , bm_cli_benchmark_stop}
};

bm_master_message master_message;

static thread_coap_utils_light_command_t m_command = THREAD_COAP_UTILS_LIGHT_CMD_OFF; /**< This variable stores command that has been most recently used. */

void bm_cli_benchmark_start(uint8_t aArgsLength, char *aArgs[]) {
    NRF_LOG_INFO("Benchmark start");

    master_message.bm_status = true;
//    master_message.bm_master_ip6_address = NULL;
    master_message.bm_time = 10;

    //m_command = ((m_command == THREAD_COAP_UTILS_LIGHT_CMD_OFF) ? THREAD_COAP_UTILS_LIGHT_CMD_ON : THREAD_COAP_UTILS_LIGHT_CMD_OFF);

    bm_coap_multicast_start_request_send(master_message, THREAD_COAP_UTILS_MULTICAST_REALM_LOCAL);
//    thread_coap_utils_multicast_light_request_send(m_command,
//        THREAD_COAP_UTILS_MULTICAST_REALM_LOCAL);

    otCliOutput("done \r\n", sizeof("done \r\n"));
}

void bm_cli_benchmark_stop(uint8_t aArgsLength, char *aArgs[]) {
    NRF_LOG_INFO("Benchmark stop");

    m_command = ((m_command == THREAD_COAP_UTILS_LIGHT_CMD_OFF) ? THREAD_COAP_UTILS_LIGHT_CMD_ON : THREAD_COAP_UTILS_LIGHT_CMD_OFF);
    thread_coap_utils_multicast_light_request_send(m_command,
        THREAD_COAP_UTILS_MULTICAST_REALM_LOCAL);

    otCliOutput("done \r\n", sizeof("done \r\n"));
}

/**@brief Function for initialize custom cli commands */
void bm_custom_cli_init(void){
    otCliSetUserCommands(bm_cli_usercommands, sizeof(bm_cli_usercommands));
}