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
#define isMaster 0									// Node is the Master (1) or Slave (0)
#define CommonMode NRF_RADIO_MODE_BLE_LR125KBIT		// Common Mode
#define CommonStartCH 37							// Common Start Channel
#define CommonEndCH 39								// Common End Channel
#define CommonCHCnt CommonEndCH - CommonStartCH + 1 // Common Channel Count
// Selection Between nrf52840 and nrf5340 of TxPower -> The highest available for Common Channel is recomended
#if defined(RADIO_TXPOWER_TXPOWER_Pos8dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_POS8DBM // Common Tx Power < 8 dBm
#elif defined(RADIO_TXPOWER_TXPOWER_0dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_0DBM // Common Tx Power < 0 dBm
#endif

#define MSB_MAC_Address 0xF4CE // MSB of MAC for Nordic Semi Vendor
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
/* The Timeout of the Report State is calculatet at Runtime = CommonCHCnt * NodesCnt * (TimoutRepReqCHPkt_ms + PramCHCnt * TimoutRepCHPkt_ms). 
A node's Report Timeout is calculatet at runtime = CH's *TimoutRepCHPkt_ms. */
#define TimoutRepReqCHPkt_ms 10
#define TimoutRepCHPkt_ms 5
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
K_TIMER_DEFINE(state_timer2, NULL, NULL);
// Reports finished all received Signal MAC_LSB_Address
#define ReportsDoneMACAddress 0xFFFFFFFF
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
	u8_t TxPower;
	u8_t NodesCnt;
} __attribute__((packed));

struct ReportsReqPkt
{
	u32_t LSB_MAC_Address;
} __attribute__((packed));

struct CHReportsPkt
{
	u8_t CH;
	u16_t CRCOK_CNT;
	u16_t CRCERR_CNT;
	u8_t Avg_SIG_RSSI;
	u8_t Avg_NOISE_RSSI;
} __attribute__((packed));

struct MasterReport
{
	u64_t TimeStamp;
	std::vector<NodeReport> nodrep;
};

struct NodeReport
{
	u32_t LSB_MAC_Address;
	std::vector<ChannelReport> chrep;
};

struct ChannelReport
{
	u8_t CH;
	u16_t TxPkt_CNT;
	u16_t CRCOK_CNT;
	u16_t CRCERR_CNT;
	u8_t Avg_SIG_RSSI;
	u8_t Avg_NOISE_RSSI;
};

 struct ChannelReport_array {
     struct ChannelReport Channels;
 };

struct Nodes_JSON_Report {
     //struct ChannelReport_array Channels[40];
	 struct ChannelReport_array; // <- Pointer from vector to array
     size_t ChannelReport_array_len;
 };

/*===========================================*/

/* ------------------- Variables -----------------*/
Simple_nrf_radio simple_nrf_radio;
u32_t LSB_MAC_Address = NRF_FICR->DEVICEADDR[0];
RADIO_PACKET radio_pkt_Rx, radio_pkt_Tx = {};
u8_t CHidx;

DiscoveryPkt Disc_pkt_RX, Disc_pkt_TX = {};
uint64_t MockupTS;
u32_t LatestTxMasterTimestamp;
bool Synced = false;

MockupPkt Mockup_pkt_RX, Mockup_pkt_TX = {};
bool GotReportReq = false;

ParamPkt Param_pkt_RX, Param_pkt_TX = {};
ParamPkt ParamLocal, ParamLocalBuf = {CommonStartCH, CommonEndCH, CommonMode, 20, 0, CommonTxPower, 0};
bool GotParam = false;

std::vector<ChannelReport> chrep_local;

ReportsReqPkt RepoReq_pkt_RX, RepoReq_pkt_TX = {};
uint64_t ST_REPORTS_TIMEOUT_MS;
uint64_t ST_NODE_CHANNEL_REPORT_TIMEOUT_MS;

CHReportsPkt Repo_pkt_RX, Repo_pkt_TX = {};
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
{ // Maybee at error check later for validating the Params are valid
	if (sizeof(argv) == sizeof(Param_pkt_TX) + 1)
	{
		ParamLocalBuf.StartCH = *argv[1];
		ParamLocalBuf.StopCH = *argv[2];
		ParamLocalBuf.Mode = *argv[3];
		ParamLocalBuf.Size = *argv[4];
		ParamLocalBuf.CCMA_CA = *argv[5];
		ParamLocalBuf.TxPower = *argv[6];
		shell_print(shell, "Done\n");
	}
	else
	{
		shell_error(shell, "Number of Arguments incorrect! expected:\n setParams <StartCH> <StopCH> <Mode> <Size> <CCMA_CA> <Tx_Power>\n");
	}
	return 0;
}
SHELL_CMD_REGISTER(setParams, NULL, "Set Parameters", cmd_setParams);

/*=================================================*/

/*-------------- Get a Nodereport Index by its MAC -----------*/
static u8_t GetNodeidx(u32_t MAC)
{
	// Range based for
	u8_t idx;
	for (const auto &value : masrep.nodrep)
	{
		if (value.LSB_MAC_Address == MAC)
		{
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
	simple_nrf_radio.setTxP(CommonTxPower);
	synctimer_TimeStampCapture_clear();
	synctimer_TimeStampCapture_enable();
	radio_pkt_Tx.length = sizeof(Disc_pkt_TX);
	radio_pkt_Tx.PDU = (u8_t *)&Disc_pkt_TX;
	CHidx = CommonCHCnt;
	/* Do work and keep track of Time Left */
	for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		while ((k_timer_remaining_get(&state_timer) / CHidx) > 0)
		{
			if (isMaster)
			{
				Disc_pkt_TX.MockupTimestamp = MockupTS;
				Disc_pkt_TX.LastTxTimestamp = synctimer_getTxTimeStamp();
				simple_nrf_radio.Send(radio_pkt_Tx, K_MSEC(k_timer_remaining_get(&state_timer) / CHidx));
			}
			else
			{
				s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&state_timer) / CHidx));
				if (ret > 0)
				{
					synctimer_TimeStampCapture_disable();
					// Keep on Receiving for the last Tx Timestamp
					s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&state_timer) / CHidx));
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
		CHidx--;
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
	simple_nrf_radio.setTxP(CommonTxPower);
	radio_pkt_Tx.length = sizeof(Mockup_pkt_TX);
	radio_pkt_Tx.PDU = (u8_t *)&Mockup_pkt_TX;
	Mockup_pkt_TX.LSB_MAC_Address = LSB_MAC_Address;
	CHidx = CommonCHCnt;
	/* Do work and keep track of Time Left */
	for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		while ((k_timer_remaining_get(&state_timer) / CHidx) > 0)
		{
			if (isMaster)
			{
				s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&state_timer) / CHidx));
				if (ret > 0)
				{
					if (GetNodeidx(((MockupPkt *)radio_pkt_Rx.PDU)->LSB_MAC_Address) == 0xFF)
					{
						masrep.nodrep.push_back({((MockupPkt *)radio_pkt_Rx.PDU)->LSB_MAC_Address});
					}
				}
			}
			else if (!GotReportReq)
			{
				k_sleep(K_MSEC(sys_rand32_get() % (k_timer_remaining_get(&state_timer) / CHidx - 5))); // Sleep Random Mockup Delay minus 5ms send Margin
				simple_nrf_radio.Send(radio_pkt_Tx, K_MSEC(k_timer_remaining_get(&state_timer) / CHidx));
				if (ch != CommonEndCH)
				{
					k_sleep(K_MSEC(k_timer_remaining_get(&state_timer) / CHidx)); // Sleep till next CH
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
		CHidx--;
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
	simple_nrf_radio.setTxP(CommonTxPower);
	radio_pkt_Tx.length = sizeof(Param_pkt_TX);
	radio_pkt_Tx.PDU = (u8_t *)&Param_pkt_TX;
	ParamLocalBuf.NodesCnt = masrep.nodrep.size(); // Fix the NodesCnt
	ParamLocal = ParamLocalBuf;					   // Copy the Params to be used
	Param_pkt_TX = ParamLocal;
	CHidx = CommonCHCnt;
	/* Do work and keep track of Time Left */
	for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		while ((k_timer_remaining_get(&state_timer) / CHidx) > 0)
		{
			if (isMaster)
			{
				simple_nrf_radio.Send(radio_pkt_Tx, K_MSEC(k_timer_remaining_get(&state_timer) / CHidx));
			}
			else
			{
				GotParam = false;
				s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&state_timer) / CHidx));
				if (ret > 0)
				{
					ParamLocal = *((ParamPkt *)radio_pkt_Rx.PDU);
					GotParam = true;
					return;
				}
			}
		}
		CHidx--;
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
	simple_nrf_radio.setTxP((nrf_radio_txpower_t)ParamLocal.TxPower);
	// Master and Slave have to do all init to keep Time Sync
	radio_pkt_Tx.length = ParamLocal.Size;
	sys_rand_get(radio_pkt_Tx.PDU, ParamLocal.Size); // Fill the Packet with Random Data
	CHidx = (ParamLocal.StopCH - ParamLocal.StartCH) + 1;
	if (CHidx == 0)
	{
		CHidx = 1;
	}
	chrep_local.clear(); // Clear the local Channel Report
	RxPktStatLog log;	 //Temp
	u16_t PktCnt_temp;	 //Temp
	u8_t Noise;			 //Temp
	/* Do work and keep track of Time Left */
	for (int ch = ParamLocal.StartCH; ch <= ParamLocal.StopCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		chrep_local.push_back({}); // Add Channel
		if (isMaster)
		{
			k_sleep(K_MSEC(ST_TIME_MARGIN_MS));																									   //Let Slave meassure noise and prepare Reception
			PktCnt_temp = simple_nrf_radio.BurstCntPkt(radio_pkt_Tx, K_MSEC((k_timer_remaining_get(&state_timer) / (CHidx)) - ST_TIME_MARGIN_MS)); //Margin to let Slave Receive longer than sending and log data
			k_sleep(K_MSEC(ST_TIME_MARGIN_MS));																									   //Let Slave End Reception
		}
		else
		{
			//======================= TODO ============================
			chrep_local.back().Avg_NOISE_RSSI = simple_nrf_radio.RSSI(8); //Find out how long it takes and rise itirations
			//======================= TODO ============================
			log = simple_nrf_radio.ReceivePktStatLog(K_MSEC((k_timer_remaining_get(&state_timer) / (CHidx))));
		}
		// Slave and Master log all Data to keep timesync
		chrep_local.back().TxPkt_CNT = PktCnt_temp;
		chrep_local.back().CRCOK_CNT = log.CRCOKcnt;
		chrep_local.back().CRCERR_CNT = log.CRCErrcnt;
		chrep_local.back().Avg_SIG_RSSI = log.RSSI_Sum_Avg;
		CHidx--;
	}
	return;
}

void ST_REPORT_fn(void)
{
	/* start one shot timer that expires after Timeslot ms */
	ST_NODE_CHANNEL_REPORT_TIMEOUT_MS = (ParamLocal.StopCH - ParamLocal.StartCH + 1) * TimoutRepCHPkt_ms + TimoutRepReqCHPkt_ms;
	ST_REPORTS_TIMEOUT_MS = CommonCHCnt * ParamLocal.NodesCnt * ST_NODE_CHANNEL_REPORT_TIMEOUT_MS;
	k_timer_start(&state_timer, K_MSEC(ST_REPORTS_TIMEOUT_MS), K_MSEC(0));
	synctimer_setSyncTimeCompareInt((synctimer_getSyncTime() + ST_REPORTS_TIMEOUT_MS * 1000 + ST_TIME_MARGIN_MS * 1000), ST_transition_cb);
	/* init */
	simple_nrf_radio.setMode(CommonMode);
	simple_nrf_radio.setAA(ReportsAddress);
	simple_nrf_radio.setTxP(CommonTxPower);
	bool CHsGot[40];	  // Received Channels from a Node
	u8_t Nodeidx = 0;		  // Node Index
	u8_t CHidx = 0;		  	  // Channel Index
	bool NodeDone = false;		  // Node Done Flag
	u8_t NodeHadCHcnt = 1;   // Counter to keep on with how many tries a node had on the Channels. Do Next If the Counter is equal to the number of Common Channels
	if (masrep.nodrep.size() != 0){
		RepoReq_pkt_TX.LSB_MAC_Address = masrep.nodrep[Nodeidx].LSB_MAC_Address; // Prepare first Node
	}
	GotReportReq = false; // Reset Got Request Flag
	if (isMaster)
	{
		radio_pkt_Tx.length = sizeof(RepoReq_pkt_TX);
		radio_pkt_Tx.PDU = (u8_t *)&RepoReq_pkt_TX;
		radio_pkt_Rx.length = sizeof(Repo_pkt_RX);
		radio_pkt_Rx.PDU = (u8_t *)&Repo_pkt_RX;		
	} else {
		radio_pkt_Rx.length = sizeof(RepoReq_pkt_RX);
		radio_pkt_Rx.PDU = (u8_t *)&RepoReq_pkt_RX;
		radio_pkt_Tx.length = sizeof(Repo_pkt_TX);
		radio_pkt_Tx.PDU = (u8_t *)&Repo_pkt_TX;
	}
	/* Do work and keep track of Time Left */
	while (k_timer_remaining_get(&state_timer) > 0){	
		// Do each Channel. Each Node Has at least three Chances to Transmit the Reports
		for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
		{
			simple_nrf_radio.setCH(ch);
			k_timer_start(&state_timer2, K_MSEC(ST_NODE_CHANNEL_REPORT_TIMEOUT_MS), K_MSEC(0));
			if (isMaster)
			{
				simple_nrf_radio.Send(radio_pkt_Tx,K_MSEC(k_timer_remaining_get(&state_timer2))); // Send Report Request
				while ((k_timer_remaining_get(&state_timer2) > 0))
				{
					if (RepoReq_pkt_TX.LSB_MAC_Address != ReportsDoneMACAddress){
						s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&state_timer2))); // Wait for Reports
						if (ret > 0)
						{	// Check that Channel wasnt allready received
							if (CHsGot[((CHReportsPkt *)radio_pkt_Rx.PDU)->CH] == false)
							{ 
								masrep.nodrep[Nodeidx].chrep.push_back({((CHReportsPkt *)radio_pkt_Rx.PDU)->CH,
																		chrep_local[((CHReportsPkt *)radio_pkt_Rx.PDU)->CH].TxPkt_CNT,
																		((CHReportsPkt *)radio_pkt_Rx.PDU)->CRCOK_CNT,
																		((CHReportsPkt *)radio_pkt_Rx.PDU)->CRCERR_CNT,
																		((CHReportsPkt *)radio_pkt_Rx.PDU)->Avg_SIG_RSSI,
																		((CHReportsPkt *)radio_pkt_Rx.PDU)->Avg_NOISE_RSSI});
								CHsGot[((CHReportsPkt *)radio_pkt_Rx.PDU)->CH] = true;
							}
							if (masrep.nodrep[Nodeidx].chrep.size() == chrep_local.size())
							{
								NodeDone = true;
								break;
							}
						}
					} else {
						simple_nrf_radio.Send(radio_pkt_Tx,K_MSEC(k_timer_remaining_get(&state_timer2))); // Send Report Done Bursts
					}				
				}
				if (NodeDone || NodeHadCHcnt == CommonCHCnt){
					NodeDone = false; //Clear Node Done Flag
					NodeHadCHcnt = 1; // Clear CHCnt
					std::fill_n(CHsGot, sizeof(CHsGot), false); // Clear Got Channels array
					if (RepoReq_pkt_TX.LSB_MAC_Address == ReportsDoneMACAddress){
						return; // When  transmittedNode done exit
					}
					if (Nodeidx == (masrep.nodrep.size() - 1 )){ 
						RepoReq_pkt_TX.LSB_MAC_Address = ReportsDoneMACAddress; // All Nodes Done :)
					} else {
						Nodeidx++; // Do Next Node ...
						RepoReq_pkt_TX.LSB_MAC_Address = masrep.nodrep[Nodeidx].LSB_MAC_Address; // Do next Node
					}					
				} else {
					NodeHadCHcnt++;
				}
				k_sleep(K_MSEC(k_timer_remaining_get(&state_timer2))); // Wait for the Next State too keep Timesync with Slaves		
			// Slave Code
			} else {
				s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&state_timer2))); //Wait for Report Request
				if (ret > 0)
				{
					if (((ReportsReqPkt *)radio_pkt_Rx.PDU)->LSB_MAC_Address == LSB_MAC_Address)
					{
						GotReportReq = true; // Set Flag that a Report Req for this Node was received
						CHidx = 0;
						// Burst out all available Channel Reports
						while(k_timer_remaining_get(&state_timer)>0){
							Repo_pkt_TX.CH = chrep_local[CHidx].CH;
							Repo_pkt_TX.Avg_NOISE_RSSI= chrep_local[CHidx].Avg_NOISE_RSSI;
							Repo_pkt_TX.Avg_SIG_RSSI= chrep_local[CHidx].Avg_SIG_RSSI;
							Repo_pkt_TX.CRCOK_CNT= chrep_local[CHidx].CRCOK_CNT;
							Repo_pkt_TX.CRCERR_CNT= chrep_local[CHidx].CRCERR_CNT;
							simple_nrf_radio.Send(radio_pkt_Tx,K_MSEC(k_timer_remaining_get(&state_timer2))); // Send Channel Report
							if (CHidx == (chrep_local.size() - 1 )){
								break;
							} else {
								CHidx++;
							}
						}
						k_sleep(K_MSEC(k_timer_remaining_get(&state_timer2))); // If there is time left sleep to keep Timesync				
					}
					else if (((ReportsReqPkt *)radio_pkt_Rx.PDU)->LSB_MAC_Address == ReportsDoneMACAddress)
					{
						return; // Reporting Done
					}
					else
					{
						k_sleep(K_MSEC(k_timer_remaining_get(&state_timer2))); // Wait for the Other Node to Finsih Reporting
					}
				}
			} // End Slave
		} // End Channel
	} // End Reporting
}

void ST_PUBLISH_fn(void)
{
	/* start one shot timer that expires after Timeslot ms */
	k_timer_start(&state_timer, K_MSEC(ST_TIME_PUBLISH_MS), K_MSEC(0));
	synctimer_setSyncTimeCompareInt((synctimer_getSyncTime() + ST_TIME_PUBLISH_MS * 1000 + ST_TIME_MARGIN_MS * 1000), ST_transition_cb);
	/* init */

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
		case ST_PACKETS:
			ST_PACKETS_fn();
			k_sleep(K_MSEC(ST_TIME_PACKETS_MS + ST_TIME_MARGIN_MS + 10)); // Ready for the Next State. Extra Margin of 10ms to prevent reentry in case of delayed TimerInterrupt.
			break;
		case ST_REPORTS:
			ST_REPORT_fn();
			// Ready for the Next State. Timesync is given up from now on. Do this in the Discovery State. -> This is done because of Performance Reasons
			break;
		case ST_PUBLISH:
			ST_REPORT_fn();
			// Ready for the Next State. Timesync is given up from now on. Do this in the Discovery State.
			break;
		}
	}
}
