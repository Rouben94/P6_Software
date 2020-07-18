#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_RADIO_H
#define BM_RADIO_H

#include "bm_config.h"

#include <hal/nrf_radio.h>

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
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

bool bm_radio_receive(RADIO_PACKET *rx_pkt, uint32_t timeout_ms);



#ifdef __cplusplus
}
#endif
#endif