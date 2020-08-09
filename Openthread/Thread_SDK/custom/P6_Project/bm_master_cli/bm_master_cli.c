#include "bm_master_cli.h"
#include "bm_statemachine.h"
#include "nrf_log.h"

#include <openthread/cli.h>
#include <openthread/thread.h>
#include <openthread/network_time.h>

void bm_cli_write_result(bm_message_info message_info)
{
    char bm_result_cli[100] = {"\0"};

    sprintf(bm_result_cli, "<report> %u %" PRIu64 " %" PRIu64 " %u %d %x %x %x %d %d\r\n",
        message_info.message_id, message_info.net_time, message_info.net_time_ack,
        message_info.number_of_hops, message_info.RSSI, message_info.source_address,
        message_info.dest_address, message_info.grp_address, message_info.data_size, bm_get_node_id());

    otCliOutput(bm_result_cli, sizeof(bm_result_cli));
}