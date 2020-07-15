#include <zephyr.h>

/*
//#include "bm_blemesh.h"
#include "bm_timesync.h"
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"
*/
#include "Simple_nrf_radio.hpp"

Simple_nrf_radio simple_radio;

void main(void)
{
	printk("Benchmark Started\n");
	k_sleep(K_MSEC(1000));
	NRF_CLOCK->TASKS_HFCLKSTART = 1;    //Start high frequency clock
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0; //Clear event
	//bm_statemachine();
	if (true)
	{
		char *Test_String = "C Programming is the shit\0";
		RADIO_PACKET s = {};
		s.PDU = (u8_t *)Test_String;
		s.length = strlen(Test_String) + 1; // Immer Länge + 1 bei String
		nrf_radio_power_set(NRF_RADIO,true);
		simple_radio.setMode(NRF_RADIO_MODE_BLE_1MBIT);
		simple_radio.setCH(37);
		simple_radio.setAA(0x34125678);
		while (true)
		{
			//nrf_radio_mode_set(NRF_RADIO,NRF_RADIO_MODE_BLE_1MBIT);
			//nrf_radio_frequency_set(NRF_RADIO,2407);
			printk("Channel %d\n", nrf_radio_frequency_get(NRF_RADIO));
			printk("Radio State %d\n", nrf_radio_state_get(NRF_RADIO));
			simple_radio.Send(s);
			//printk("Sent Packages: %d\n", simple_radio.BurstCntPkt(s,0,K_MSEC(1000)));
			printk("Radio State %d\n", nrf_radio_state_get(NRF_RADIO));
			printk("Pkt Len %d\n", ((u8_t *)NRF_RADIO->PACKETPTR)[0]);
			printk("Radio Mode %d\n", nrf_radio_mode_get(NRF_RADIO));
			printk("Channel %d\n", nrf_radio_frequency_get(NRF_RADIO));			
		}
	}
	else
	{
		RADIO_PACKET r = {};
		nrf_radio_power_set(NRF_RADIO,true);
		simple_radio.setMode(NRF_RADIO_MODE_BLE_1MBIT);
		simple_radio.setCH(37);
		simple_radio.setAA(0x34125678);
		while (true)
		{
			printk("Channel %d\n", nrf_radio_frequency_get(NRF_RADIO));
			printk("Radio State %d\n", nrf_radio_state_get(NRF_RADIO));
			simple_radio.Receive(&r, K_MSEC(100));
			printk("Radio State %d\n", nrf_radio_state_get(NRF_RADIO));
			printk("Pkt Len %d\n", ((u8_t *)NRF_RADIO->PACKETPTR)[0]);
			printk("Radio Mode %d\n", nrf_radio_mode_get(NRF_RADIO));
			printk("Channel %d\n", nrf_radio_frequency_get(NRF_RADIO));
		}
	}
	//bm_blemesh_enable();
}
