/*
This file is part of Benchamrk-Shared-Library.

Benchamrk-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchamrk-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchamrk-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR	   :   Raffael Anklin        */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_CONTROL_H
#define BM_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
  uint32_t MACAddressDst;         // Control Message belongs to a specific address or can be broadcast (0xFFFFFFFF)
  uint8_t depth;                  // Depth relativ to Master (ignore messages which have a higher depth)
  uint8_t NextStateNr;            // The state which the Master requests -> 0 is ignored
  uint16_t benchmark_time_s;      // Set the Benchmark Time -> 0 is ignored
  uint16_t benchmark_packet_cnt;  // Set the Benchmark Packets Count -> 0 is irgnored
  uint64_t GroupAddress;          // Set the Group Address of the Dst -> 0 is ignored
  uint8_t NodeId;                 // Set the NodeId of the Dst -> 0 is ignored
  uint16_t AdditionalPayloadSize; // Set the Additional Payload Size of the Dst -> 0 is ignored
  uint32_t DestMAC_1;             // Zigbee Directed Destination 1
  uint32_t DestMAC_2;             // Zigbee Directed Destination 2
  uint32_t DestMAC_3;             // Zigbee Directed Destination 3
  bool Ack;                       // Set the Ack of the Dst
  bool benchmark_Traffic_Generation_Mode;  // 0 = Random, 1 = Sequentialy
} __attribute__((packed)) bm_control_msg_t;

void bm_control_msg_publish(bm_control_msg_t bm_control_msg);

bool bm_control_msg_subscribe(bm_control_msg_t *bm_control_msg);

#endif

#ifdef __cplusplus
}
#endif