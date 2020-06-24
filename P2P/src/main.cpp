#include <string.h>
#include <stdlib.h>
#include <vector>
#include <zephyr.h>
#include <shell/shell.h>
#include <hal/nrf_radio.h>
#include <drivers/clock_control.h>
#include <drivers/clock_control/nrf_clock_control.h>
#include "Timer_sync.h"
#include "Simple_nrf_radio.h"


#define isMaster 0								   //Defines if Node is the Master (1) or Slave (0)
#define DiscoveryMode NRF_RADIO_MODE_BLE_LR125KBIT // Defines the Discovery Mode used
#define DiscoveryStartCH 37						   // Defines the Start Channel of Discovery
#define DiscoveryEndCH 39						   // Defines the End Channel of Discovery

#define SW0_NODE	DT_ALIAS(led0)

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

//Radio radio;
Simple_nrf_radio simple_nrf_radio;
Stopwatch stopwatch;

// Definition of Sync Package
struct TIMESYNC_RADIO_PACKET
{
	u32_t SyncTimeOffset; // Timer Sync Offset
} __attribute__((packed)) SyncPkt;

// Definition of Radio Package
RADIO_PACKET radio_pkt = {};

int cmd_handler_send()
{
	simple_nrf_radio.Send(radio_pkt, 5000);
	SyncPkt.SyncTimeOffset = synctimer_getTxTimeStamp();
	printk("Ticks: %d\n\r",synctimer_getSyncTime());
	return 0;
}
SHELL_CMD_REGISTER(send, NULL, "Send Sync Packet", cmd_handler_send);
int cmd_handler_receive()
{
	simple_nrf_radio.Receive(&radio_pkt, 5000);
	SyncPkt = *(TIMESYNC_RADIO_PACKET*) radio_pkt.PDU;
	if (SyncPkt.SyncTimeOffset > 0){
		synctimer_setSync(SyncPkt.SyncTimeOffset);
		printk("Master: %d\n\r",Timestamp_Master);
		printk("Slave: %d\n\r",Timestamp_Slave);
		printk("Diff: %d\n\r",Timestamp_Diff);
	}
	printk("Ticks: %d\n\r",synctimer_getSyncTime());
	synctimer_TimeStampCapture_disable();
	return 0;
}
SHELL_CMD_REGISTER(receive, NULL, "Receive Sync Packet", cmd_handler_receive);

void main(void)
{
	// Init
	printk("Started \n");
	stopwatch.start_hp();
	int res = simple_nrf_radio.RSSI(8);
	stopwatch.stop_hp();
	printk("RSSI: %d\n", res);
	// Init Sync Package
	radio_pkt.length = sizeof(SyncPkt);
	radio_pkt.PDU = (u8_t *)&SyncPkt;
	// Start Sync if Master
	synctimer_init();
	synctimer_TimeStampCapture_enable();
	synctimer_start();
	printk("Synctimer Started");
}
