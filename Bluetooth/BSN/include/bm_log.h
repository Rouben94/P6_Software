#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_LOG_H
#define BM_LOG_H

#include "bm_config.h"

/* Struct for benchmark message information */
typedef struct
{
  uint16_t message_id; 
  uint64_t net_time;
  uint64_t ack_net_time;
  uint8_t number_of_hops;
  uint8_t rssi;
  uint16_t src_addr;
  uint16_t dst_addr;
  uint16_t group_addr;
  uint16_t data_size;
} bm_message_info;

/* Array of structs to save benchmark message info to */
extern bm_message_info message_info[NUMBER_OF_BENCHMARK_REPORT_MESSAGES];

void bm_clear_log();

void bm_append_log(bm_message_info msginfo);



#ifdef __cplusplus
}
#endif
#endif