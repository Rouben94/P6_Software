#include <zephyr.h>

//#include "bm_blemesh.h"
#include "bm_timesync.h"
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

#include "Simple_nrf_radio.hpp"

Simple_nrf_radio simple_radio;


void main(void)
{
	bm_cli_log("Benchmark Started\n");
	//bm_statemachine();
	if (isTimeMaster){
		char *Test_String = "C Programming is the shit";
		RADIO_PACKET s = {};
		s.PDU = (u8_t *)Test_String;
		s.length = strlen(Test_String) + 1; // Immer Länge + 1 bei String
		while (true){
			simple_radio.Send(s);
		}
	} else {
		RADIO_PACKET r = {};
		simple_radio.Receive(&r,K_FOREVER);
		printk("%s",r.PDU);
		k_sleep(K_FOREVER);		
	}
	//bm_blemesh_enable();
}
