#include <string.h>
#include <stdlib.h>
#include <vector>
#include <zephyr.h>
#include <shell/shell.h>
#include <hal/nrf_radio.h>
#include <drivers/clock_control.h>
#include <drivers/clock_control/nrf_clock_control.h>
#include "simple_nrf_radio.h"

#define isMaster 0								   //Defines if Node is the Master (1) or Slave (0)
#define DiscoveryMode NRF_RADIO_MODE_BLE_LR125KBIT // Defines the Discovery Mode used
#define DiscoveryStartCH 37						   // Defines the Start Channel of Discovery
#define DiscoveryEndCH 39						   // Defines the End Channel of Discovery

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
u8_t MAC_Address[5] = NRF_FICR->DEVICEADDR[0]; // Random Device Address generation
Stopwatch stopwatch;

// Definition of Discovery Package
#define Broadcast_Address 0x8E89BED6



struct DISCOVERY_RADIO_PACKET
{
	u8_t opcode = 0;		  // opcode for Discovery Package
	u8_t time_till_mockup_ms; // Time left till Mockup Delay start in ms
	u32_t address = Device_Address; // Random Device Address generation
} __attribute__((packed));

int cmd_handler_send()
{
	char *Test_String = "C Programming is the shit";
	RADIO_PACKET s = {};
	s.PDU = (u8_t *)Test_String;
	s.length = strlen(Test_String) + 1; // Immer Länge + 1 bei String
	simple_nrf_radio.Send(s, 5000);
	return 0;
}
SHELL_CMD_REGISTER(send, NULL, "Send Payload", cmd_handler_send);
int cmd_handler_receive()
{
	RADIO_PACKET r;
	memset((u8_t *)&r, 0, sizeof(r));
	simple_nrf_radio.Receive((RADIO_PACKET *)&r, 10000);
	printk("Length %d\n", r.length);
	printk("%s\n", r.PDU);
	return 0;
}
SHELL_CMD_REGISTER(receive, NULL, "Receive Payload", cmd_handler_receive);

// Define a Timer used for Discovery Sync
K_TIMER_DEFINE(timer1, NULL, NULL);

// Define States for a simple Statemachine
#define ST_SLEEP 9
#define ST_DISCOVERY 10
u8_t currentState = ST_SLEEP;

void main(void)
{
	// Init
	printk("Started \n");
	stopwatch.start_hp();
	int res = simple_nrf_radio.RSSI(8);
	stopwatch.stop_hp();
	printk("RSSI: %d\n", res);

	// Init the Radio Packets
	DISCOVERY_RADIO_PACKET discPkt_Rx,discPkt_Tx = {};
	RADIO_PACKET radio_pkt_Rx,radio_pkt_Tx = {};
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
