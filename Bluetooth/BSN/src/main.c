#include <zephyr.h>

#include "bm_blemesh.h"
#include "bm_timesync.h"
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

void main(void)
{
	bm_cli_log("Benchmark Started\n");
	bm_statemachine();
	//bm_blemesh_enable();
}
