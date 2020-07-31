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


#include "bm_config.h"
#include "bm_radio.h"
#include "bm_timesync.h"

#include <hal/nrf_radio.h>
#include <string.h>

#ifdef ZEPHYR_BLE_MESH

/* ---------------------- RADIO AREA Zephyr ------------------------ */

/** Maximum radio RX or TX payload. */
#define RADIO_MAX_PAYLOAD_LEN 256
/** IEEE 802.15.4 maximum payload length. */
#define IEEE_MAX_PAYLOAD_LEN 127

typedef struct
{
  uint8_t length;
  uint8_t PDU[RADIO_MAX_PAYLOAD_LEN - 1];     // Max Size of PDU
} __attribute__((packed)) PACKET_PDU_ALIGNED; // Attribute informing compiler to pack all members to 8bits or 1byte

static PACKET_PDU_ALIGNED tx_pkt_aligned;

typedef struct /* For IEEE The address has to be transmitted explicit */
{
  uint8_t length;
  uint32_t address;
  uint8_t PDU[IEEE_MAX_PAYLOAD_LEN - 4 - 1];       // Max Size of PDU
} __attribute__((packed)) PACKET_PDU_ALIGNED_IEEE; // Attribute informing compiler to pack all members to 8bits or 1byte

static PACKET_PDU_ALIGNED_IEEE tx_pkt_aligned_IEEE;

static int const BLE_CH_freq[40] = {2404, 2406, 2408, 2410, 2412, 2414, 2416, 2418, 2420, 2422, 2424, 2428, 2430, 2432, 2434, 2436, 2438, 2440, 2442, 2444, 2446, 2448, 2450, 2452, 2454, 2456, 2458, 2460, 2462, 2464, 2466, 2468, 2470, 2472, 2474, 2476, 2478, 2402, 2426, 2480};
static int const IEEE802_15_4_CH_freq[16] = {2405, 2410, 2415, 2420, 2425, 2430, 2435, 2440, 2445, 2450, 2455, 2460, 2465, 2470, 2475, 2480}; // List of IEEE802.15.4 Channels

/* Define Packet Buffers */
static uint8_t *tx_buf;                                // Pointer to the used Tx Buffer Payload
static uint8_t tx_buf_len;                             // Length of the Tx Buffer
static uint8_t rx_buf_ieee[IEEE_MAX_PAYLOAD_LEN] = {}; // Rx Buffer for IEEE operation
static uint8_t rx_buf[RADIO_MAX_PAYLOAD_LEN] = {};     // Rx Buffer for BLE operation
static uint8_t *rx_buf_ptr;
static uint32_t address; // Storing the Access Address

void bm_radio_clock_init() {
  NRF_CLOCK->TASKS_HFCLKSTART = 1;    //Start high frequency clock
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0; //Clear event
}

void bm_radio_disable(void) {
  nrf_radio_shorts_set(NRF_RADIO, 0);
  nrf_radio_int_disable(NRF_RADIO, ~0);
  nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_DISABLED);

  nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_DISABLE);
  while (!nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_DISABLED)) {
    /* Do nothing */
  }
  nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_DISABLED);
}

void bm_radio_setMode(nrf_radio_mode_t m) {
  // Set the desired Radio Mode
  nrf_radio_mode_set(NRF_RADIO, m);
  // Enable Fast Ramp Up (no TIFS) and set Tx Default mode to Center
  nrf_radio_modecnf0_set(NRF_RADIO, true, RADIO_MODECNF0_DTX_Center);
  // CRC16-CCITT Conform
  nrf_radio_crc_configure(NRF_RADIO, RADIO_CRCCNF_LEN_Three, NRF_RADIO_CRC_ADDR_SKIP, 0x00065B);
  nrf_radio_crcinit_set(NRF_RADIO, 0x555555);
  // Packet configuration
  nrf_radio_packet_conf_t packet_conf;
  memset(&packet_conf, 0, sizeof(packet_conf));
  packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_8BIT; // 8-bit preamble
  packet_conf.lflen = 8;                             // lengthfield size = 8 bits
  packet_conf.s0len = 0;                             // S0 size = 0 bytes
  packet_conf.s1len = 0;                             // S1 size = 0 bits
  packet_conf.maxlen = 255;                          // max 255-byte payload
  packet_conf.statlen = 0;                           // 0-byte static length
  packet_conf.balen = 3;                             // 3-byte base address length (4-byte full address length)
  packet_conf.big_endian = false;                    // Bit 24: 1 Small endian
  packet_conf.whiteen = true;                        // Bit 25: 1 Whitening enabled
  packet_conf.crcinc = 0;                            // Indicates if LENGTH field contains CRC or not
  switch (m) {
  case NRF_RADIO_MODE_BLE_1MBIT:
    // Nothing to be done :)
    break;
  case NRF_RADIO_MODE_BLE_2MBIT:
    packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_16BIT; // 16-bit preamble
    break;
  case NRF_RADIO_MODE_BLE_LR500KBIT:
  case NRF_RADIO_MODE_BLE_LR125KBIT:
    packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_LONG_RANGE; // 10-bit preamble
    packet_conf.cilen = 2;                                   // Length of code indicator (Bits) - long range
    packet_conf.termlen = 3;                                 // Length of TERM field (Bits) in Long Range operation
    packet_conf.maxlen = IEEE_MAX_PAYLOAD_LEN;               // max byte payload
    // Enable Fast Ramp Up (no TIFS) and set Tx Defalut mode to Center
    nrf_radio_modecnf0_set(NRF_RADIO, true, RADIO_MODECNF0_DTX_Center);
    // CRC24-Bit
    nrf_radio_crc_configure(NRF_RADIO, RADIO_CRCCNF_LEN_Three, NRF_RADIO_CRC_ADDR_SKIP, 0);
    break;
  case NRF_RADIO_MODE_IEEE802154_250KBIT:
    packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_32BIT_ZERO; // 32-bit preamble
    packet_conf.balen = 0;                                   // --> Address Matching is not suppported in IEEE802.15.4 mode
    packet_conf.maxlen = IEEE_MAX_PAYLOAD_LEN;               // max 127-byte payload
    packet_conf.lflen = 8;                                   // lengthfield size = 8 bits
    packet_conf.crcinc = 1;                                  // Indicates if LENGTH field contains CRC or not
    // Enable Fast Ramp Up (no TIFS) and set Tx Defalut mode to Center
    nrf_radio_modecnf0_set(NRF_RADIO, true, RADIO_MODECNF0_DTX_Center);
    // CRC 16-bit ITU-T conform
    nrf_radio_crc_configure(NRF_RADIO, RADIO_CRCCNF_LEN_Two, NRF_RADIO_CRC_ADDR_IEEE802154, 0x11021);
    nrf_radio_crcinit_set(NRF_RADIO, 0x5555);
  default:
    break;
  }
  nrf_radio_packet_configure(NRF_RADIO, &packet_conf);
}

void bm_radio_setCH(uint8_t CH) {
  // Accept only numbers within 0-39 for BLE Channels
  if ((39 >= CH && CH >= 0) && nrf_radio_mode_get(NRF_RADIO) != NRF_RADIO_MODE_IEEE802154_250KBIT) {
    nrf_radio_frequency_set(NRF_RADIO, BLE_CH_freq[CH]);
    nrf_radio_datawhiteiv_set(NRF_RADIO, CH);
    //bm_cli_log("Set Frequency to %d\n", BLE_CH_freq[CH]);
  } else if (26 >= CH && CH >= 11) // Accept only numbers within 11-26 for IEEE802.15.4 Channels
  {
    nrf_radio_frequency_set(NRF_RADIO, IEEE802_15_4_CH_freq[CH - 11]);
  }
}

void bm_radio_setTxP(nrf_radio_txpower_t TxP) {
  nrf_radio_txpower_set(NRF_RADIO, TxP);
}

void bm_radio_setAA(uint32_t aa) {
  address = aa; // Store the Access Address
  /* Set the device address 0 to use when transmitting. */
  nrf_radio_txaddress_set(NRF_RADIO, 0);
  /* Enable the device address 0 to use to select which addresses to
	 	* receive
	 	*/
  nrf_radio_rxaddresses_set(NRF_RADIO, 1);
  /* Set the access address */
  nrf_radio_prefix0_set(NRF_RADIO, (aa >> 24) & RADIO_PREFIX0_AP0_Msk);
  nrf_radio_base0_set(NRF_RADIO, (aa << 8) & 0xFFFFFF00);
  //nrf_radio_sfd_set(NRF_RADIO, (aa >> 24) & RADIO_PREFIX0_AP0_Msk); // Set the SFD for the IEEE Radio Mode to the Prefix (first byte of Address) -> Dont do because of CCA Carrier Mode
}

bool bm_radio_crcok_int = false;
/* Zephyr Way */
/*
static void bm_radio_handler(){
    bm_cli_log("Interrupt\n");
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_CRCOK)){
        bm_radio_crcok_int = true;
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
    k_wakeup(wakeup_thread_tid);
}
*/
/*
ISR_DIRECT_DECLARE(bm_radio_handler)
{
    //bm_cli_log("Interrupt\n");
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_CRCOK)){
        bm_radio_crcok_int = true;
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
    k_wakeup(wakeup_thread_tid);
    ISR_DIRECT_PM();
    return 1;
}
*/

/* NRF SDK WAY 
void RADIO_IRQHandler(void){
    //bm_cli_log("Interrupt\n");
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_CRCOK)){
        bm_radio_crcok_int = true;
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
}
*/

void bm_radio_init() {
  // Enable the High Frequency clock on the processor. This is a pre-requisite for
  // the RADIO module. Without this clock, no communication is possible.
  bm_radio_clock_init();
  nrf_radio_power_set(NRF_RADIO, 1);          // Power ON Radio
  bm_radio_disable();                         // Disable Radio
  bm_radio_setMode(NRF_RADIO_MODE_BLE_1MBIT); // Set Mode to BLE 1MBITS
  bm_radio_setAA(0x8E89BED6);                 // Default Advertisment Address BLE 0x8E89BED6
  bm_radio_setCH(11);                         // Default Advertisment Channel
  bm_radio_setTxP(NRF_RADIO_TXPOWER_0DBM);    // Set Tx Power to 0dbm
                                              //IRQ_DIRECT_CONNECT(RADIO_IRQn, 6, bm_radio_handler, 0); // Connect Radio ISR Zephyr WAY
                                              //irq_connect_dynamic(RADIO_IRQn, 6, bm_radio_handler, NULL, 0); // Connect Radio ISR Zephyr WAY
                                              //irq_enable(RADIO_IRQn);                                 // Enable Radio ISR Zephyr WAY
                                              // NVIC_EnableIRQ(RADIO_IRQn);                               // Enable Radio ISR NRF SDK WAY
}

void bm_radio_send(RADIO_PACKET tx_pkt) {
  bm_radio_disable(); // Disable the Radio
  /* Setup Paket */
  if (nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_IEEE802154_250KBIT) {
    tx_pkt_aligned_IEEE.length = tx_pkt.length + 2 + sizeof(address);                  // Because Length includes CRC Field
    tx_pkt_aligned_IEEE.address = address;                                             // Save  address because IEEE wont transmit it by itself
    memset(tx_pkt_aligned_IEEE.PDU, 0, sizeof(tx_pkt_aligned_IEEE.PDU));               // Initialize Data Structure
    memcpy(tx_pkt_aligned_IEEE.PDU, tx_pkt.PDU, tx_pkt.length);                        // Copy the MAC PDU to the RAM PDU
    tx_buf = (u8_t *)&tx_pkt_aligned_IEEE;                                             // Set the Tx Buffer Pointer
    tx_buf_len = tx_pkt.length + sizeof(address) + sizeof(tx_pkt_aligned_IEEE.length); // Save the Size of the tx_buf
  } else {
    tx_pkt_aligned.length = tx_pkt.length;
    memset(tx_pkt_aligned.PDU, 0, sizeof(tx_pkt_aligned.PDU));          // Initialize Data Structure
    memcpy(tx_pkt_aligned.PDU, tx_pkt.PDU, tx_pkt.length);              // Copy the MAC PDU to the RAM PDU
    tx_buf = (u8_t *)&tx_pkt_aligned;                                   // Set the Tx Buffer Pointer
    tx_buf_len = tx_pkt_aligned.length + sizeof(tx_pkt_aligned.length); // Save the Size of the tx_buf
  }
  nrf_radio_packetptr_set(NRF_RADIO, tx_buf);
  nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_PHYEND_DISABLE_MASK); // Start after Ready and Disable after END or PHY-End
  nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_DISABLED);
  nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_TXEN);
  while (!(nrf_radio_state_get(NRF_RADIO) == 0)) // Check for Disabled State
  {
    __NOP(); // Since the LLL Driver owns the Interrupt we have to Poll
             //__WFE(); // Wait for Timer Interrupt nRF5SDK Way
             // k_sleep(K_FOREVER); // Zephyr Way
  }
  bm_radio_disable();
}

void bm_radio_send_burst(RADIO_PACKET tx_pkt,uint32_t burst_time_ms) { 
  bm_synctimer_timeout_compare_int = false;
    synctimer_setCompareInt(burst_time_ms); 
    while (!(bm_synctimer_timeout_compare_int)) {
      bm_radio_send(tx_pkt);
    }
    bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
}

bool bm_radio_receive(RADIO_PACKET *rx_pkt, uint32_t timeout_ms) {
  bm_radio_disable();
  /* Initialize Rx Buffer */
  if (nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_IEEE802154_250KBIT) {
    memset(rx_buf_ieee, 0, sizeof(rx_buf_ieee)); // Erase old Rx buffer content
    rx_buf_ptr = rx_buf_ieee;
  } else {
    memset(rx_buf, 0, sizeof(rx_buf)); // Erase old Rx buffer content
    rx_buf_ptr = rx_buf;
  }
  nrf_radio_packetptr_set(NRF_RADIO, rx_buf_ptr);
  nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);                                                                                                                                // Clear CRCOK Event                                                                                                                                         // Set Interrupt Thread to wakeup
  nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_ADDRESS_RSSISTART_MASK | NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_PHYEND_DISABLE_MASK); // Start after Ready and Start after END -> wait for CRCOK Event otherwise keep on receiving
  nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RXEN);
  bm_synctimer_timeout_compare_int = false;
  bm_radio_crcok_int = false; // Reset Interrupt Flags
  synctimer_setCompareInt(timeout_ms);
  while (!(bm_synctimer_timeout_compare_int)) {
    //__SEV();__WFE();__WFE(); // Wait for Timer Interrupt nRF5SDK Way
    __NOP(); // Since the LLL Driver owns the Interrupt we have to Poll
    if (nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_CRCOK) && nrf_radio_state_get(NRF_RADIO) == NRF_RADIO_STATE_DISABLED && ((PACKET_PDU_ALIGNED *)rx_buf_ptr)->length == rx_pkt->length) {
      rx_pkt->PDU = ((PACKET_PDU_ALIGNED *)rx_buf_ptr)->PDU;
      rx_pkt->length = ((PACKET_PDU_ALIGNED *)rx_buf_ptr)->length;
      bm_radio_disable();
      nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RSSISTOP);
      nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);
      bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
      return true;
    }
  }
  bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
  nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RSSISTOP);
  bm_radio_disable();
  return false;
}

#elif defined NRF_SDK_ZIGBEE

/* ---------------------- RADIO AREA NRF5SDK_Zigbee ------------------------ */

/** Maximum radio RX or TX payload. */
#define RADIO_MAX_PAYLOAD_LEN 256
/** IEEE 802.15.4 maximum payload length. */
#define IEEE_MAX_PAYLOAD_LEN 127

typedef struct
{
  uint8_t length;
  uint8_t PDU[RADIO_MAX_PAYLOAD_LEN - 1];     // Max Size of PDU
} __attribute__((packed)) PACKET_PDU_ALIGNED; // Attribute informing compiler to pack all members to 8bits or 1byte

static PACKET_PDU_ALIGNED tx_pkt_aligned;

typedef struct /* For IEEE The address has to be transmitted explicit */
{
  uint8_t length;
  uint32_t address;
  uint8_t PDU[IEEE_MAX_PAYLOAD_LEN - 4 - 1];       // Max Size of PDU
} __attribute__((packed)) PACKET_PDU_ALIGNED_IEEE; // Attribute informing compiler to pack all members to 8bits or 1byte

static PACKET_PDU_ALIGNED_IEEE tx_pkt_aligned_IEEE;

static int const BLE_CH_freq[40] = {2404, 2406, 2408, 2410, 2412, 2414, 2416, 2418, 2420, 2422, 2424, 2428, 2430, 2432, 2434, 2436, 2438, 2440, 2442, 2444, 2446, 2448, 2450, 2452, 2454, 2456, 2458, 2460, 2462, 2464, 2466, 2468, 2470, 2472, 2474, 2476, 2478, 2402, 2426, 2480};
static int const IEEE802_15_4_CH_freq[16] = {2405, 2410, 2415, 2420, 2425, 2430, 2435, 2440, 2445, 2450, 2455, 2460, 2465, 2470, 2475, 2480}; // List of IEEE802.15.4 Channels

/* Define Packet Buffers */
static uint8_t *tx_buf;                                // Pointer to the used Tx Buffer Payload
static uint8_t tx_buf_len;                             // Length of the Tx Buffer
static uint8_t rx_buf_ieee[IEEE_MAX_PAYLOAD_LEN] = {}; // Rx Buffer for IEEE operation
static uint8_t rx_buf[RADIO_MAX_PAYLOAD_LEN] = {};     // Rx Buffer for BLE operation
static uint8_t *rx_buf_ptr;
static uint32_t address; // Storing the Access Address

void bm_radio_clock_init() {
  NRF_CLOCK->TASKS_HFCLKSTART = 1;    //Start high frequency clock
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0; //Clear event
}

void bm_radio_disable(void) {
  nrf_radio_shorts_set(0);
  nrf_radio_int_disable(~0);
  nrf_radio_event_clear(NRF_RADIO_EVENT_DISABLED);

  nrf_radio_task_trigger(NRF_RADIO_TASK_DISABLE);
  while (!nrf_radio_event_check(NRF_RADIO_EVENT_DISABLED)) {
    /* Do nothing */
  }
  nrf_radio_event_clear(NRF_RADIO_EVENT_DISABLED);
}

void bm_radio_setMode(nrf_radio_mode_t m) {
  // Set the desired Radio Mode
  nrf_radio_mode_set(m);
  // Enable Fast Ramp Up (no TIFS) and set Tx Default mode to Center
  nrf_radio_modecnf0_set(true, RADIO_MODECNF0_DTX_Center);
  // CRC16-CCITT Conform
  nrf_radio_crc_configure(RADIO_CRCCNF_LEN_Three, NRF_RADIO_CRC_ADDR_SKIP, 0x00065B);
  nrf_radio_crcinit_set(0x555555);
  // Packet configuration
  nrf_radio_packet_conf_t packet_conf;
  memset(&packet_conf, 0, sizeof(packet_conf));
  packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_8BIT; // 8-bit preamble
  packet_conf.lflen = 8;                             // lengthfield size = 8 bits
  packet_conf.s0len = 0;                             // S0 size = 0 bytes
  packet_conf.s1len = 0;                             // S1 size = 0 bits
  packet_conf.maxlen = 255;                          // max 255-byte payload
  packet_conf.statlen = 0;                           // 0-byte static length
  packet_conf.balen = 3;                             // 3-byte base address length (4-byte full address length)
  packet_conf.big_endian = false;                    // Bit 24: 1 Small endian
  packet_conf.whiteen = true;                        // Bit 25: 1 Whitening enabled
  packet_conf.crcinc = 0;                            // Indicates if LENGTH field contains CRC or not
  switch (m) {
  case NRF_RADIO_MODE_BLE_1MBIT:
    // Nothing to be done :)
    break;
  case NRF_RADIO_MODE_BLE_2MBIT:
    packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_16BIT; // 16-bit preamble
    break;
  case NRF_RADIO_MODE_BLE_LR500KBIT:
  case NRF_RADIO_MODE_BLE_LR125KBIT:
    packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_LONG_RANGE; // 10-bit preamble
    packet_conf.cilen = 2;                                   // Length of code indicator (Bits) - long range
    packet_conf.termlen = 3;                                 // Length of TERM field (Bits) in Long Range operation
    packet_conf.maxlen = IEEE_MAX_PAYLOAD_LEN;               // max byte payload
    // Enable Fast Ramp Up (no TIFS) and set Tx Defalut mode to Center
    nrf_radio_modecnf0_set(true, RADIO_MODECNF0_DTX_Center);
    // CRC24-Bit
    nrf_radio_crc_configure(RADIO_CRCCNF_LEN_Three, NRF_RADIO_CRC_ADDR_SKIP, 0);
    break;
  case NRF_RADIO_MODE_IEEE802154_250KBIT:
    packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_32BIT_ZERO; // 32-bit preamble
    packet_conf.balen = 0;                                   // --> Address Matching is not suppported in IEEE802.15.4 mode
    packet_conf.maxlen = IEEE_MAX_PAYLOAD_LEN;               // max 127-byte payload
    packet_conf.lflen = 8;                                   // lengthfield size = 8 bits
    packet_conf.crcinc = 1;                                  // Indicates if LENGTH field contains CRC or not
    // Enable Fast Ramp Up (no TIFS) and set Tx Defalut mode to Center
    nrf_radio_modecnf0_set(true, RADIO_MODECNF0_DTX_Center);
    // CRC 16-bit ITU-T conform
    nrf_radio_crc_configure(RADIO_CRCCNF_LEN_Two, NRF_RADIO_CRC_ADDR_IEEE802154, 0x11021);
    nrf_radio_crcinit_set(0x5555);
  default:
    break;
  }
  nrf_radio_packet_configure(&packet_conf);
}

void bm_radio_setCH(uint8_t CH) {
  // Accept only numbers within 0-39 for BLE Channels
  if ((39 >= CH && CH >= 0) && nrf_radio_mode_get() != NRF_RADIO_MODE_IEEE802154_250KBIT) {
    nrf_radio_frequency_set(BLE_CH_freq[CH]);
    nrf_radio_datawhiteiv_set(CH);
    //bm_cli_log("Set Frequency to %d\n", BLE_CH_freq[CH]);
  } else if (26 >= CH && CH >= 11) // Accept only numbers within 11-26 for IEEE802.15.4 Channels
  {
    nrf_radio_frequency_set(IEEE802_15_4_CH_freq[CH - 11]);
  }
}

void bm_radio_setTxP(nrf_radio_txpower_t TxP) {
  nrf_radio_txpower_set(TxP);
}

void bm_radio_setAA(uint32_t aa) {
  address = aa; // Store the Access Address
  /* Set the device address 0 to use when transmitting. */
  nrf_radio_txaddress_set(0);
  /* Enable the device address 0 to use to select which addresses to
	 	* receive
	 	*/
  nrf_radio_rxaddresses_set(1);
  /* Set the access address */
  nrf_radio_prefix0_set((aa >> 24) & RADIO_PREFIX0_AP0_Msk);
  nrf_radio_base0_set((aa << 8) & 0xFFFFFF00);
  //nrf_radio_sfd_set( (aa >> 24) & RADIO_PREFIX0_AP0_Msk); // Set the SFD for the IEEE Radio Mode to the Prefix (first byte of Address) -> Dont do because of CCA Carrier Mode
}

bool bm_radio_crcok_int = false;
/* Zephyr Way */
/*
static void bm_radio_handler(){
    bm_cli_log("Interrupt\n");
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_CRCOK)){
        bm_radio_crcok_int = true;
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
    k_wakeup(wakeup_thread_tid);
}
*/
/*
ISR_DIRECT_DECLARE(bm_radio_handler)
{
    //bm_cli_log("Interrupt\n");
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_CRCOK)){
        bm_radio_crcok_int = true;
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
    k_wakeup(wakeup_thread_tid);
    ISR_DIRECT_PM();
    return 1;
}
*/

/* NRF SDK WAY 
void RADIO_IRQHandler(void){
    //bm_cli_log("Interrupt\n");
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_CRCOK)){
        bm_radio_crcok_int = true;
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
}
*/

void bm_radio_init() {
  // Enable the High Frequency clock on the processor. This is a pre-requisite for
  // the RADIO module. Without this clock, no communication is possible.
  bm_radio_clock_init();
  nrf_radio_power_set(1);                     // Power ON Radio
  bm_radio_disable();                         // Disable Radio
  bm_radio_setMode(NRF_RADIO_MODE_BLE_1MBIT); // Set Mode to BLE 1MBITS
  bm_radio_setAA(0x8E89BED6);                 // Default Advertisment Address BLE 0x8E89BED6
  bm_radio_setCH(11);                         // Default Advertisment Channel
  bm_radio_setTxP(NRF_RADIO_TXPOWER_0DBM);    // Set Tx Power to 0dbm
                                              //IRQ_DIRECT_CONNECT(RADIO_IRQn, 6, bm_radio_handler, 0); // Connect Radio ISR Zephyr WAY
                                              //irq_connect_dynamic(RADIO_IRQn, 6, bm_radio_handler, NULL, 0); // Connect Radio ISR Zephyr WAY
                                              //irq_enable(RADIO_IRQn);                                 // Enable Radio ISR Zephyr WAY
                                              // NVIC_EnableIRQ(RADIO_IRQn);                               // Enable Radio ISR NRF SDK WAY
}

void bm_radio_send(RADIO_PACKET tx_pkt) {
  bm_radio_disable(); // Disable the Radio
  /* Setup Paket */
  if (nrf_radio_mode_get() == NRF_RADIO_MODE_IEEE802154_250KBIT) {
    tx_pkt_aligned_IEEE.length = tx_pkt.length + 2 + sizeof(address);                  // Because Length includes CRC Field
    tx_pkt_aligned_IEEE.address = address;                                             // Save  address because IEEE wont transmit it by itself
    memset(tx_pkt_aligned_IEEE.PDU, 0, sizeof(tx_pkt_aligned_IEEE.PDU));               // Initialize Data Structure
    memcpy(tx_pkt_aligned_IEEE.PDU, tx_pkt.PDU, tx_pkt.length);                        // Copy the MAC PDU to the RAM PDU
    tx_buf = (uint8_t *)&tx_pkt_aligned_IEEE;                                          // Set the Tx Buffer Pointer
    tx_buf_len = tx_pkt.length + sizeof(address) + sizeof(tx_pkt_aligned_IEEE.length); // Save the Size of the tx_buf
  } else {
    tx_pkt_aligned.length = tx_pkt.length;
    memset(tx_pkt_aligned.PDU, 0, sizeof(tx_pkt_aligned.PDU));          // Initialize Data Structure
    memcpy(tx_pkt_aligned.PDU, tx_pkt.PDU, tx_pkt.length);              // Copy the MAC PDU to the RAM PDU
    tx_buf = (uint8_t *)&tx_pkt_aligned;                                // Set the Tx Buffer Pointer
    tx_buf_len = tx_pkt_aligned.length + sizeof(tx_pkt_aligned.length); // Save the Size of the tx_buf
  }
  nrf_radio_packetptr_set(tx_buf);
  nrf_radio_shorts_enable(NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_PHYEND_DISABLE_MASK); // Start after Ready and Disable after END or PHY-End
  nrf_radio_event_clear(NRF_RADIO_EVENT_DISABLED);
  nrf_radio_task_trigger(NRF_RADIO_TASK_TXEN);
  while (!(nrf_radio_state_get() == 0)) // Check for Disabled State
  {
    __NOP(); // Since the LLL Driver owns the Interrupt we have to Poll
             //__WFE(); // Wait for Timer Interrupt nRF5SDK Way
             // k_sleep(K_FOREVER); // Zephyr Way
  }
  bm_radio_disable();
}



void bm_radio_send_burst(RADIO_PACKET tx_pkt,uint32_t burst_time_ms) { 
  bm_synctimer_timeout_compare_int = false;
    synctimer_setCompareInt(burst_time_ms); 
    while (!(bm_synctimer_timeout_compare_int)) {
      bm_radio_send(tx_pkt);
    }
    bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
}


bool bm_radio_receive(RADIO_PACKET *rx_pkt, uint32_t timeout_ms) {
  bm_radio_disable();
  /* Initialize Rx Buffer */
  if (nrf_radio_mode_get() == NRF_RADIO_MODE_IEEE802154_250KBIT) {
    memset(rx_buf_ieee, 0, sizeof(rx_buf_ieee)); // Erase old Rx buffer content
    rx_buf_ptr = rx_buf_ieee;
  } else {
    memset(rx_buf, 0, sizeof(rx_buf)); // Erase old Rx buffer content
    rx_buf_ptr = rx_buf;
  }
  nrf_radio_packetptr_set(rx_buf_ptr);
  nrf_radio_event_clear(NRF_RADIO_EVENT_CRCOK);                                                                                                                                // Clear CRCOK Event                                                                                                                                         // Set Interrupt Thread to wakeup
  nrf_radio_shorts_enable(NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_ADDRESS_RSSISTART_MASK | NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_PHYEND_DISABLE_MASK); // Start after Ready and Start after END -> wait for CRCOK Event otherwise keep on receiving
  nrf_radio_task_trigger(NRF_RADIO_TASK_RXEN);
  bm_synctimer_timeout_compare_int = false;
  synctimer_setCompareInt(timeout_ms);
  while (!(bm_synctimer_timeout_compare_int)) {
    //__SEV();__WFE();__WFE(); // Wait for Timer Interrupt nRF5SDK Way
    __NOP(); // Since the LLL Driver owns the Interrupt we have to Poll
    if (nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_CRCOK) && nrf_radio_state_get(NRF_RADIO) == NRF_RADIO_STATE_DISABLED && ((PACKET_PDU_ALIGNED *)rx_buf_ptr)->length == rx_pkt->length) {
      rx_pkt->PDU = ((PACKET_PDU_ALIGNED *)rx_buf_ptr)->PDU;
      rx_pkt->length = ((PACKET_PDU_ALIGNED *)rx_buf_ptr)->length;
      bm_radio_disable();
      nrf_radio_task_trigger(NRF_RADIO_TASK_RSSISTOP);
      nrf_radio_event_clear(NRF_RADIO_EVENT_CRCOK);
      bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
      return true;
    }
  }
  bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
  nrf_radio_task_trigger(NRF_RADIO_TASK_RSSISTOP);
  bm_radio_disable();
  return false;
}

#endif