#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "bm_statemachine.h"
#include "bm_master_cli.h"
#include "bm_coap.h"
#include "nrf_log.h"

#include <openthread/cli.h>
#include <openthread/thread.h>


void bm_cli_benchmark_start(uint8_t aArgsLength, char *aArgs[]);
void bm_cli_benchmark_stop(uint8_t aArgsLength, char *aArgs[]);

otCliCommand bm_cli_usercommands[2] = {
  {"benchmark_start", bm_cli_benchmark_start},
  {"benchmark_stop" , bm_cli_benchmark_stop}
};

bm_master_message master_message;

void bm_cli_benchmark_start(uint8_t aArgsLength, char *aArgs[]) {
    NRF_LOG_INFO("Benchmark start");
    NRF_LOG_INFO("Argument: %s", aArgs[0]);

    master_message.bm_status = true;
    master_message.bm_master_ip6_address = *otThreadGetMeshLocalEid(thread_ot_instance_get());
    master_message.bm_time = (uint32_t)atoi(aArgs[0]);

    bm_coap_multicast_start_send(master_message);

    otCliOutput("done \r\n", sizeof("done \r\n"));
}

void bm_cli_benchmark_stop(uint8_t aArgsLength, char *aArgs[]) {
    NRF_LOG_INFO("Benchmark stop");
    
    master_message.bm_status = false;
    otIp6AddressFromString("0", &master_message.bm_master_ip6_address);
    master_message.bm_time = NULL;

    bm_coap_multicast_start_send(master_message);

    otCliOutput("done \r\n", sizeof("done \r\n"));
}

void bm_cli_write_result(bm_message_info message_info)
{
    char buf1[50] = {"\0"};
    char buf2[50] = {"\0"};
    char buf3[50] = {"\0"};
    char buf4[50] = {"\0"};

    sprintf(buf1, "ID: %u ", message_info.message_id);
    sprintf(buf2, "Time: %u ", message_info.net_time);
    sprintf(buf3, "Hops: %u ", message_info.number_of_hops);
    sprintf(buf4, "Data: %u \r\n", message_info.data_size);

    otCliOutput(buf1, sizeof(buf1));
    otCliOutput(buf2, sizeof(buf2));
    otCliOutput(buf3, sizeof(buf3));
    otCliOutput(buf4, sizeof(buf4));
}

/**@brief Function for initialize custom cli commands */
void bm_custom_cli_init(void){
    otCliSetUserCommands(bm_cli_usercommands, 2 * sizeof(bm_cli_usercommands[0]));
}