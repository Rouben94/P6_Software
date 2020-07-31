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

#include <stdint.h>
#include <stdbool.h>

  
  typedef struct
  {
    uint32_t MACAddressDst; // Controll Message Belongs to a Specific address or can be braodcast (0xFFFFFFFF)
    uint8_t depth; // Depth Relativ to Master (ignore Messages which have a higher depth)
    uint8_t NextStateNr; // The State which the Master Requests -> 0 is ignored
    uint16_t benchmark_time_s; // Set the Benchmark Time -> 0 is ignored
    uint16_t benchmark_packet_cnt; // Set the Benchmark Packets Count -> 0 is irgnored
    uint8_t GroupAddress; // Set the Group Address of the Dst -> 0 is ignored
    uint8_t NodeId; // Set the NodeId of the Dst -> 0 is ignored
  } __attribute__((packed)) bm_control_msg_t;

void bm_control_msg_publish(bm_control_msg_t bm_control_msg);

bool bm_control_msg_subscribe(bm_control_msg_t * bm_control_msg);

#endif

#ifdef __cplusplus
}
#endif