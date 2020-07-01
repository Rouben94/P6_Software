#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

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

void bm_cli_write_result(uint64_t time, uint16_t ID)
{
    char buf1[50] = {"\0"};
    char buf2[50] = {"\0"};

    sprintf(buf1, "ID %u: ", ID);
    sprintf(buf2, "%u \r\n", time);

    otCliOutput(buf1, sizeof(buf1));
    otCliOutput(buf2, sizeof(buf2));
}

/**@brief Function for initialize custom cli commands */
void bm_custom_cli_init(void){
    otCliSetUserCommands(bm_cli_usercommands, 2 * sizeof(bm_cli_usercommands[0]));
}