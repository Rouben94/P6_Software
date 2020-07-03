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

    sprintf(buf1, "ID: %u, ", message_info.message_id);
    sprintf(buf2, "Time: %u us, ", message_info.net_time);
    sprintf(buf3, "Hops: %u, ", message_info.number_of_hops);
    sprintf(buf4, "RSSI: %d dB, ", message_info.RSSI);

    if (message_info.data_size)
    {
        sprintf(buf5, "Data: 1024 byte \r\n");
    } else if (!message_info.data_size)
    {
        sprintf(buf5, "Data: 1 bit \r\n");
    }
    
    otCliOutput(buf1, sizeof(buf1));
    otCliOutput(buf2, sizeof(buf2));
    otCliOutput(buf3, sizeof(buf3));
    otCliOutput(buf4, sizeof(buf4));
    otCliOutput(buf5, sizeof(buf5));
}