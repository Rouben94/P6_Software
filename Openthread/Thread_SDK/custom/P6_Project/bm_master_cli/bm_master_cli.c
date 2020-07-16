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
    char buf1[50] = {"\0"};
    char buf2[50] = {"\0"};
    char buf3[50] = {"\0"};
    char buf4[50] = {"\0"};
    char buf5[50] = {"\0"};
    char buf6[50] = {"\0"};
    char buf7[50] = {"\0"};
    char buf8[50] = {"\0"};

    sprintf(buf1, "ID: %u, ", message_info.message_id);
    sprintf(buf2, "Time: %u us, ", message_info.net_time);
    sprintf(buf3, "Ack_Time: %u us, ", message_info.net_time_ack);
    sprintf(buf4, "Hops: %u, ", message_info.number_of_hops);
    sprintf(buf5, "RSSI: %d dB, ", message_info.RSSI);
    sprintf(buf6, "Src: %x:%x:%x:%x:%x:%x:%x:%x, ", message_info.source_address.mFields.m16[0], message_info.source_address.mFields.m16[1], message_info.source_address.mFields.m16[2], message_info.source_address.mFields.m16[3], message_info.source_address.mFields.m16[4], message_info.source_address.mFields.m16[5], message_info.source_address.mFields.m16[6], message_info.source_address.mFields.m16[7]);
    sprintf(buf7, "Dst: %x:%x:%x:%x:%x:%x:%x:%x, ", message_info.dest_address.mFields.m16[0], message_info.dest_address.mFields.m16[1], message_info.dest_address.mFields.m16[2], message_info.dest_address.mFields.m16[3], message_info.dest_address.mFields.m16[4], message_info.dest_address.mFields.m16[5], message_info.dest_address.mFields.m16[6], message_info.dest_address.mFields.m16[7]);

    if (message_info.data_size)
    {
        sprintf(buf8, "Data: 1024 byte \r\n");
    } else if (!message_info.data_size)
    {
        sprintf(buf8, "Data: 1 bit \r\n");
    }
    
    otCliOutput(buf1, sizeof(buf1));
    otCliOutput(buf2, sizeof(buf2));
    otCliOutput(buf3, sizeof(buf3));
    otCliOutput(buf4, sizeof(buf4));
    otCliOutput(buf5, sizeof(buf5));
    otCliOutput(buf6, sizeof(buf6));
    otCliOutput(buf7, sizeof(buf7));
    otCliOutput(buf8, sizeof(buf8));
}