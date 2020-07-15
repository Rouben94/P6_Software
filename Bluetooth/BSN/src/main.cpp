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
	//bm_statemachine();
	if (false){
		char *Test_String = "C Programming is the shit\0";
		RADIO_PACKET s = {};
		s.PDU = (u8_t *)Test_String;
		s.length = strlen(Test_String) + 1; // Immer Länge + 1 bei String
		while (true){
			simple_radio.Send(s);
		}
	} else {
		RADIO_PACKET r = {};
		simple_radio.Receive(&r,K_FOREVER);
		printk("%s",(char *)r.PDU);
		k_sleep(K_MSEC(10));
		k_sleep(K_FOREVER);		
	}
	//bm_blemesh_enable();
}

	
