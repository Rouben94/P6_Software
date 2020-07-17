#include <zephyr.h>

#include "bm_blemesh.h"
#include "bm_timesync.h"
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

uint32_t LSB_MAC_Address;


void main(void)
{
	// Init MAC Address
	LSB_MAC_Address = NRF_FICR->DEVICEADDR[1];
	bm_cli_log("Preprogrammed Randomly Static MAC-Address (LSB): %x\n", LSB_MAC_Address);

	// Start Benchmark
	bm_cli_log("Starting Benchmark...\n");
	bm_statemachine();


	//bm_blemesh_enable();
}
