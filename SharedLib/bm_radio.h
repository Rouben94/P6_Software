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

/* AUTHOR   :   Raffael Anklin        */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_RADIO_H
#define BM_RADIO_H

#include "bm_config.h"

#include <hal/nrf_radio.h>

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee || defined NRF_SDK_THREAD || defined NRF_SDK_MESH
#endif

typedef struct
{
  uint8_t length;
  uint8_t *PDU;    // Pointer to PDU
  uint8_t Rx_RSSI; // Received RSSI of Packet
} RADIO_PACKET;

/* Initialize Radio */
void bm_radio_init();


void bm_radio_disable(void);

void bm_radio_setMode(nrf_radio_mode_t m);

void bm_radio_setCH(uint8_t CH);

void bm_radio_setAA(uint32_t aa);

void bm_radio_setTxP(nrf_radio_txpower_t TxP);


void bm_radio_send(RADIO_PACKET tx_pkt);

void bm_radio_send_burst(RADIO_PACKET tx_pkt,uint32_t burst_time_ms);

bool bm_radio_receive(RADIO_PACKET *rx_pkt, uint32_t timeout_ms);





#ifdef __cplusplus
}
#endif
#endif