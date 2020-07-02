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
#include "Timer_sync.h"
using namespace std;

/* ------------- Definitions --------------*/
#define isMaster 0								// Node is the Master (1) or Slave (0)
#define CommonMode NRF_RADIO_MODE_BLE_LR125KBIT // Common Mode
#define CommonStartCH 37						// Common Start Channel
#define CommonEndCH 39							// Common End Channel
#define CommonCHCnt (CommonEndCH - CommonStartCH)	// Common Channel Count
#define MSB_MAC_Address 0xF4CE					// MSB of MAC for Nordic Semi Vendor
// Master: Find Nodes by Broadcasting ---- Slave: Listen for Broadcasts. If already Discovered Sleep
#define ST_DISCOVERY 10
// Master: Listen for Mockup Answers ---- Slave: Send a radnom delayed Mockup Answer. If already Discovered Sleep
#define ST_MOCKUP 20
// Master: Publish Settings and Timesync by Broadcasting ---- Slave: Listen for Settings and Timesync. If received go to sleep
#define ST_PARAM 30
// Master: Publish Packets by Broadcasting with defined Settings ---- Slave: Listen for Packets with defined Settings.
#define ST_PACKETS 40
// Master: Listen for Reports. Save Downlink RSSI ---- Slave: Send Report in Random Timeslot. Sleep before and after.
#define ST_REPORTS 50
// Master: Time for Master to Publish the Reports to CLI  ---- Slave: Sleep
#define ST_PUBLISH 60
// Timeslots for the Sates in ms. The Timesync has to be accurate enough. -> Optimized for 50 Nodes, 3 Channels and BLE LR125kBit
#define ST_TIME_DISCOVERY_MS 300
#define ST_TIME_MOCKUP_MS 1200
#define ST_TIME_PARAM_MS 60
#define ST_TIME_PACKETS_MS 600
#define ST_TIME_REPORTS_MS 900
#define ST_TIME_PUBLISH_MS 60
// Margin for State Transition (Let the State Terminate)
#define ST_TIME_MARGIN_MS 5
// Addresses used by State. Random Generated for good diversity.
#define DiscoveryAddress 0x9CE74F9A
#define MockupAddress 0x57855CC7
#define ParamAddress 0x32C731E9
#define PacketsAddress 0xF492A652
#define ReportsAddress 0x37DAFC9D
// Define a Timer used inside States for Timing
K_TIMER_DEFINE(state_timer, NULL, NULL);
// Define a Timer free for Timing
K_TIMER_DEFINE(uni_timer, NULL, NULL);
// Size of stack area used by each thread
#define ST_STACKSIZE 1024
// Scheduling priority used by each thread
#define ST_PRIORITY 7
// Define a Stack Area
K_THREAD_STACK_DEFINE(st_stack_area, ST_STACKSIZE);
/*============================================*/

/* -------------- Data Container Definitions ------------------*/
struct DiscoveryPkt
{
	u32_t LastTxTimestamp;
	u32_t MockupTimestamp;
} __attribute__((packed));

struct MockupPkt
{
	u32_t LSB_MAC_Address;
} __attribute__((packed));

struct ParamPkt
{
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

/* ------------------- Variables -----------------*/
Simple_nrf_radio simple_nrf_radio;
u32_t LSB_MAC_Address = NRF_FICR->DEVICEADDR[0];
RADIO_PACKET radio_pkt_Rx, radio_pkt_Tx = {};
DiscoveryPkt Disc_pkt_RX, Disc_pkt_TX = {};
u32_t MockupTS;
u32_t LatestTxMasterTimestamp;
ParamPkt Param_pkt_RX, Param_pkt_TX = {};
ReportsPkt Repo_pkt_RX, Repo_pkt_TX = {};
vector<PublishList> pb_list;
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

/*--------------------- State Threads -------------------*/

void TH_ST_DISCOVERY(void)
{
	/* start one shot timer that expires after Timeslot ms */
	k_timer_start(&state_timer, K_MSEC(ST_TIME_DISCOVERY_MS), K_MSEC(0));
	/* init */
	simple_nrf_radio.setMode(CommonMode);
	simple_nrf_radio.setAA(DiscoveryAddress);
	if (isMaster){
		MockupTS = synctimer_getSyncTime() + ST_TIME_DISCOVERY_MS + ST_TIME_MARGIN_MS;
	}
	radio_pkt_Tx.length = sizeof(Disc_pkt_TX);
	radio_pkt_Tx.PDU = (u8_t *)&Disc_pkt_TX;
	/* Do work and keep track of Time Left */
	for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		k_timer_start(&uni_timer, K_MSEC(k_timer_remaining_get(&state_timer)/CommonCHCnt), K_MSEC(0));
		while (k_timer_remaining_get(&uni_timer) > 0)
		{
			if (isMaster){
				Disc_pkt_TX.MockupTimestamp = MockupTS; 
				Disc_pkt_TX.LastTxTimestamp = synctimer_getTxTimeStamp(); 
				simple_nrf_radio.Send(radio_pkt_Tx, K_MSEC(k_timer_remaining_get(&uni_timer)));
			} else {
				s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&uni_timer)));
				if (ret > 0)
				{
					MockupTS = ((DiscoveryPkt *)radio_pkt_Rx.PDU)->MockupTimestamp;
					LatestTxMasterTimestamp = ((DiscoveryPkt *)radio_pkt_Rx.PDU)->MockupTimestamp;
					if (LatestTxMasterTimestamp == 0){ // The received Package was the first one. Keep on Receiving...
						s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&uni_timer)));
						if (ret > 0)
						{
							LatestTxMasterTimestamp = ((DiscoveryPkt *)radio_pkt_Rx.PDU)->MockupTimestamp;
						}
					}
					return;
				}
			}		
		}
	}
	return;
}

K_THREAD_DEFINE(TH_ST_DISCOVERY_id, STACKSIZE, TH_ST_DISCOVERY, NULL, NULL, NULL,
				PRIORITY, 0, 0);

/*=======================================================*/

void main(void)
{
	printk("P2P-Benchamrk Started\n");
	// Init

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

				// Listen for Mockup Response Package
				s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, 100);
				/* check if a Discovery Packet was received */
				if (ret > 0)
				{
					printk("%d\n", ((DISCOVERY_RADIO_PACKET *)radio_pkt_Rx.PDU)->opcode);
					printk("%d\n", ((DISCOVERY_RADIO_PACKET *)radio_pkt_Rx.PDU)->address);
					// Send Superframe Allocation
				}
				else
				{
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
				if (ret > 0)
				{
					if (((DISCOVERY_RADIO_PACKET *)radio_pkt_Rx.PDU)->opcode == 0)
						//Wait till Mockup Window
						k_sleep(K_MSEC(((DISCOVERY_RADIO_PACKET *)radio_pkt_Rx.PDU)->time_till_mockup_ms));
					// Wait for random time to send mockup answer
					k_sleep(sys_rand32_get() % 100);
					discPkt_Tx.opcode = 01;
					simple_nrf_radio.Send(radio_pkt_Tx, 1000);
					// Wait for Superframe Allocation Info
				}
				else
				{
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
