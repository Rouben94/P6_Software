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

class Simple_nrf_radio
{
private:
	int BLE_CH_freq[40] = {2404, 2406, 2408, 2410, 2412, 2414, 2416, 2418, 2420, 2422, 2424, 2428, 2430, 2432, 2434, 2436, 2438, 2440, 2442, 2444, 2446, 2448, 2450, 2452, 2454, 2456, 2458, 2460, 2462, 2464, 2466, 2468, 2470, 2472, 2474, 2476, 2478, 2402, 2426, 2480};
	int IEEE802_15_4_CH_freq[16] = {2405, 2410, 2415, 2420, 2425, 2430, 2435, 2440, 2445, 2450, 2455, 2460, 2465, 2470, 2475, 2480}; // List of IEEE802.15.4 Channels
	k_tid_t ISR_Thread_ID;																											 //Thread ID waiting for ISR
	k_tid_t *ISR_Thread_ID_ptr;																										 //Pointer for Thread ID waiting for ISR
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
	 * @param payload Pointer what payload to send
	 * @param size Length of the Payload to Send
	 * @param timeout Waittimeout in ms for a packet to be sent
	 */
	void Send(RADIO_PACKET tx_pkt, int timeout);
	/**
	 * Receive a Payload
	 *
	 * @param payload Pointer where to store received payload
	 * @param timeout Waittimeout in ms for a packet to be received
	 * 
	 * @return Zero of timeout occured or number of miliseconds till timeout occurs
	 */
	s32_t Receive(RADIO_PACKET *rx_pkt, int timeout);
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