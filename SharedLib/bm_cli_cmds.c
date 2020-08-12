/*
This file is part of Benchmark-Shared-Library.

Benchmark-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchmark-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchmark-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR Part ZEPHYR_BLE_MESH 	   :   Raffael Anklin        */
/* AUTHOR Part NRF_SDK_ZIGBEE 	 :     Cyrill Horath      */


#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

#ifdef BENCHMARK_MASTER
#ifdef ZEPHYR_BLE_MESH
#include <shell/shell.h>
#include <stdlib.h>
#include <zephyr.h>

#elif defined NRF_SDK_ZIGBEE || defined NRF_SDK_THREAD
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
  if (argc == 10) {
    bm_cli_cmd_setNodeSettings.MAC = atoi(argv[1]);          // MAC Adress in integer value format
    bm_cli_cmd_setNodeSettings.GroupAddress = atoi(argv[2]); // Group Address Number (0-25)
    bm_cli_cmd_setNodeSettings.NodeId = atoi(argv[3]); // NodeId Number (0-50)
    bm_cli_cmd_setNodeSettings.Ack = atoi(argv[4]); // Enable Acknowledge
    bm_cli_cmd_setNodeSettings.AdditionalPayloadSize = atoi(argv[5]); // Additional Paylouad Size
    bm_cli_cmd_setNodeSettings.benchmark_Traffic_Generation_Mode = atoi(argv[6]); // Payload Generation Mode
    bm_cli_cmd_setNodeSettings.DestMAC_1 = atoi(argv[7]); // Zigbee Directed Destination 1
    bm_cli_cmd_setNodeSettings.DestMAC_2 = atoi(argv[8]); // Zigbee Directed Destination 2
    bm_cli_cmd_setNodeSettings.DestMAC_3 = atoi(argv[9]); // Zigbee Directed Destination 3
    bm_cli_cmd_setNodeSettings.req = true;
    shell_print(shell, "Request Sheduled for MAC: %x", bm_cli_cmd_setNodeSettings.MAC);
  } else {
    shell_error(shell, "Number of Arguments incorrect! expected:\n setNodeSettings <MAC in Integer format> <GroupNumber> <Node Id> <Ack> <AdditionalPayloadSize> <BenchmarkTrafficGenMode> <DST_MAC_1> <DST_MAC_2> <DST_MAC_3>\n");
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

#elif defined NRF_SDK_ZIGBEE || defined NRF_SDK_THREAD

static void cmd_getNodeReport(nrf_cli_t const *p_cli, size_t argc, char **argv) { // Todo: Add error check for validating the Params are valid
  if (argc == 2) {
    bm_cli_cmd_getNodeReport.MAC = strtoul(argv[1], NULL, NULL); // MAC Adress in integer value format
    bm_cli_cmd_getNodeReport.req = true;
    nrf_cli_print(p_cli, "Report request scheduled for MAC: 0x%x, %s", bm_cli_cmd_getNodeReport.MAC, argv[1]);
  } else {
    nrf_cli_error(p_cli, "Number of Arguments incorrect! expected:\n getNodeReport <MAC in Integer format>\n");
  }
}
NRF_CLI_CMD_REGISTER(getNodeReport, NULL, "Get the Node Report", cmd_getNodeReport);

static void cmd_setNodeSettings(nrf_cli_t const *p_cli, size_t argc, char **argv) { // Todo: Add error check for validating the Params are valid
  if (argc == 10) {
    bm_cli_cmd_setNodeSettings.MAC = strtoul(argv[1], NULL, NULL); // MAC Adress in integer value format
    bm_cli_cmd_setNodeSettings.GroupAddress = atoi(argv[2]);       // Group Address Number (0-25)
    bm_cli_cmd_setNodeSettings.NodeId = atoi(argv[3]); // NodeId Number (0-50)
    bm_cli_cmd_setNodeSettings.Ack = atoi(argv[4]); // Enable Acknowledge
    bm_cli_cmd_setNodeSettings.AdditionalPayloadSize = atoi(argv[5]); // Additional Payload Size
    bm_cli_cmd_setNodeSettings.benchmark_Traffic_Generation_Mode = atoi(argv[6]); // Payload Generation Mode
    bm_cli_cmd_setNodeSettings.DestMAC_1 = strtoul(argv[7], NULL, NULL); // Zigbee Directed Destination 1
    bm_cli_cmd_setNodeSettings.DestMAC_2 = strtoul(argv[8], NULL, NULL); // Zigbee Directed Destination 2
    bm_cli_cmd_setNodeSettings.DestMAC_3 = strtoul(argv[9], NULL, NULL); // Zigbee Directed Destination 3
    bm_cli_cmd_setNodeSettings.req = true;
    nrf_cli_print(p_cli, "Set node settings request scheduled for MAC: 0x%x, %s", bm_cli_cmd_setNodeSettings.MAC, argv[1]);
  } else {
    nrf_cli_error(p_cli, "Number of Arguments incorrect! expected:\n setNodeSettings <MAC in Integer format> <GroupNumber> <Node Id> <Ack> <AdditionalPayloadSize> <BenchmarkTrafficGenMode> <DST_MAC_1> <DST_MAC_2> <DST_MAC_3>\n");
  }
}
NRF_CLI_CMD_REGISTER(setNodeSettings, NULL, "Set the Node Settings", cmd_setNodeSettings);

static void cmd_startBM(nrf_cli_t const *p_cli, size_t argc, char **argv) { // Todo: Add error check for validating the Params are valid
  if (argc == 3) {
    bm_cli_cmd_startBM.benchmark_time_s = atoi(argv[1]);
    bm_cli_cmd_startBM.benchmark_packet_cnt = atoi(argv[2]);
    bm_cli_cmd_startBM.req = true;
    nrf_cli_print(p_cli, "Benchmark start scheduled");
  } else {
    nrf_cli_error(p_cli, "Number of Arguments incorrect! expected:\n startBM <BenchmarkTime (seconds)> <BenchmarkPacketsCount>\n");
  }
}
NRF_CLI_CMD_REGISTER(startBM, NULL, "Start the Benchmark", cmd_startBM);

#endif
#endif