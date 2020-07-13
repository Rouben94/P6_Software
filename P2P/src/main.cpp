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
#define isMaster 1									// Node is the Master (1) or Slave (0)
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
// Default CCA Threshold Value
#define CCA_TRESHOLD_DB 90													 // Specified as per 802.15.4 Spec should be 10dB Above Receiver Sensitivity (-100dBm)
#define ED_RSSISCALE 4														 // See Elctrical Specifications
#define ED_RSSIOFFS -92														 // See Elctrical Specifications (-92dBm)
#define CCA_TRESHOLD_ED_SCALE (CCA_TRESHOLD_DB - ED_RSSIOFFS) / ED_RSSISCALE // PRF[dBm] = ED_RSSIOFFS + ED_RSSISCALE x VALHARDWARE -----> VALHARDWARE = (PRF[dBm] - ED_RSSIOFFS) / ED_RSSISCALE

// Master: Find Nodes by Broadcasting ---- Slave: Listen for Broadcasts. If already Discovered Sleep
#define ST_DISCOVERY 10
// Master: Listen for Mockup Answers ---- Slave: Send a radnom delayed Mockup Answer. If already Discovered Sleep
#define ST_MOCKUP 20
// Master: Publish Settings and Timesync by Broadcasting ---- Slave: Listen for Settings and Timesync. If received go to sleep
#define ST_PARAM 30
// Master: Publish Packets by Broadcasting with defined Settings ---- Slave: Listen for Packets with defined Settings.
#define ST_PACKETS 40
// Master: Send Report Request ---- Slave: Listen for a Report Request
#define ST_REPORTS_REQ 50
// Master: Listen for Reports.  ---- Slave: Send Report
#define ST_REPORTS 60
// Master: Time for Master to Publish the Reports to CLI  ---- Slave: Sleep
#define ST_PUBLISH 70
// Timeslots for the Sates in ms. The Timesync has to be accurate enough. -> Optimized for 50 Nodes, 3 Channels and BLE LR125kBit
#define ST_TIME_DISCOVERY_MS 300
#define ST_TIME_MOCKUP_MS 1200
#define ST_TIME_PARAM_MS 60
#define ST_TIME_PACKETS_MS 5000 // Worst Case 40 CHs, Size 255 and BLE LR 125kBit -> ca. 2 Packets
#define ST_TIME_REPORT_REQ_MS 30
#define ST_TIME_REPORT_PKT_MS 7 /* A node's Report Timeout is calculatet at runtime = CHcnt *ST_TIME_REPORT_PKT_MS. */
#define ST_TIME_REPORT_MASTER_SAVE_MS 10
#define ST_TIME_PUBLISH_MS 500 // Should be allright with 40CHs.
// Margin for State Transition (Let the State Terminate)
#define ST_TIME_MARGIN_MS 5
// Addresses used by State. Random Generated for good diversity.
#define DiscoveryAddress 0x9CE74F9A
#define MockupAddress 0x57855CC7
#define ParamAddress 0x32C731E9
#define PacketsAddress 0xF492A652
#define ReportsReqAddress 0xC7B5A35D
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

struct ChannelReport
{
	bool valid;
	u16_t TxPkt_CNT;
	CHReportsPkt CHRepPkt;
};

struct NodeReport
{
	u32_t LSB_MAC_Address;
	u8_t RepCHcnt;
	std::vector<ChannelReport> chrep;
};

struct MasterReport
{
	u64_t TimeStamp;
	std::vector<NodeReport> nodrep;
};

/*===========================================*/

/* ------------------- Variables -----------------*/
Simple_nrf_radio simple_nrf_radio;
u32_t LSB_MAC_Address = NRF_FICR->DEVICEADDR[1];
RADIO_PACKET radio_pkt_Rx, radio_pkt_Tx = {};
u8_t CHidx;

DiscoveryPkt Disc_pkt_RX, Disc_pkt_TX = {};
uint64_t MockupTS;
u32_t LatestTxMasterTimestamp;
bool Synced = false;

MockupPkt Mockup_pkt_RX, Mockup_pkt_TX = {};
uint32_t rand_32 = sys_rand32_get(); //Takes Long Time so do once in init
bool GotReportReq = false;

ParamPkt Param_pkt_RX, Param_pkt_TX = {};
ParamPkt ParamLocal, ParamLocalBuf = {CommonStartCH, CommonEndCH, 20, CommonMode, false, CommonTxPower, 0};
//ParamPkt ParamLocal, ParamLocalBuf = {11, 26, 20, CommonMode, false, CommonTxPower, 0};
bool GotParam = false;

std::vector<ChannelReport> chrep_local;
u8_t RandData[250];

ReportsReqPkt RepoReq_pkt_RX, RepoReq_pkt_TX = {};
bool CHsGot[40];	// Received Channels from a Node
u8_t Nodeidx = 0;	// Node Index
u8_t NodeCHcnt = 0; // Node had Channel Count
bool ReportingDone = false;
u8_t ReportingDoneCnt = 0;

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
	printk("Radio State: %d\n", NRF_RADIO->STATE);
	return 0;
}
SHELL_CMD_REGISTER(test, NULL, "Testing Command", cmd_handler_test);

static int cmd_setParams(const struct shell *shell, size_t argc, char **argv)
{ // Todo: Add error check for validating the Params are valid
	if (argc == 7)
	{
		ParamLocalBuf.StartCH = atoi(argv[1]);
		ParamLocalBuf.StopCH = atoi(argv[2]);
		ParamLocalBuf.Mode = atoi(argv[3]);
		ParamLocalBuf.Size = atoi(argv[4]);
		ParamLocalBuf.CCMA_CA = atoi(argv[5]);
		ParamLocalBuf.TxPower = atoi(argv[6]);
		shell_print(shell, "Done setParams %d %d %d %d %d %d\n", ParamLocalBuf.StartCH, ParamLocalBuf.StopCH, ParamLocalBuf.Mode, ParamLocalBuf.Size, ParamLocalBuf.CCMA_CA, ParamLocalBuf.TxPower);
	}
	else
	{
		shell_error(shell, "Number of Arguments incorrect! expected:\n setParams <StartCH> <StopCH> <Mode> <Size> <CCMA_CA> <Tx_Power>\n");
	}
	return 0;
}

SHELL_CMD_REGISTER(setParams, NULL, "Set First Parameters", cmd_setParams);

/*=================================================*/

/*-------------- Convert RSSI to dBm -----------*/
static u8_t RSSI_to_dBm(u8_t RSSI)
{
	//PRF[dBm] = ED_RSSIOFFS + ED_RSSISCALE x VALHARDWARE
	return ED_RSSIOFFS + ED_RSSISCALE * RSSI;
}

/*-------------- Get a Nodereport Index by its MAC -----------*/
static u8_t GetNodeidx(u32_t MAC)
{
	// Range based for
	u8_t idx = 0;
	for (const auto &value : masrep.nodrep)
	{
		if (value.LSB_MAC_Address == MAC)
		{
			return idx;
		}
		idx++;
	}
	return 0xFF; //Node not found
}

class Stopwatch
{
public:
	u32_t start_time;
	u32_t stop_time;
	u32_t cycles_spent;
	u32_t nanoseconds_spent;

	s64_t time_stamp;
	s64_t milliseconds_spent;

	void start_hp()
	{
		/* capture initial time stamp */
		start_time = k_cycle_get_32();
	}
	void stop_hp()
	{
		/* capture final time stamp */
		stop_time = k_cycle_get_32();
		/* compute how long the work took (assumes no counter rollover) */
		cycles_spent = stop_time - start_time;
		nanoseconds_spent = SYS_CLOCK_HW_CYCLES_TO_NS(cycles_spent);
		printk("Time took %dns\n", nanoseconds_spent);
	}
	void start()
	{
		/* capture initial time stamp */
		time_stamp = k_uptime_get();
	}
	void stop()
	{
		/* compute how long the work took (also updates the time stamp) */
		milliseconds_spent = k_uptime_delta(&time_stamp);
		printk("Time took %lldms\n", milliseconds_spent);
	}
};

Stopwatch stpw;

/*--------------------- States -------------------*/

static void ST_transition_cb(void)
{
	printk("%u%u\n", (uint32_t)(synctimer_getSyncTime() >> 32), (uint32_t)synctimer_getSyncTime());
	// Not Synced Slave Nodes stay in Discovery State.
	if ((currentState == ST_DISCOVERY && (Synced && !isMaster)) || (currentState == ST_DISCOVERY && isMaster))
	{
		currentState = ST_MOCKUP;
	}
	// Only Continue when at Least one Node is Found. Slaves continue anyway
	else if (currentState == ST_MOCKUP)
	{
		if ((isMaster && masrep.nodrep.size() != 0) || (!isMaster))
			currentState = ST_PARAM;
		else
		{
			currentState = ST_DISCOVERY;
		}
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
		currentState = ST_REPORTS_REQ;
	}
	else if (currentState == ST_REPORTS_REQ)
	{
		currentState = ST_REPORTS;
	}
	// Master have to Report Reporting Done Flag for all CHannels
	else if (currentState == ST_REPORTS && isMaster)
	{
		if (ReportingDone && ReportingDoneCnt == CommonCHCnt)
		{
			currentState = ST_PUBLISH;
		}
		else
		{
			currentState = ST_REPORTS_REQ;
		}
	}
	// Slaves who are finished can go over to Publish state
	else if (currentState == ST_REPORTS && !isMaster)
	{
		if (ReportingDone)
		{
			currentState = ST_PUBLISH;
		}
		else
		{
			currentState = ST_REPORTS_REQ;
		}
	}
	else if (currentState == ST_PUBLISH)
	{
		currentState = ST_DISCOVERY;
	}
	printk("Current State: %d\n", currentState);
	printk("%s\n", k_thread_state_str(ST_Machine_tid));
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
				simple_nrf_radio.Send(radio_pkt_Tx);
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
		if (isMaster)
		{
			while ((k_timer_remaining_get(&state_timer) / CHidx) > 0)
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
		}
		else
		{
			if ((!GotReportReq) || (!GotParam))
			{
				k_sleep(K_MSEC(rand_32 % ((k_timer_remaining_get(&state_timer) / CHidx) - 10))); // Sleep Random Mockup Delay minus 10ms send Margin
				simple_nrf_radio.Send(radio_pkt_Tx);
			}
			else
			{
				// Node is known to Master. Can Return and Sleep..
				return;
			}
			k_sleep(K_MSEC(k_timer_remaining_get(&state_timer) / CHidx)); // Sleep till CH Ends
		}
		CHidx = CHidx - 1;
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
	Param_pkt_TX = ParamLocal;					   // ParamLocal are the Same on Master and Slave
	CHidx = CommonCHCnt;
	/* Do work and keep track of Time Left */
	for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
	{
		simple_nrf_radio.setCH(ch);
		while ((k_timer_remaining_get(&state_timer) / CHidx) > 0)
		{
			if (isMaster)
			{
				simple_nrf_radio.Send(radio_pkt_Tx);
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
	radio_pkt_Tx.PDU = RandData;
	CHidx = (ParamLocal.StopCH - ParamLocal.StartCH) + 1;
	if (CHidx == 0)
	{
		CHidx = 1;
	}
	chrep_local.clear();   // Clear the local Channel Report
	RxPktStatLog log;	   //Temp
	u16_t PktCnt_temp = 0; //Temp
	/* Do work and keep track of Time Left */
	for (int ch = ParamLocal.StartCH; ch <= ParamLocal.StopCH; ch++)
	{
		k_timer_start(&state_timer2, K_MSEC(k_timer_remaining_get(&state_timer) / (CHidx)), K_MSEC(0));
		simple_nrf_radio.setCH(ch);
		chrep_local.push_back({}); // Add Channel
		//printk("TIME NOW: %u\n",(uint32_t)synctimer_getSyncTime());
		if (isMaster)
		{
			//chrep_local.back().Avg_NOISE_RSSI = simple_nrf_radio.RSSI(40); //Find out how long it takes and rise itirations
			k_sleep(K_MSEC(5));																							 //Let Slave meassure noise and prepare Reception
			PktCnt_temp = simple_nrf_radio.BurstCntPkt(radio_pkt_Tx, K_MSEC(k_timer_remaining_get(&state_timer2) - 15)); //Margin to let Slave Receive longer than sending and log data
																														 //Let Slave End Reception
		}
		else
		{
			//======================= TODO ============================
			chrep_local.back().CHRepPkt.Avg_NOISE_RSSI = simple_nrf_radio.RSSI(32); //Find out how long it takes and rise itirations
			//======================= TODO ============================
			log = simple_nrf_radio.ReceivePktStatLog(K_MSEC(k_timer_remaining_get(&state_timer2) - 10));
		}
		// Slave and Master log all Data to keep timesync
		chrep_local.back().TxPkt_CNT = PktCnt_temp;
		chrep_local.back().CHRepPkt.CRCOK_CNT = log.CRCOKcnt;
		chrep_local.back().CHRepPkt.CRCERR_CNT = log.CRCErrcnt;
		chrep_local.back().CHRepPkt.Avg_SIG_RSSI = log.RSSI_Sum_Avg;
		chrep_local.back().CHRepPkt.CH = ch;
		k_sleep(K_MSEC(k_timer_remaining_get(&state_timer2)));
		CHidx--;
		//k_sleep(K_MSEC(k_timer_remaining_get(&state_timer) / (CHidx)));
	}
	return;
}

void ST_REPORT_REQ_fn(u8_t CH, u8_t NodeIdx)
{
	/* start one shot timer that expires after Timeslot ms */
	k_timer_start(&state_timer, K_MSEC(ST_TIME_REPORT_REQ_MS), K_MSEC(0));
	synctimer_setSyncTimeCompareInt((synctimer_getSyncTime() + ST_TIME_REPORT_REQ_MS * 1000 + ST_TIME_MARGIN_MS * 1000), ST_transition_cb);
	// Prepare Node Req
	simple_nrf_radio.setMode(CommonMode);
	simple_nrf_radio.setAA(ReportsReqAddress);
	simple_nrf_radio.setTxP(CommonTxPower);
	simple_nrf_radio.setCH(CH);
	radio_pkt_Tx.length = sizeof(RepoReq_pkt_TX);
	radio_pkt_Tx.PDU = (u8_t *)&RepoReq_pkt_TX;
	radio_pkt_Rx.length = sizeof(RepoReq_pkt_RX);
	radio_pkt_Rx.PDU = (u8_t *)&RepoReq_pkt_RX;
	// Set Request MAC Address
	if (NodeIdx < masrep.nodrep.size() && isMaster)
	{
		RepoReq_pkt_TX.LSB_MAC_Address = masrep.nodrep[NodeIdx].LSB_MAC_Address;
	}
	else if (ReportingDone && isMaster)
	{
		RepoReq_pkt_TX.LSB_MAC_Address = ReportsDoneMACAddress;
		ReportingDoneCnt++;
	}
	while (k_timer_remaining_get(&state_timer) > 0)
	{
		if (isMaster)
		{
			k_sleep(K_MSEC(5));
			simple_nrf_radio.Send(radio_pkt_Tx); // Burst Report Request
		}
		else
		{
			s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&state_timer))); //Wait for Report Request
			if (ret > 0)
			{
				if (((ReportsReqPkt *)radio_pkt_Rx.PDU)->LSB_MAC_Address == LSB_MAC_Address)
				{
					GotReportReq = true; // Set Flag that a Report Req for this Node was received
					break;
				}
				else if (((ReportsReqPkt *)radio_pkt_Rx.PDU)->LSB_MAC_Address == ReportsDoneMACAddress)
				{
					ReportingDone = true; // Set Flag that the Reporting is Done
					printk("used Normal exit\n");
					break;
				}
			}
		}
	}
	k_sleep(K_MSEC(k_timer_remaining_get(&state_timer))); // Wait for the Next State too keep Timesync
	return;
}

void ST_REPORT_fn(u8_t CH, u8_t NodeIdx)
{
	/* start one shot timer that expires after Timeslot ms */
	ST_REPORTS_TIMEOUT_MS = (ParamLocal.StopCH - ParamLocal.StartCH + 1) * ST_TIME_REPORT_PKT_MS;
	k_timer_start(&state_timer, K_MSEC(ST_REPORTS_TIMEOUT_MS), K_MSEC(0));
	synctimer_setSyncTimeCompareInt((synctimer_getSyncTime() + ST_REPORTS_TIMEOUT_MS * 1000 + ST_TIME_MARGIN_MS * 1000 + ST_TIME_REPORT_MASTER_SAVE_MS * 1000), ST_transition_cb);
	// Prepare Node Report
	simple_nrf_radio.setMode(CommonMode);
	simple_nrf_radio.setAA(ReportsAddress);
	simple_nrf_radio.setTxP(CommonTxPower);
	simple_nrf_radio.setCH(CH);
	radio_pkt_Tx.length = sizeof(Repo_pkt_TX);
	radio_pkt_Tx.PDU = (u8_t *)&Repo_pkt_TX;
	radio_pkt_Rx.length = sizeof(Repo_pkt_RX);
	radio_pkt_Rx.PDU = (u8_t *)&Repo_pkt_RX;
	// Check if Reporting Done
	if (Nodeidx == ParamLocal.NodesCnt || ReportingDone)
	{
		// Increase Channel IDX
		CHidx++;
		if (CHidx == CommonCHCnt)
		{
			CHidx = 0;
		}
		k_sleep(K_MSEC(k_timer_remaining_get(&state_timer))); // Wait for the Next State too keep Timesync
		return;
	}

	if (isMaster)
	{
		while (k_timer_remaining_get(&state_timer) > 0)
		{
			//printk("TIME in CH: %d : %u\n",CH,(u32_t)synctimer_getSyncTime());
			s32_t ret = simple_nrf_radio.Receive(&radio_pkt_Rx, K_MSEC(k_timer_remaining_get(&state_timer))); // Wait for Reports
			if (ret > 0)
			{
				chrep_local[((CHReportsPkt *)radio_pkt_Rx.PDU)->CH - ParamLocal.StartCH].CHRepPkt = *((CHReportsPkt *)radio_pkt_Rx.PDU);
				chrep_local[((CHReportsPkt *)radio_pkt_Rx.PDU)->CH - ParamLocal.StartCH].valid = true;
			}
		}
	}
	else
	{
		k_sleep(K_MSEC(1)); // Let Master Prepare Reception
		if (GotReportReq && k_timer_remaining_get(&state_timer) > 0)
		{
			for (u8_t i = 0; i < chrep_local.size(); i++)
			{
				Repo_pkt_TX = chrep_local[i].CHRepPkt;
				//printk("TIME in CH: %d : %u sent Pckt CH: %d\n",CH,(u32_t)synctimer_getSyncTime(),chrep_local[i].CHRepPkt.CH);
				simple_nrf_radio.Send(radio_pkt_Tx); // Burst Reports
			}
		}
	}
	//printk("TIME Done in CH: %d : %u\n", CH, (u32_t)synctimer_getSyncTime());
	k_sleep(K_MSEC(k_timer_remaining_get(&state_timer))); // Wait for the Next State too keep Timesync
	k_timer_start(&state_timer, K_MSEC(ST_TIME_REPORT_MASTER_SAVE_MS), K_MSEC(0));
	if (isMaster)
	{ // Check that all Channels are received.
		bool allValid = true;
		for (u8_t i = 0; i < chrep_local.size(); i++)
		{
			if (chrep_local[i].valid == false)
			{
				allValid = false;
				break;
			}
		}
		masrep.nodrep[Nodeidx].RepCHcnt++; // Increase Report Channel Counter
		if (masrep.nodrep[Nodeidx].RepCHcnt == CommonCHCnt || allValid)
		{
			// Fill up Masterreport
			masrep.nodrep[Nodeidx].chrep.clear();
			for (u8_t i = 0; i < chrep_local.size(); i++)
			{
				if (chrep_local[i].valid == true)
				{
					masrep.nodrep[Nodeidx].chrep.push_back({chrep_local[i]});
				}
			}
			// Check if any valid Data was Received from Node and Remove Node if not
			if (masrep.nodrep[Nodeidx].chrep.size() == 0)
			{
				// Didnt Received anything from Node... delete it from Nodelist
				masrep.nodrep.erase(masrep.nodrep.begin() + Nodeidx);
				ParamLocal.NodesCnt--;
				Nodeidx--; //Decrement Node index
			}
			// Do Next Node...
			Nodeidx++;
			if (Nodeidx < masrep.nodrep.size())
			{
				masrep.nodrep[Nodeidx].RepCHcnt = 0; // Init Node CHcnt
				for (u8_t i = 0; i < chrep_local.size(); i++)
				{
					chrep_local[i].valid = false; // Erase Valid Statement
				}
			}
			else
			{
				ReportingDone = true;
			}
		}
	}
	else
	{
		// Do a Savety Point for the Slave to Prevente get Stuck
		if (CHidx == CommonCHCnt - 1)
		{
			Nodeidx++;
		}
		if (Nodeidx == ParamLocal.NodesCnt)
		{
			ReportingDone = true;
			printk("used Safety exit\n");
		}
	}
	// Increase Channel IDX
	CHidx++;
	if (CHidx == CommonCHCnt)
	{
		CHidx = 0;
	}
	k_sleep(K_MSEC(k_timer_remaining_get(&state_timer))); // Wait for the Next State too keep Timesync
	return;
}

void ST_PUBLISH_fn(void)
{
	/* start one shot timer that expires after Timeslot ms */
	k_timer_start(&state_timer, K_MSEC(ST_TIME_PUBLISH_MS), K_MSEC(0));
	synctimer_setSyncTimeCompareInt((synctimer_getSyncTime() + ST_TIME_PUBLISH_MS * 1000 + ST_TIME_MARGIN_MS * 1000), ST_transition_cb);
	/* init */
	if (isMaster && !(masrep.nodrep.size() == 0))
	{
		// Do Reporting of Nodes
		for (const auto &node : masrep.nodrep)
		{
			for (const auto &ch : node.chrep)
			{
				printk("<NODE_REPORT_BEGIN>\r\n");
				// <MACAddress> <CH> <PktCnt> <CRCOKCnt> <CRCERRCnt> <AvgSigRSSI> <AvgNoiseRSSI>
				printk("%x %d %d %d %d %d %d\n", node.LSB_MAC_Address, ch.CHRepPkt.CH, ch.TxPkt_CNT, ch.CHRepPkt.CRCOK_CNT, ch.CHRepPkt.CRCERR_CNT, ch.CHRepPkt.Avg_SIG_RSSI, ch.CHRepPkt.Avg_NOISE_RSSI);
				printk("<NODE_REPORT_END>\r\n");
			}
		}
	}
	else
	{
		// For Debug
		for (const auto &ch : chrep_local)
		{
			printk("<NODE_REPORT_BEGIN>\r\n");
			// <MACAddress> <CH> <PktCnt> <CRCOKCnt> <CRCERRCnt> <AvgSigRSSI> <AvgNoiseRSSI>
			printk("%x %d %d %d %d %d %d\n", LSB_MAC_Address, ch.CHRepPkt.CH, ch.TxPkt_CNT, ch.CHRepPkt.CRCOK_CNT, ch.CHRepPkt.CRCERR_CNT, ch.CHRepPkt.Avg_SIG_RSSI, ch.CHRepPkt.Avg_NOISE_RSSI);
			printk("<NODE_REPORT_END>\r\n");
		}
		return; // Slave just sleeps in this state. Has nothing to do...
	}
}

/*=======================================================*/

void main(void)
{
	printk("P2P-Benchamrk Started\n");
	// Init Random Data
	sys_rand_get(RandData, 250);
	printk("Random Test Data Generated\n");
	// Init Synctimer
	synctimer_init();
	synctimer_start();
	printk("Synctimer ready and started\n");
	//config_debug_ppi_and_gpiote_radio_state(); // For Debug Timesync
	// Init MAC Address
	printk("Preprogrammed Randomly Static MAC-Address (LSB): %x\n", LSB_MAC_Address);

	// Statemachine
	ST_Machine_tid = k_current_get();
	while (true)
	{
		switch (currentState)
		{
		case ST_DISCOVERY:
			ST_DISCOVERY_fn();
			k_sleep(K_MSEC((ST_TIME_DISCOVERY_MS + ST_TIME_MARGIN_MS + 10))); // Keep waiting till next State. Not Synced Nodes do Power Save with 50% Duty Cycle.
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
			/* Init the Reporting */
			ReportingDone = false;
			GotReportReq = false;
			CHidx = 0;	 // Init CHannel Index
			Nodeidx = 0; // Init Node Index
			for (u8_t i = 0; i < chrep_local.size(); i++)
			{
				chrep_local[i].valid = false; // Erase Valid Statement
			}
			if (isMaster)
			{
				masrep.nodrep[Nodeidx].RepCHcnt = 0; // Init Node CHcnt
			}
			ReportingDoneCnt = 0;										  // init Reporting Done Counter
			k_sleep(K_MSEC(ST_TIME_PACKETS_MS + ST_TIME_MARGIN_MS + 10)); // Ready for the Next State. Extra Margin of 10ms to prevent reentry in case of delayed TimerInterrupt.
			break;
		case ST_REPORTS_REQ:
			ST_REPORT_REQ_fn(CHidx, Nodeidx);
			k_sleep(K_MSEC(ST_TIME_REPORT_REQ_MS + ST_TIME_MARGIN_MS + 10)); // Ready for the Next State. Extra Margin of 10ms to prevent reentry in case of delayed TimerInterrupt.
			break;
		case ST_REPORTS:
			ST_REPORT_fn(CHidx, Nodeidx);
			k_sleep(K_MSEC(ST_REPORTS_TIMEOUT_MS + ST_TIME_MARGIN_MS + 10)); // Ready for the Next State. Extra Margin of 10ms to prevent reentry in case of delayed TimerInterrupt.
			break;
		case ST_PUBLISH:
			ST_PUBLISH_fn();
			k_sleep(K_MSEC(ST_TIME_PUBLISH_MS + ST_TIME_MARGIN_MS + 10));
			// Ready for the Next State. Timesync is given up from now on. Do this in the Discovery State.
			break;
		}
	}
}
