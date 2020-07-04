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

/* ------------- Definitions --------------*/
#define isMaster 0								// Node is the Master (1) or Slave (0)
#define CommonMode NRF_RADIO_MODE_BLE_LR125KBIT // Common Mode
#define CommonStartCH 37						// Common Start Channel
#define CommonEndCH 39							// Common End Channel
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
#define ST_TIME_PACKETS_MS 2000 // Worst Case 40 CHs, Size 255 and BLE LR 125kBit -> ca. 2 Packets
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
// Paramter Restrictions
#define MaxNodesCnt 50
#define MaxCHCnt 40
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
	u8_t Size;
	u8_t Mode;
	u8_t CCMA_CA;
} __attribute__((packed));

struct ReportsPkt
{
	u16_t CRCOK_CNT;
	u16_t CRCERR_CNT;
	u8_t Avg_SIG_RSSI;
	u8_t Avg_NOISE_RSSI;
} __attribute__((packed));

struct MasterReport
{
	u64_t TimeStamp;
	std::vector <NodeReport> nodrep;
};

struct NodeReport
{
	u32_t LSB_MAC_Address;
	std::vector <ChannelReport> chrep;
};

struct ChannelReport
{
	u16_t TxPkt_CNT;
	u16_t CRCOK_CNT;
	u16_t CRCERR_CNT;
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
u8_t CHcnt = CommonEndCH - CommonStartCH +1;
u32_t LatestTxMasterTimestamp;
bool Synced = false;

MockupPkt Mockup_pkt_RX, Mockup_pkt_TX = {};
bool GotReportReq = false;

ParamPkt Param_pkt_RX, Param_pkt_TX = {};
ParamPkt ParamLocal = {CommonStartCH,CommonEndCH,CommonMode,20,0};
bool GotParam = false;

std::vector <ChannelReport> chrep_local;

ReportsPkt Repo_pkt_RX, Repo_pkt_TX = {};
MasterReport masrep;

u8_t currentState = ST_DISCOVERY;
k_tid_t ST_Machine_tid;
/*=================================================*/

/*------------------ Shell ----------------------*/
static int cmd_handler_test()
{
	/*
	char *Test_String = "C Programming is the shit";
	RADIO_PACKET s = {};
	s.PDU = (u8_t *)Test_String;
	s.length = strlen(Test_String) + 1; // Immer Länge + 1 bei String
	simple_nrf_radio.Send(s, K_MSEC(5000));
	*/
	printk("%u%u\n", (uint32_t)(synctimer_getSyncTime() >> 32), (uint32_t)synctimer_getSyncTime());
	return 0;
}
SHELL_CMD_REGISTER(test, NULL, "Testing Command", cmd_handler_test);

static int cmd_setParams(const struct shell *shell, size_t argc, char **argv)
{	// Maybee at error check later for validating the Params are valid
	if (sizeof(argv) == sizeof(Param_pkt_TX) + 1)
	{
		ParamLocal.StartCH = *argv[1];
		ParamLocal.StopCH = *argv[2];
		ParamLocal.Mode = *argv[3];
		ParamLocal.Size = *argv[4];
		ParamLocal.CCMA_CA = *argv[5];
		shell_print(shell, "Done\n");
	}
	else
	{
		shell_error(shell, "Number of Arguments incorrect! expected:\n setParams <StartCH> <StopCH> <Mode> <Size> <CCMA_CA>\n");
	}
	return 0;
}
SHELL_CMD_REGISTER(setParams, NULL, "Set Start Channel", cmd_setParams);

/*=================================================*/

/*-------------- Get a Nodereport Index by its MAC -----------*/
static u8_t GetNodeidx(u32_t MAC){
	// Range based for
	u8_t idx;
	for(const auto& value: masrep.nodrep) {
		if (value.LSB_MAC_Address == MAC){
			return idx;
		}
	}
	return 0xFF; //Node not found
}


/*--------------------- States -------------------*/

static void ST_transition_cb(void)
{
	printk("%u%u\n", (uint32_t)(synctimer_getSyncTime() >> 32), (uint32_t)synctimer_getSyncTime());
	// Not Synced Slave Nodes stay in Discovery State.
	if ((currentState == ST_DISCOVERY && (Synced && !isMaster)) || (currentState == ST_DISCOVERY && isMaster))
	{
		currentState = ST_MOCKUP;
	}
	else if (currentState == ST_MOCKUP)
	{
		currentState = ST_PARAM;
	}
	// Only Nodes who received the Param can continue.
	else if ((currentState == ST_PARAM && isMaster) || (currentState == ST_PARAM && !isMaster && GotParam))
	{
		currentState = ST_PACKETS;
	}
	// Nodes who didn't received the Param go to Discovery State
	else if ((currentState == ST_PARAM && !isMaster && !GotParam))
	{
		currentState = ST_DISCOVERY;
	}
	else if (currentState == ST_PACKETS)
	{
		currentState = ST_REPORTS;
	}
	else if (currentState == ST_REPORTS)
	{
		currentState = ST_PUBLISH;
	}
	else if (currentState == ST_PUBLISH)
	{
		currentState = ST_DISCOVERY;
	}
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
		MockupTS = (synctimer_getSyncTime() + ST_TIME_DISCOVERY_MS * 1000 + ST_TIME_MARGIN_MS * 1000);
		synctimer_setSyncTimeCompareInt(MockupTS, ST_transition_cb);
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
		if (CHcnt == 0)
		{
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
						synctimer_setSyncTimeCompareInt(((DiscoveryPkt *)radio_pkt_Rx.PDU)->MockupTimestamp, ST_transition_cb);
						Synced = true;
						return;
					}
				}
				Synced = false;
			}
		}
	}
	return;
}

void ST_MOCKUP_fn(void)
{
	/* start one shot timer that expires after Timeslot ms */
	k_timer_start(&state_timer, K_MSEC(ST_TIME_MOCKUP_MS), K_MSEC(0));
	synctimer_setSyncTimeCompareInt((synctimer_getSyncTime() + ST_TIME_MOCKUP_MS * 1000 + ST_TIME_MARGIN_MS * 1000), ST_transition_cb);
	/* init */
	simple_nrf_radio.setMode(CommonMode);
	simple_nrf_radio.setAA(MockupAddress);
	radio_pkt_Tx.length = sizeof(Mockup_pkt_TX);
	radio_pkt_Tx.PDU = (u8_t *)&Mockup_pkt_TX;
	Mockup_pkt_TX.LSB_MAC_Address = LSB_MAC_Address;
	/* Do work and keep track of Time Left */
	for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		if (CHcnt == 0)
		{
			CHcnt++;
		}
		k_timer_start(&uni_timer, K_MSEC(ST_TIME_MOCKUP_MS / CHcnt), K_MSEC(0));
		while ((k_timer_remaining_get(&uni_timer) > 0) && (k_timer_remaining_get(&state_timer) > 0))
		{
			if (isMaster)
			{
				s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&uni_timer)));
				if (ret > 0)
				{
					if (GetNodeidx(((MockupPkt *)radio_pkt_Rx.PDU)->LSB_MAC_Address) == 0xFF){
						masrep.nodrep.push_back({((MockupPkt *)radio_pkt_Rx.PDU)->LSB_MAC_Address});
					}
				}
			}
			else if (!GotReportReq)
			{
				k_sleep(K_MSEC(sys_rand32_get() % (k_timer_remaining_get(&uni_timer) - 5))); // Sleep Random Mockup Delay minus 5ms send Margin
				simple_nrf_radio.Send(radio_pkt_Tx, K_MSEC(k_timer_remaining_get(&uni_timer)));
				if (ch != CommonEndCH)
				{
					k_sleep(K_MSEC(k_timer_remaining_get(&uni_timer))); // Sleep till next CH
				}
				else
				{
					return; // If was last channel Return and Sleep...
				}
			}
			else
			{
				// Node is known to Master. Can Return and Sleep..
				return;
			}
		}
	}
	return;
}

void ST_PARAM_fn(void)
{
	/* start one shot timer that expires after Timeslot ms */
	k_timer_start(&state_timer, K_MSEC(ST_TIME_PARAM_MS), K_MSEC(0));
	synctimer_setSyncTimeCompareInt((synctimer_getSyncTime() + ST_TIME_PARAM_MS * 1000 + ST_TIME_MARGIN_MS * 1000), ST_transition_cb);
	/* init */
	simple_nrf_radio.setMode(CommonMode);
	simple_nrf_radio.setAA(ParamAddress);
	radio_pkt_Tx.length = sizeof(Param_pkt_TX);
	radio_pkt_Tx.PDU = (u8_t *)&Param_pkt_TX;
	Param_pkt_TX = ParamLocal;
	/* Do work and keep track of Time Left */
	for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		if (CHcnt == 0)
		{
			CHcnt++;
		}
		k_timer_start(&uni_timer, K_MSEC(ST_TIME_PARAM_MS / CHcnt), K_MSEC(0));
		while ((k_timer_remaining_get(&uni_timer) > 0) && (k_timer_remaining_get(&state_timer) > 0))
		{
			if (isMaster)
			{
				simple_nrf_radio.Send(radio_pkt_Tx, K_MSEC(k_timer_remaining_get(&uni_timer)));
			} else {
				GotParam = false;
				s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&uni_timer)));
				if (ret > 0){
					ParamLocal = *((ParamPkt *)radio_pkt_Rx.PDU);
					GotParam = true;
					return;
				}
			}
		}
	}
	return;
}

void ST_PACKETS_fn(void)
{
	/* start one shot timer that expires after Timeslot ms */
	k_timer_start(&state_timer, K_MSEC(ST_TIME_PACKETS_MS), K_MSEC(0));
	synctimer_setSyncTimeCompareInt((synctimer_getSyncTime() + ST_TIME_PACKETS_MS * 1000 + ST_TIME_MARGIN_MS * 1000), ST_transition_cb);
	/* init */
	simple_nrf_radio.setMode((nrf_radio_mode_t)ParamLocal.Mode);
	simple_nrf_radio.setAA(PacketsAddress);
	// Node has to do all init to keep Time Sync
	radio_pkt_Tx.length = ParamLocal.Size;
	sys_rand_get(radio_pkt_Tx.PDU,ParamLocal.Size); // Fill the Packet with Random Data
	u8_t paramChCNT = (ParamLocal.StopCH - ParamLocal.StartCH);
	if (paramChCNT == 0){
		paramChCNT++;
	}	
	chrep_local.clear(); // Clear the local Channel Report
	RxPktStatLog log; //Temp
	u16_t PktCnt_temp; //Temp
	int Noise_before, Noise_after; //Temp
	/* Do work and keep track of Time Left */
	for (int ch = ParamLocal.StartCH; ch <= ParamLocal.StopCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		if (CHcnt == 0)
		{
			CHcnt++;
		}
		chrep_local.push_back({}); // Add Channel
		if (isMaster)
		{
			//Timer with Margin to let Slave Receive longer than sending
			k_timer_start(&uni_timer, K_MSEC((ST_TIME_PACKETS_MS / (paramChCNT))-2*ST_TIME_MARGIN_MS), K_MSEC(0));
			Noise_before = simple_nrf_radio.RSSI(8);
			k_sleep(K_MSEC(ST_TIME_MARGIN_MS)); //Let Slave meassure noise and prepare Reception
			while ((k_timer_remaining_get(&uni_timer) > 0) && (k_timer_remaining_get(&state_timer) > 0))
			{
				PktCnt_temp = simple_nrf_radio.BurstCntPkt(radio_pkt_Tx, K_MSEC(k_timer_remaining_get(&uni_timer)));				
			}
			k_sleep(K_MSEC(ST_TIME_MARGIN_MS)); //Let Slave meassure noise and End Reception
			Noise_after = simple_nrf_radio.RSSI(8);
		} else {
			k_timer_start(&uni_timer, K_MSEC((ST_TIME_PACKETS_MS / (paramChCNT))), K_MSEC(0));
			Noise_before = simple_nrf_radio.RSSI(8);
			while ((k_timer_remaining_get(&uni_timer) > 0) && (k_timer_remaining_get(&state_timer) > 0))
			{
				log = simple_nrf_radio.ReceivePktStatLog(K_MSEC(k_timer_remaining_get(&uni_timer)));	
			}
			Noise_after = simple_nrf_radio.RSSI(8);
		}
		// Slave and Master log all Data to keep timesync
		chrep_local.back().TxPkt_CNT = PktCnt_temp;
		chrep_local.back().CRCOK_CNT = log.CRCOKcnt;
		chrep_local.back().CRCERR_CNT = log.CRCErrcnt;
		chrep_local.back().Avg_SIG_RSSI = log.RSSI_Sum_Avg;
		chrep_local.back().Avg_NOISE_RSSI = (Noise_after+Noise_before)/2;
	}
	return;
}
		


/*=======================================================*/

void main(void)
{
	printk("P2P-Benchamrk Started\n");
	// Init Synctimer
	synctimer_init();
	synctimer_start();
	//config_debug_ppi_and_gpiote_radio_state(); // For Debug Timesync

	// Statemachine
	ST_Machine_tid = k_current_get();
	while (true)
	{
		switch (currentState)
		{
		case ST_DISCOVERY:
			ST_DISCOVERY_fn();
			k_sleep(K_MSEC(ST_TIME_DISCOVERY_MS + ST_TIME_MARGIN_MS)); // Keep waiting till next State. Not Synced Nodes do Power Save with 50% Duty Cycle.
			break;
		case ST_MOCKUP:
			ST_MOCKUP_fn();
			k_sleep(K_MSEC(ST_TIME_MOCKUP_MS + ST_TIME_MARGIN_MS + 10)); // Ready for the Next State. Extra Margin of 10ms to prevent reentry in case of delayed TimerInterrupt.
			break;
		case ST_PARAM:
			ST_PARAM_fn();
			k_sleep(K_MSEC(ST_TIME_PARAM_MS + ST_TIME_MARGIN_MS + 10)); // Ready for the Next State. Extra Margin of 10ms to prevent reentry in case of delayed TimerInterrupt.
			break;
		}
	}
}
