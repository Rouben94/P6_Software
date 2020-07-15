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

    for(int i=0; i<8; i++)
    {
        char buf9[5] = {"\0"};
        char buf10[5] = {"\0"};

        if(i==7)
        {
            sprintf(buf9, "%x", message_info.source_address.mFields.m16[i]);
            sprintf(buf10, "%x", message_info.dest_address.mFields.m16[i]);
        } else
        {
            sprintf(buf9, "%x:", message_info.source_address.mFields.m16[i]);
            sprintf(buf10, "%x:", message_info.dest_address.mFields.m16[i]);
        }

        if(message_info.source_address.mFields.m16[i] == 0)
        {
            sprintf(buf9, ":", message_info.source_address.mFields.m16[i]);
        }

        if(message_info.dest_address.mFields.m16[i] == 0)
        {
            sprintf(buf10, ":", message_info.dest_address.mFields.m16[i]);
        }

        strncat(buf6, buf9, 5);
        strncat(buf7, buf10, 5);
    }

    sprintf(buf1, "ID: %u, ", message_info.message_id);
    sprintf(buf2, "Time: %u us, ", message_info.net_time);
    sprintf(buf3, "Ack_Time: %u us, ", message_info.net_time_ack);
    sprintf(buf4, "Hops: %u, ", message_info.number_of_hops);
    sprintf(buf5, "RSSI: %d dB, ", message_info.RSSI);
    sprintf(buf6, "Source: %s, ", buf6);
    sprintf(buf7, "Dest: %s, ", buf7);

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
    otCliOutput(buf6, sizeof(buf3));
    otCliOutput(buf7, sizeof(buf4));
    otCliOutput(buf8, sizeof(buf5));
}