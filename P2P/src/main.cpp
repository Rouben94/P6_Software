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

#include <string.h>
#include <stdlib.h>
#include <vector>
#include <zephyr.h>
#include <shell/shell.h>
#include <hal/nrf_radio.h>
#include <drivers/clock_control.h>
#include <drivers/clock_control/nrf_clock_control.h>
#include "simple_nrf_radio.h"

/* ------------- Definitions --------------*/
#define isMaster 0								   	// Node is the Master (1) or Slave (0)
#define CommonMode NRF_RADIO_MODE_BLE_LR125KBIT 	// Common Mode
#define CommonStartCH 37						   	// Common Start Channel 
#define CommonEndCH 39						   		// Common End Channel 
#define MSB_MAC_Address 0xF4CE						// MSB of MAC for Nordic Semi Vendor
// Master: Find Nodes by Broadcasting ---- Slave: Listen for Broadcasts and Answer with mockup. If already Discovered Sleep
#define ST_DISCOVERY 10	
// Master: Publish Settings and Timesync by Broadcasting ---- Slave: Listen for Settings and Timesync. If received go to sleep														
#define ST_PARAM 20
// Master: Publish Packets by Broadcasting with defined Settings ---- Slave: Listen for Packets with defined Settings. 
#define ST_PACKETS 30
// Master: Listen for Reports. Save Downlink RSSI ---- Slave: Send Report in Given Timeslot. Sleep before and after. 
#define ST_REPORTS 40
// Master: Time for Master to Publish the Reports to CLI  ---- Slave: Sleep
#define ST_PUBLISH 50
// Timeslots for the Sates in ms. The Timesync has to be accurate enough. 
#define ST_TIME_DISCOVERY_MS 200					
#define ST_TIME_PARAM_MS 50							
#define ST_TIME_PACKETS_MS 500						
#define ST_TIME_REPORTS_MS 200
#define ST_TIME_PUBLISH_MS 50
// Addresses used by State
#define DiscoveryAddress 0x8E89BED6
#define ParamAddress 0x8E89BED7
#define PacketsAddress 0x8E89BED8
#define ReportsAddress 0x8E89BED9
/*============================================*/

/* -------------- Data Container Definitions ------------------*/
struct DiscoveryPkt
{	
	u32_t LSB_MAC_Address; 
} __attribute__((packed));

struct ParamPkt
{	
	u32_t LastTxTimestamp;
	u8_t StartCH;
	u8_t StopCH;
	u8_t Mode;
	u8_t CCMA_CA;
} __attribute__((packed));

struct ReportsPkt
{	
	u32_t LSB_MAC_Address;
	u8_t CRCOK_CNT;
	u8_t CRCERR_CNT;
	u8_t Avg_SIG_RSSI;
	u8_t Avg_NOISE_RSSI;
} __attribute__((packed));

struct PublishList
{	
	u32_t LSB_MAC_Address;
	u8_t Pkt_CNT;
	u8_t CRCOK_CNT;
	u8_t CRCERR_CNT;
	u8_t Avg_SIG_RSSI;
	u8_t Avg_NOISE_RSSI;
};
/*===========================================*/


/* ------------------- Global Variables -----------------*/
Simple_nrf_radio simple_nrf_radio;
u32_t LSB_MAC_Address = NRF_FICR->DEVICEADDR[0];
RADIO_PACKET radio_pkt_Rx, radio_pkt_Tx = {};
u8_t currentState = ST_DISCOVERY;
/*=================================================*/


/*------------------ Shell ----------------------*/
int cmd_handler_send()
{
	char *Test_String = "C Programming is the shit";
	RADIO_PACKET s = {};
	s.PDU = (u8_t *)Test_String;
	s.length = strlen(Test_String) + 1; // Immer Länge + 1 bei String
	simple_nrf_radio.Send(s, K_MSEC(5000));
	return 0;
}
SHELL_CMD_REGISTER(send, NULL, "Send Payload", cmd_handler_send);
int cmd_handler_receive()
{
	RADIO_PACKET r;
	memset((u8_t *)&r, 0, sizeof(r));
	simple_nrf_radio.Receive((RADIO_PACKET *)&r, K_MSEC(10000));
	printk("Length %d\n", r.length);
	printk("%s\n", r.PDU);
	return 0;
}
SHELL_CMD_REGISTER(receive, NULL, "Receive Payload", cmd_handler_receive);
/*=================================================*/

// Define a Timer used for Discovery Sync
K_TIMER_DEFINE(timer1, NULL, NULL);

void main(void)
{
	// Init
	printk("Started \n");
	int res = simple_nrf_radio.RSSI(8);
	printk("RSSI: %d\n", res);

	// Init the Radio Packets
	DISCOVERY_RADIO_PACKET discPkt_Rx,discPkt_Tx = {};
	
	radio_pkt_Rx.length = sizeof(discPkt_Rx);
	radio_pkt_Rx.PDU = (u8_t *)&discPkt_Rx;
	radio_pkt_Tx.length = sizeof(discPkt_Tx);
	radio_pkt_Tx.PDU = (u8_t *)&discPkt_Tx;

	// Master / Slave Selection
	if (isMaster)
	{
		while (true)
		{
			switch (currentState)
			{
			case ST_DISCOVERY:
				simple_nrf_radio.setMode(DiscoveryMode);
				simple_nrf_radio.setAA(Broadcast_Address);
				for (int ch = DiscoveryStartCH; ch <= DiscoveryEndCH; ch++)
				{
					/* set radio channel */
					simple_nrf_radio.setCH(ch);
					discPkt_Tx.opcode = 0;
					/* start one shot timer that expires after 100 ms */
					k_timer_start(&timer1, K_MSEC(100), K_MSEC(0));
					// Burst Discovery Messages
					while (k_timer_remaining_get(&timer1) > 0)
					{
						discPkt_Tx.time_till_mockup_ms = k_timer_remaining_get(&timer1); //Get the Remaining time till mockup delay starts in ms
						simple_nrf_radio.Send(radio_pkt_Tx, 1000);
					}
					// Listen for Mockup Response Package
					s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, 100);
					/* check if a Discovery Packet was received */
					if (ret > 0) {
						printk("%d\n",((DISCOVERY_RADIO_PACKET*)radio_pkt_Rx.PDU)->opcode);
						printk("%d\n",((DISCOVERY_RADIO_PACKET*)radio_pkt_Rx.PDU)->address);
						// Send Superframe Allocation
						
					} else {
						currentState = ST_SLEEP;
					}
				}
				currentState = ST_SLEEP;
				break;
			case ST_SLEEP:
				// do something in the stop state
				k_sleep(1000);
				currentState = ST_DISCOVERY;
				break;
			}
		}
	}
	// Slave Code
	else
	{
		while (true)
		{
			switch (currentState)
			{
			case ST_DISCOVERY:
				simple_nrf_radio.setMode(DiscoveryMode);
				for (int ch = DiscoveryStartCH; ch <= DiscoveryEndCH; ch++)
				{
					/* set radio channel */
					simple_nrf_radio.setCH(ch);
					/* start receiving of Discovery Burst */
					s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(1000));
					/* check if a Discovery Packet was received */
					if (ret > 0) {
						if (((DISCOVERY_RADIO_PACKET*)radio_pkt_Rx.PDU)->opcode == 0)
						//Wait till Mockup Window
						k_sleep(K_MSEC(((DISCOVERY_RADIO_PACKET*)radio_pkt_Rx.PDU)->time_till_mockup_ms));
						// Wait for random time to send mockup answer
						k_sleep(sys_rand32_get()%100);
						discPkt_Tx.opcode = 01;
						simple_nrf_radio.Send(radio_pkt_Tx, 1000);
						// Wait for Superframe Allocation Info

					} else {
						continue;
					}
				}
				break;

			case ST_SLEEP:
				// do something in the sleep state
				k_sleep(1000);
				currentState = ST_DISCOVERY;
				break;
			}
		}
	}
}
