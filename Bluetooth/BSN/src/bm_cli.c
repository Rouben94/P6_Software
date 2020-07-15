#include "bm_cli.h"
#include <zephyr.h>

void bm_cli_log(const char *fmt, ...){
    // Zephyr way to Log info
    
    printk(fmt);
    //k_sleep(K_MSEC(10)); // Somehow free time for the printing Thread
    // ToDo Log for nRF5 SDK
    return;
}