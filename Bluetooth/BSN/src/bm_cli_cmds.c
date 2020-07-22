
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

#ifdef BENCHMARK_MASTER
#ifdef ZEPHYR_BLE_MESH
#include <shell/shell.h>
#include <stdlib.h>
#include <zephyr.h>

#elif defined NRF_SDK_Zigbee
// function is Define in header file
#include "nrf_cli.h"
#include "nrf_log.h"
#include "nrf_stack_guard.h"
#include "sdk_common.h"
#include <ctype.h>

#endif

bm_cli_cmd_getNodeReport_t bm_cli_cmd_getNodeReport;
bm_cli_cmd_setNodeSettings_t bm_cli_cmd_setNodeSettings;
bm_cli_cmd_startBM_t bm_cli_cmd_startBM;

#ifdef ZEPHYR_BLE_MESH

static int cmd_getNodeReport(const struct shell *shell, size_t argc, char **argv) { // Todo: Add error check for validating the Params are valid
  if (argc == 2) {
    bm_cli_cmd_getNodeReport.MAC = atoi(argv[1]); // MAC Adress in integer value format
    bm_cli_cmd_getNodeReport.req = true;
    shell_print(shell, "Report Request Sheduled for MAC: %x", bm_cli_cmd_getNodeReport.MAC);
  } else {
    shell_error(shell, "Number of Arguments incorrect! expected:\n getNodeReport <MAC in Integer format>\n");
  }
  return 0;
}
SHELL_CMD_REGISTER(getNodeReport, NULL, "Get the Node Report", cmd_getNodeReport);
static int cmd_setNodeSettings(const struct shell *shell, size_t argc, char **argv) { // Todo: Add error check for validating the Params are valid
  if (argc == 3) {
    bm_cli_cmd_setNodeSettings.MAC = atoi(argv[1]);          // MAC Adress in integer value format
    bm_cli_cmd_setNodeSettings.GroupAddress = atoi(argv[2]); // Group Address Number (0-25)
    bm_cli_cmd_setNodeSettings.req = true;
    shell_print(shell, "Request Sheduled for MAC: %x", bm_cli_cmd_setNodeSettings.MAC);
  } else {
    shell_error(shell, "Number of Arguments incorrect! expected:\n setNodeSettings <MAC in Integer format> <GroupNumber>\n");
  }
  return 0;
}
SHELL_CMD_REGISTER(setNodeSettings, NULL, "Set the Node Settings", cmd_setNodeSettings);
static int cmd_startBM(const struct shell *shell, size_t argc, char **argv) { // Todo: Add error check for validating the Params are valid
  if (argc == 3) {
    bm_cli_cmd_startBM.benchmark_time_s = atoi(argv[1]);
    bm_cli_cmd_startBM.benchmark_packet_cnt = atoi(argv[2]);
    bm_cli_cmd_startBM.req = true;
    shell_print(shell, "Benchmark Start scheduled");
  } else {
    shell_error(shell, "Number of Arguments incorrect! expected:\n startBM <BenchmarkTime (seconds)> <BenchmarkPacketsCount>\n");
  }
  return 0;
}
SHELL_CMD_REGISTER(startBM, NULL, "Start the Benchmark", cmd_startBM);

#elif defined NRF_SDK_Zigbee

static void cmd_getNodeReport(nrf_cli_t const *p_cli, size_t argc, char **argv) { // Todo: Add error check for validating the Params are valid
  if (argc == 2) {
    bm_cli_cmd_getNodeReport.MAC = atoi(argv[1]); // MAC Adress in integer value format
    bm_cli_cmd_getNodeReport.req = true;
    nrf_cli_print(p_cli, "Report Request Sheduled for MAC: %x", bm_cli_cmd_getNodeReport.MAC);
  } else {
    nrf_cli_error(p_cli, "Number of Arguments incorrect! expected:\n getNodeReport <MAC in Integer format>\n");
  }
}
NRF_CLI_CMD_REGISTER(getNodeReport, NULL, "Get the Node Report", cmd_getNodeReport);

static void cmd_setNodeSettings(nrf_cli_t const *p_cli, size_t argc, char **argv) { // Todo: Add error check for validating the Params are valid
  if (argc == 3) {
    bm_cli_cmd_setNodeSettings.MAC = atoi(argv[1]);          // MAC Adress in integer value format
    bm_cli_cmd_setNodeSettings.GroupAddress = atoi(argv[2]); // Group Address Number (0-25)
    bm_cli_cmd_setNodeSettings.req = true;
    nrf_cli_print(p_cli, "Request Sheduled for MAC: %x", bm_cli_cmd_getNodeReport.MAC);
  } else {
    nrf_cli_error(p_cli, "Number of Arguments incorrect! expected:\n setNodeSettings <MAC in Integer format> <GroupNumber>\n");
  }
}
NRF_CLI_CMD_REGISTER(setNodeSettings, NULL, "Set the Node Settings", cmd_setNodeSettings);

static void cmd_startBM(nrf_cli_t const *p_cli, size_t argc, char **argv) { // Todo: Add error check for validating the Params are valid
  if (argc == 3) {
    bm_cli_cmd_startBM.benchmark_time_s = atoi(argv[1]);
    bm_cli_cmd_startBM.benchmark_packet_cnt = atoi(argv[2]);
    bm_cli_cmd_startBM.req = true;
    nrf_cli_print(p_cli, "Benchmark Start scheduled");
  } else {
    nrf_cli_error(p_cli, "Number of Arguments incorrect! expected:\n startBM <BenchmarkTime (seconds)> <BenchmarkPacketsCount>\n");
  }
}
NRF_CLI_CMD_REGISTER(startBM, NULL, "Start the Benchmark", cmd_startBM);


#endif
#endif