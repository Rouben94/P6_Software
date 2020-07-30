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

#define msg_time_ms 10            // Time needed for one message
#define msg_cnt 5                // Messages count used to transmit
#define backoff_time_max_ms 1000 // Calculate with probability of collisions

#define ControlAddress 0xA3F79C12
#define CommonMode NRF_RADIO_MODE_BLE_LR125KBIT         // Common Mode
#define CommonStartCH 37                            // Common Start Channel
#define CommonEndCH 39                              // Common End Channel
#define CommonCHCnt (CommonEndCH - CommonStartCH + 1) // Common Channel Count
// Selection Between nrf52840 and nrf5340 of TxPower -> The highest available for Common Channel is recomended
#if defined(RADIO_TXPOWER_TXPOWER_Pos8dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_POS8DBM // Common Tx Power < 8 dBm
#elif defined(RADIO_TXPOWER_TXPOWER_0dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_0DBM // Common Tx Power < 0 dBm
#endif

static RADIO_PACKET Radio_Packet_TX, Radio_Packet_RX;

void bm_control_msg_publish(bm_control_msg_t bm_control_msg) {
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(ControlAddress);
  bm_radio_setTxP(CommonTxPower);
  Radio_Packet_TX.length = sizeof(bm_control_msg);
  Radio_Packet_TX.PDU = (uint8_t *)&bm_control_msg;
  for (int ch_rx = CommonStartCH; ch_rx <= CommonEndCH; ch_rx++) {
    for (int ch = CommonStartCH; ch <= CommonEndCH; ch++) {
      bm_radio_setCH(ch);
      bm_radio_send_burst(Radio_Packet_TX, msg_time_ms * msg_cnt);
    }
  }
  bm_sleep(2 * backoff_time_max_ms + msg_time_ms * msg_cnt * CommonCHCnt * CommonCHCnt); // Sleep till all relays neerby should be done
}

bool bm_control_msg_subscribe(bm_control_msg_t *bm_control_msg) {
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(ControlAddress);
  bm_radio_setTxP(CommonTxPower);
  while (true) {
    for (int ch = CommonStartCH; ch <= CommonEndCH; ch++) {
      bm_radio_setCH(ch);
      if (bm_radio_receive(&Radio_Packet_RX, msg_time_ms * msg_cnt * CommonCHCnt)) {
        bm_cli_log("Control Message received\n");
        *bm_control_msg = *(bm_control_msg_t *)Radio_Packet_RX.PDU; // Bring the sheep to a dry place
        bm_sleep(bm_rand_32 % backoff_time_max_ms);                 // Sleep from 0 till Random Backoff Time
        /* Relay Message */
        Radio_Packet_TX.length = sizeof(bm_control_msg);
        Radio_Packet_TX.PDU = (uint8_t *)&bm_control_msg;
        for (int ch_rx = CommonStartCH; ch_rx <= CommonEndCH; ch_rx++) {
          for (int ch = CommonStartCH; ch <= CommonEndCH; ch++) {
            bm_radio_setCH(ch);
            bm_radio_send_burst(Radio_Packet_TX, msg_time_ms * msg_cnt);
          }
        }
        bm_cli_log("Control Message relayed\n");
        if (backoff_time_max_ms - (bm_rand_32 % backoff_time_max_ms) > msg_time_ms) {
          bm_sleep(backoff_time_max_ms - (bm_rand_32 % backoff_time_max_ms)); // Sleep the Rest of the Backoff Time
        }
        bm_sleep(backoff_time_max_ms); // Sleep for satfy to net get a backloop ...
        return true;
      }
    }
  }

  return false;
}