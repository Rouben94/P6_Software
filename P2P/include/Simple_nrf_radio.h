/*
This file is part of P2P-Benchamrk.

P2P-Benchamrk is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

P2P-Benchamrk is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with P2P-Benchamrk.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Author: Raffael  */

#ifndef SIMPLE_NRF_RADIO_H
#define SIMPLE_NRF_RADIO_H

#include <stdlib.h>
#include <string.h>
#include <zephyr.h>
#include <hal/nrf_radio.h>
#include <drivers/clock_control.h>
#include <drivers/clock_control/nrf_clock_control.h>



/** Maximum radio RX or TX payload. */
#define RADIO_MAX_PAYLOAD_LEN 256
/** IEEE 802.15.4 maximum payload length. */
#define IEEE_MAX_PAYLOAD_LEN 127

struct RADIO_PACKET
{
	uint8_t length = 0;
	u8_t *PDU;			// Pointer to PDU
	u8_t Rx_RSSI = 174; // Received RSSI of Packet
};

struct TxPktStatLog
{
	u16_t Pktcnt = 0;
	bool act = false; // true if Logging is enabled
};

struct RxPktStatLog
{
	u16_t CRCOKcnt = 0;
	u16_t CRCErrcnt = 0;
	u16_t RSSI_Sum_Avg = 174; //Thermal Noise
	bool act = false; // true if Logging is enabled
};

struct Radio_Handler_Context
{
	RxPktStatLog rx_stat;
	k_tid_t* thread_id; //Pointer for Thread ID waiting for ISR
};

class Simple_nrf_radio
{
private:
	int BLE_CH_freq[40] = {2404, 2406, 2408, 2410, 2412, 2414, 2416, 2418, 2420, 2422, 2424, 2428, 2430, 2432, 2434, 2436, 2438, 2440, 2442, 2444, 2446, 2448, 2450, 2452, 2454, 2456, 2458, 2460, 2462, 2464, 2466, 2468, 2470, 2472, 2474, 2476, 2478, 2402, 2426, 2480};
	int IEEE802_15_4_CH_freq[16] = {2405, 2410, 2415, 2420, 2425, 2430, 2435, 2440, 2445, 2450, 2455, 2460, 2465, 2470, 2475, 2480}; // List of IEEE802.15.4 Channels
	k_tid_t ISR_Thread_ID;																											 //Thread ID waiting for ISR
	k_tid_t *ISR_Thread_ID_ptr;																										 //Pointer for Thread ID waiting for ISR
	Radio_Handler_Context rhctx;
	/* Define the Packet PDU which is sent or received by the radio */
	/* For info about Packing and unpacking see 
	https://www.geeksforgeeks.org/structure-member-alignment-padding-and-data-packing/?ref=rp
	*/
	struct PACKET_PDU_ALIGNED
	{
		uint8_t length;
		u8_t PDU[RADIO_MAX_PAYLOAD_LEN - sizeof(length)] = {}; // Max Size of PDU
	} __attribute__((packed)) tx_pkt_aligned;				   // Attribute informing compiler to pack all members to 8bits or 1byte
	/* For IEEE The address has to be transmitted explicit */
	struct PACKET_PDU_ALIGNED_IEEE
	{
		uint8_t length;
		u32_t address;
		u8_t PDU[IEEE_MAX_PAYLOAD_LEN - sizeof(address) - sizeof(length)] = {}; // Max Size of PDU
	} __attribute__((packed)) tx_pkt_aligned_IEEE;								// Attribute informing compiler to pack all members to 8bits or 1byte
	/* Define Packet Buffers */
	u8_t *tx_buf;								 // Pointer to the used Tx Buffer Payload
	uint8_t tx_buf_len;							 // Length of the Tx Buffer
	u8_t rx_buf_ieee[IEEE_MAX_PAYLOAD_LEN] = {}; // Rx Buffer for IEEE operation
	u8_t rx_buf[RADIO_MAX_PAYLOAD_LEN] = {};	 // Rx Buffer for BLE operation
	u8_t *rx_buf_ptr;
	u32_t address; // Storing the Access Address
public:
    /**
	 * Init the Radio
	 *
	 */
	Simple_nrf_radio();
	/**
	 * Radio ISR caused by radio interrupts
	 *
	 * @param context Data Container for the ISR.
	 */
	static void radio_handler(void *context);
	/**
	 * Enable HFCLK 
	 *
	 */
	static void clock_init(void);
	/**
	 * Meassure RSSI Value
	 *
	 * @param cycles Number of samples to average
	 */
	static int RSSI(u8_t cycles);
	/**
	 * Send a Payload
	 *
	 * @param tx_pkt Radio Paket to Send
	 * @param CCA_Mode 0 = Disabled / 1 = Mode 1 (energy above threshold) / 2 = Mode 2 (carrier sense only) / 3 = Mode 3 (carrier sense and threshold) / 4 = Mode 4 (carrier sense or threshold)
	 */
	void Send(RADIO_PACKET tx_pkt, u8_t CCA_Mode);
	/**
	 * Burst send out the same packet till timeout
	 *
	 * @param tx_pkt Radio Paket to Send
	 * @param CCA_Mode 0 = Disabled / 1 = Mode 1 (energy above threshold) / 2 = Mode 2 (carrier sense only) / 3 = Mode 3 (carrier sense and threshold) / 4 = Mode 4 (carrier sense or threshold)
	 * @param timeout Waittimeout in ms for bursting
     * @return Number of Packeets sent
	 */
	s32_t Receive(RADIO_PACKET *rx_pkt, k_timeout_t timeout);
	/**
	 * Receive Packets and only Log Data
	 *
	 * @param timeout Waittimeout in ms packet to be received and logged
     * 
	 * @return Struct of Log Data CRCOKcnt, CRCErrcnt, RSSIAvg
	 */
	RxPktStatLog ReceivePktStatLog(k_timeout_t timeout);
	/**
	 * Set the Radio Mode.
	 *
	 * @param m NRF Radio Mode to select (BLE or IEEE etc.)
	 */
	void setMode(nrf_radio_mode_t m);
	/**
	 * Set the Radio Channel
	 *
	 * @param CH Radio Channel to set (for BLE from 0-39 and IEEE802.15.4 from 11-26)
	 */
	void setCH(u8_t CH);
	/**
	 * Set the Tx Power of the Radio
	 *
	 * @param TxP TxPower to select
	 */
	void setTxP(nrf_radio_txpower_t TxP);
	/**
	 * Set the AccessAddress of the Radio
	 *
	 * @param aa AccessAddress to send or receive from. 
	 */
	void setAA(u32_t aa);
	/**
	 * Disable the Radio
	 *
	 */
	static void radio_disable(void);
};

#endif
