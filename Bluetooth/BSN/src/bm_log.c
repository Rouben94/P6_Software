#include "bm_config.h"
#include "bm_log.h"
#include "bm_cli.h"

#include <string.h>

// Save the Loged Data to
bm_message_info message_info[NUMBER_OF_BENCHMARK_REPORT_MESSAGES] = {0};

uint16_t log_entry_cnt; // Count the Log Entry 

void bm_clear_log(){
    log_entry_cnt = 0;
    memset(message_info, 0, sizeof(message_info));
}

void bm_append_log(bm_message_info msginfo){
    if (log_entry_cnt > sizeof(message_info) -1) {
        bm_cli_log("Log Buffer Overflow... Ignoring Data\n");
        return;
    }
    message_info[log_entry_cnt] = msginfo;
    log_entry_cnt++;
}

