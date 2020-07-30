#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "bm_statemachine.h"
#include "bm_master_cli.h"
#include "bm_coap.h"
#include "nrf_log.h"

#include <openthread/cli.h>
#include <openthread/thread.h>

void bm_cli_write_result(bm_message_info message_info)
{
    char buf[9][50] = {"\0"};
    char bm_result_cli[100] = {"\0"};

    strcpy(bm_result_cli, "<report> ");
    sprintf(buf[0], "%u ", message_info.message_id);
    sprintf(buf[1], "%u ", message_info.net_time);
    sprintf(buf[2], "%u ", message_info.net_time_ack);
    sprintf(buf[3], "%u ", message_info.number_of_hops);
    sprintf(buf[4], "%d ", message_info.RSSI);
    sprintf(buf[5], "%x ", message_info.source_address.mFields.m16[7]);
    sprintf(buf[6], "%x ", message_info.dest_address.mFields.m16[7]);
    sprintf(buf[7], "%x ", message_info.grp_address.mFields.m16[7]);

    if (message_info.data_size)
    {
        sprintf(buf[8], "1024\r\n");
    } else if (!message_info.data_size)
    {
        sprintf(buf[8], "3\r\n");
    }

    for(int i=0; i<9; i++)
    {
        strcat(bm_result_cli, buf[i]);
    }

    otCliOutput(bm_result_cli, sizeof(bm_result_cli));
}