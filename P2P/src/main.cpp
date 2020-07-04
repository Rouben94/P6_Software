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
#define isMaster 0								  // Node is the Master (1) or Slave (0)
#define CommonMode NRF_RADIO_MODE_BLE_LR125KBIT	  // Common Mode
#define CommonStartCH 37						  // Common Start Channel
#define CommonEndCH 39							  // Common End Channel
#define MSB_MAC_Address 0xF4CE					  // MSB of MAC for Nordic Semi Vendor
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
/*============================================*/

/* -------------- Data Container Definitions ------------------*/
struct DiscoveryPkt
{
	u64_t LastTxTimestamp;
	u64_t MockupTimestamp;
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
uint64_t MockupTS;
u8_t CHcnt = CommonEndCH - CommonStartCH;
u32_t LatestTxMasterTimestamp;
ParamPkt Param_pkt_RX, Param_pkt_TX = {};
ReportsPkt Repo_pkt_RX, Repo_pkt_TX = {};
vector<PublishList> pb_list;
u8_t currentState = ST_DISCOVERY;
k_tid_t ST_Machine_tid;
/*=================================================*/

/*------------------ Shell ----------------------*/
int cmd_handler_send()
{
	/*
	char *Test_String = "C Programming is the shit";
	RADIO_PACKET s = {};
	s.PDU = (u8_t *)Test_String;
	s.length = strlen(Test_String) + 1; // Immer Länge + 1 bei String
	simple_nrf_radio.Send(s, K_MSEC(5000));
	*/
	printk("%u%u\n",(uint32_t)(synctimer_getSyncTime()>>32),(uint32_t)synctimer_getSyncTime());
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

/*--------------------- States -------------------*/

static void ST_transition_cb(void){
	printk("%u%u\n",(uint32_t)(synctimer_getSyncTime()>>32),(uint32_t)synctimer_getSyncTime());
	currentState = ST_MOCKUP;
	k_thread_state_str(ST_Machine_tid);
	k_wakeup(ST_Machine_tid);
}


void ST_DISCOVERY_fn(void)
{
	/* start one shot timer that expires after Timeslot ms */
	k_timer_start(&state_timer, K_MSEC(ST_TIME_DISCOVERY_MS), K_MSEC(0));
	/* init */
	if (isMaster)
	{
		MockupTS = (synctimer_getSyncTime() + ST_TIME_DISCOVERY_MS*1000 + ST_TIME_MARGIN_MS*1000);
		synctimer_setSyncTimeCompareInt(MockupTS,ST_transition_cb);
	}
	simple_nrf_radio.setMode(CommonMode);
	simple_nrf_radio.setAA(DiscoveryAddress);
	synctimer_TimeStampCapture_clear();
	synctimer_TimeStampCapture_enable();
	radio_pkt_Tx.length = sizeof(Disc_pkt_TX);
	radio_pkt_Tx.PDU = (u8_t *)&Disc_pkt_TX;
	/* Do work and keep track of Time Left */
	for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		if (CHcnt == 0){
			CHcnt++;
		}
		k_timer_start(&uni_timer, K_MSEC(ST_TIME_DISCOVERY_MS / CHcnt), K_MSEC(0));
		while ((k_timer_remaining_get(&uni_timer) > 0) && (k_timer_remaining_get(&state_timer) > 0))
		{
			if (isMaster)
			{
				Disc_pkt_TX.MockupTimestamp = MockupTS;
				Disc_pkt_TX.LastTxTimestamp = synctimer_getTxTimeStamp();
				simple_nrf_radio.Send(radio_pkt_Tx, K_MSEC(k_timer_remaining_get(&uni_timer)));
			}
			else
			{
				s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&uni_timer)));
				if (ret > 0)
				{
					synctimer_TimeStampCapture_disable();
					// Keep on Receiving for the last Tx Timestamp
					s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&uni_timer)));
					if (ret > 0)
					{
						synctimer_setSync(((DiscoveryPkt *)radio_pkt_Rx.PDU)->LastTxTimestamp);
						synctimer_setSyncTimeCompareInt(((DiscoveryPkt *)radio_pkt_Rx.PDU)->MockupTimestamp,ST_transition_cb);
					}
					return;
				}
			}
		}
	}
	return;
}

// Wait for random time to send mockup answer
//k_sleep(sys_rand32_get() % 100);

/*=======================================================*/

void main(void)
{
	printk("P2P-Benchamrk Started\n");
	// Init Synctimer
	synctimer_init();
	synctimer_start();
	config_debug_ppi_and_gpiote_radio_state(); // For Debug Timesync


	// Statemachine
	ST_Machine_tid = k_current_get();
	while (true)
	{
		switch (currentState)
		{
		case ST_DISCOVERY:
			ST_DISCOVERY_fn();
			k_sleep(K_MSEC(ST_TIME_DISCOVERY_MS+ST_TIME_MARGIN_MS));			
			break;
		case ST_MOCKUP:
			k_sleep(K_MSEC(1000));
			currentState = ST_DISCOVERY;
			break;
		}
	}
}
