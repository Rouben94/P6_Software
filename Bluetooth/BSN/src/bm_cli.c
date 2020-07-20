#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

/* Init the Parameters */
bm_params_t bm_params = {BENCHMARK_DEFAULT_TIME_S,BENCHMARK_DEFAULT_PACKETS_CNT};
bm_params_t bm_params_buf = {BENCHMARK_DEFAULT_TIME_S,BENCHMARK_DEFAULT_PACKETS_CNT};

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#include <shell/shell.h>
#include <stdlib.h>
void bm_cli_log(const char *fmt, ...) {
  // Zephyr way to Log info
  printk(fmt);
}
#ifdef BENCHMARK_MASTER
bm_cli_cmd_getNodeReport_t bm_cli_cmd_getNodeReport;
bm_cli_cmd_setNodeSettings_t bm_cli_cmd_setNodeSettings;
bm_cli_cmd_startBM_t bm_cli_cmd_startBM;
static int cmd_getNodeReport(const struct shell *shell, size_t argc, char **argv)
{ // Todo: Add error check for validating the Params are valid
	if (argc == 2)
	{
		bm_cli_cmd_getNodeReport.MAC = atoi(argv[1]); // MAC Adress in integer value format
    bm_cli_cmd_getNodeReport.req = true;
		shell_print(shell, "Report Request Sheduled for MAC: %x",bm_cli_cmd_getNodeReport.MAC);
	}
	else
	{
		shell_error(shell, "Number of Arguments incorrect! expected:\n getNodeReport <MAC in Integer format>\n");
	}
	return 0;
}
SHELL_CMD_REGISTER(getNodeReport, NULL, "Get the Node Report", cmd_getNodeReport);
static int cmd_setNodeSettings(const struct shell *shell, size_t argc, char **argv)
{ // Todo: Add error check for validating the Params are valid
	if (argc == 3)
	{
		bm_cli_cmd_setNodeSettings.MAC = atoi(argv[1]); // MAC Adress in integer value format
    bm_cli_cmd_setNodeSettings.GroupAddress = atoi(argv[2]); // Group Address Number (0-25)
    bm_cli_cmd_setNodeSettings.req = true;
		shell_print(shell, "Request Sheduled for MAC: %x",bm_cli_cmd_getNodeReport.MAC);
	}
	else
	{
		shell_error(shell, "Number of Arguments incorrect! expected:\n setNodeSettings <MAC in Integer format> <GroupNumber>\n");
	}
	return 0;
}
SHELL_CMD_REGISTER(setNodeSettings, NULL, "Set the Node Settings", cmd_setNodeSettings);
static int cmd_startBM(const struct shell *shell, size_t argc, char **argv)
{ // Todo: Add error check for validating the Params are valid
	if (argc == 3)
	{
		bm_cli_cmd_startBM.benchmark_time_s = atoi(argv[1]); 
    bm_cli_cmd_startBM.benchmark_packet_cnt = atoi(argv[2]); 
    bm_cli_cmd_startBM.req = true;
		shell_print(shell, "Benchmark Start scheduled");
	}
	else
	{
		shell_error(shell, "Number of Arguments incorrect! expected:\n startBM <BenchmarkTime (seconds)> <BenchmarkPacketsCount>\n");
	}
	return 0;
}
SHELL_CMD_REGISTER(startBM, NULL, "Start the Benchmark", cmd_startBM);
#endif
#elif defined NRF_SDK_Zigbee
// function is Define in header file
#endif