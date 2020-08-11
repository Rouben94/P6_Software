/*
This file is part of Benchmark-Shared-Library.

Benchmark-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchmark-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchmark-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR	   :   Raffael Anklin        */

#include "bm_control.h"
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_radio.h"
#include "bm_rand.h"
#include "bm_simple_buttons_and_leds.h"
#include "bm_timesync.h"

#include <hal/nrf_timer.h>
#include <nrfx_timer.h>

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_ZIGBEE
#endif

#define ControlAddress 0xA3F79C12
#define CommonMode NRF_RADIO_MODE_BLE_LR125KBIT       // Common Mode
#define CommonStartCH 37                              // Common Start Channel
#define CommonEndCH 39                                // Common End Channel
#define CommonCHCnt (CommonEndCH - CommonStartCH + 1) // Common Channel Count
// Selection Between nrf52840 and nrf5340 of TxPower -> The highest available for Common Channel is recomended
#if defined(RADIO_TXPOWER_TXPOWER_Pos8dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_POS8DBM // Common Tx Power < 8 dBm
#elif defined(RADIO_TXPOWER_TXPOWER_0dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_0DBM // Common Tx Power < 0 dBm
#endif

#define pub_time_on_ch_ms 10                                              // Time Sending Timesync Packets on one Channel
#define sub_time_on_ch_ms (pub_time_on_ch_ms * CommonCHCnt)               // Subscribe Time on one Channel (~15ms)
#define pub_sub_time_ms (pub_time_on_ch_ms * CommonCHCnt * CommonCHCnt)   // Publishing or Subscribing Time ms (~45ms)
#define backoff_time_max_ms (1000)                                        // Calculate with probability of collisions
#define depth_valid_time_ms (10 * (backoff_time_max_ms + pub_sub_time_ms)) // Maximum Time the Depth field is valid to not relay messages with a higher depth

static RADIO_PACKET Radio_Packet_TX, Radio_Packet_RX;

static uint64_t end_send_ts_us;
static uint8_t last_depth_received = 255;             // The last Received Depth -> Master will send 0 so the slave must have the highest possibel at init
static uint64_t last_depth_received_valid_till_ts_us; // The last Received Depth is valid till this timestamp
static uint64_t local_backoff_time_us;

void bm_control_msg_publish(bm_control_msg_t bm_control_msg) {
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(ControlAddress);
  bm_radio_setTxP(CommonTxPower);
  Radio_Packet_TX.length = sizeof(bm_control_msg);
  Radio_Packet_TX.PDU = (uint8_t *)&bm_control_msg;
  local_backoff_time_us = (bm_rand_32 % backoff_time_max_ms) * 1000; // Local Backoff time
  bm_sleep(local_backoff_time_us / 1000);                            // Sleep Backoff Time
  for (int ch_rx = CommonStartCH; ch_rx <= CommonEndCH; ch_rx++) {
    for (int ch = CommonStartCH; ch <= CommonEndCH; ch++) {
      bm_radio_setCH(CommonStartCH + ((ch + bm_rand_32) % CommonCHCnt)); // Random Channel Start
      end_send_ts_us = synctimer_getSyncTime() + pub_time_on_ch_ms * 1000;
      //bm_radio_send_burst(Radio_Packet_TX, msg_time_ms * msg_cnt);
      while (end_send_ts_us > synctimer_getSyncTime()) { // Do while Message Time is not exceeded and at least two Packets are sent
        bm_radio_send(Radio_Packet_TX);
      }
    }
  }
}

bool bm_control_msg_subscribe(bm_control_msg_t *bm_control_msg) {
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(ControlAddress);
  bm_radio_setTxP(CommonTxPower);
  Radio_Packet_RX.length = sizeof(*bm_control_msg);
  while (true) {
    for (int ch = CommonStartCH; ch <= CommonEndCH; ch++) {
      bm_radio_setCH(ch);
      if (bm_radio_receive(&Radio_Packet_RX, sub_time_on_ch_ms)) {
        *bm_control_msg = *(bm_control_msg_t *)Radio_Packet_RX.PDU; // Bring the sheep to a dry place
        bm_cli_log("Control Message received depth: %u last received depth: %u\n", (*bm_control_msg).depth, last_depth_received);
        /* Chek if Relay Message */
        if ((last_depth_received >= (*bm_control_msg).depth) || (synctimer_getSyncTime() >= last_depth_received_valid_till_ts_us)) {
          /* Relay Message */
          last_depth_received = (*bm_control_msg).depth; // Set last received depth
          bm_cli_log("set last depth: %u\n", (*bm_control_msg).depth);
          bm_cli_log("depth: %u\n", (*bm_control_msg).depth);
          (*bm_control_msg).depth++; // Increase Depth
          bm_cli_log("depth increased: %u\n", (*bm_control_msg).depth);
          last_depth_received_valid_till_ts_us = synctimer_getSyncTime() + depth_valid_time_ms * 1000; // Set Depth Valid Time
          bm_control_msg_publish(*bm_control_msg);
          bm_cli_log("Control Message relayed with outgoing depth: %u\n", (*bm_control_msg).depth);
        }
        bm_sleep(local_backoff_time_us / 1000); // Sleep Backoff Time
        return true;
      }
    }
  }
  return false;
}