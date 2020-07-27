#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_CONTROL_H
#define BM_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

  
  typedef struct
  {
    uint32_t MACAddressDst; // Controll Message Belongs to a Specific address or can be braodcast (0xFFFFFFFF)
    uint8_t NextStateNr; // The State which the Master Requests -> 0 is ignored
    uint16_t benchmark_time_s; // Set the Benchmark Time -> 0 is ignored
    uint16_t benchmark_packet_cnt; // Set the Benchmark Packets Count -> 0 is irgnored
    uint8_t GroupAddress; // Set the Group Address of the Dst -> 0 is ignored
  } __attribute__((packed)) bm_control_msg_t;

void bm_control_msg_publish(bm_control_msg_t bm_control_msg);

bool bm_control_msg_subscribe(bm_control_msg_t * bm_control_msg);

#endif

#ifdef __cplusplus
}
#endif