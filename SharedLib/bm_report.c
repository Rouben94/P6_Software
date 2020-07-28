#include "bm_report.h"
#include "bm_cli.h"
#include "bm_config.h"
#include "bm_radio.h"
#include "bm_rand.h"
#include "bm_simple_buttons_and_leds.h"
#include "bm_timesync.h"

#include <hal/nrf_timer.h>
#include <nrfx_timer.h>

#include <stdio.h>
#include <string.h>

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_ZIGBEE
#endif

#define msg_time_ms 5 // Time needed for one message
#define msg_cnt 5     // Messages count used to transmit

#define ReportAddress 0xFD84AB05
#define CommonMode NRF_RADIO_MODE_BLE_1MBIT           // Common Mode
#define CommonStartCH 37                              // Common Start Channel
#define CommonEndCH 39                                // Common End Channel
#define CommonCHCnt (CommonEndCH - CommonStartCH + 1) // Common Channel Count
// Selection Between nrf52840 and nrf5340 of TxPower -> The highest available for Common Channel is recomended
#if defined(RADIO_TXPOWER_TXPOWER_Pos8dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_POS8DBM // Common Tx Power < 8 dBm
#elif defined(RADIO_TXPOWER_TXPOWER_0dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_0DBM // Common Tx Power < 0 dBm
#endif

typedef struct
{
  uint16_t ReportEntry; // numer of Report entry requested
} __attribute__((packed)) bm_report_req_msg_t;

static RADIO_PACKET Radio_Packet_TX, Radio_Packet_RX;

bm_report_req_msg_t bm_report_req_msg;

uint16_t rec_cnt = 0;
#ifdef BENCHMARK_MASTER
bool bm_report_msg_subscribe(bm_message_info *message_info) {
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(ReportAddress);
  bm_radio_setTxP(CommonTxPower);
  uint16_t bm_message_info_entry_ind = 0;
  uint16_t Errcnt = 0;
  bm_report_req_msg.ReportEntry = bm_message_info_entry_ind;
  Radio_Packet_TX.length = sizeof(bm_report_req_msg);
  Radio_Packet_TX.PDU = (uint8_t *)&bm_report_req_msg;
  for (int i = 0; i < (CommonCHCnt * NUMBER_OF_BENCHMARK_REPORT_MESSAGES); i++) // There is only one trie to transmitt the Report for a Channel
  {
    bm_radio_setCH(CommonStartCH + i % (CommonCHCnt)); // Set the Channel alternating from StartCH till StopCH
//    bm_cli_log("Sent report req %u\n", (uint32_t)synctimer_getSyncTime());
    bm_radio_send(Radio_Packet_TX);     // Send out Report Requests
//    bm_cli_log("Wait for report %u\n", (uint32_t)synctimer_getSyncTime());
    if (bm_radio_receive(&Radio_Packet_RX, 2 * msg_time_ms)) {
      // Save Report Entry
      message_info[bm_message_info_entry_ind] = *(bm_message_info *)Radio_Packet_RX.PDU; // Bring the sheep to a dry place
      if (message_info[bm_message_info_entry_ind].net_time == 0) {
        bm_cli_log("%u Reports received\n",bm_message_info_entry_ind);
        // Publish the reports
        for (int i = 0; i < bm_message_info_entry_ind; i++) {
            /*
                    bm_cli_log("<report> ");
                    bm_cli_log("%u ", message_info[i].message_id);
                    bm_cli_log("%u", (uint32_t)(message_info[i].net_time >> 32));
                    bm_cli_log("%u ", (uint32_t)message_info[i].net_time);
                    bm_cli_log("%u", (uint32_t)(message_info[i].ack_net_time >> 32));
                    bm_cli_log("%u ", (uint32_t)message_info[i].ack_net_time);
                    bm_cli_log("%u ", message_info[i].number_of_hops);
                    bm_cli_log("%d ", message_info[i].rssi);
                    bm_cli_log("%x ", message_info[i].src_addr);
                    bm_cli_log("%x ", message_info[i].dst_addr);
                    bm_cli_log("%x ", message_info[i].group_addr);
                    bm_cli_log("\r\n");
          */
          char buf[11][50] = {"\0"};
          char str_output[150] = {"\0"};

          strcpy(str_output, "<report> ");
          sprintf(buf[0], "%u ", message_info[i].message_id);
          sprintf(buf[1], "%u", (uint32_t)(message_info[i].net_time >> 32));
          sprintf(buf[2], "%u ", (uint32_t)message_info[i].net_time);
          sprintf(buf[3], "%u", (uint32_t)(message_info[i].ack_net_time >> 32));
          sprintf(buf[4], "%u ", (uint32_t)message_info[i].ack_net_time);
          sprintf(buf[5], "%u ", message_info[i].number_of_hops);
          sprintf(buf[6], "%d ", (int8_t)message_info[i].rssi);
          sprintf(buf[7], "%x ", message_info[i].src_addr);
          sprintf(buf[8], "%x ", message_info[i].dst_addr);
          sprintf(buf[9], "%x ", message_info[i].group_addr);
          sprintf(buf[10], "%u \r\n", message_info[i].data_size);
          for (int a = 0; a < 11; a++) {
            strcat(str_output, buf[a]);
          }
          bm_cli_log("%s", str_output);
#ifdef NRF_SDK_ZIGBEE
          NRF_LOG_FLUSH();
//          NRF_LOG_PROCESS();
          bm_cli_process();
#endif
        }
        return true; // Finish here
      }
      // Publish the report
      // Next Report Entry
      bm_message_info_entry_ind++;
      bm_report_req_msg.ReportEntry = bm_message_info_entry_ind;
      // Decrease channel index to keep chanel the same
      i--;
    } else {
      Errcnt++;      
    }
    if (Errcnt > 450) { // Should be a timeout of 5 seconds
      bm_cli_log("Timeout Receiving Reports... Is Master close enough?\n");
      return false;
    }
  }
  bm_cli_log("No reports received\n");
  return false;
}
#endif

bool bm_report_msg_publish(bm_message_info *message_info) {
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(ReportAddress);
  bm_radio_setTxP(CommonTxPower);
  uint16_t bm_message_info_entry_ind = 0;
  uint16_t Errcnt = 0;
  Radio_Packet_TX.length = sizeof(bm_message_info);
  for (int i = 0; i < (CommonCHCnt * NUMBER_OF_BENCHMARK_REPORT_MESSAGES); i++) // There is only one trie to transmitt the Report for a Channel
  {
    bm_radio_setCH(CommonStartCH + i % (CommonCHCnt));                 // Set the Channel alternating from StartCH till StopCH
                                                                           //    bm_cli_log("Wait for report Req %u\n", (uint32_t)synctimer_getSyncTime());
    if (bm_radio_receive(&Radio_Packet_RX, (msg_time_ms * 3) * CommonCHCnt)) // Wait for the master to finish a request on all Channels
    {
      // Save Report Entry
      bm_message_info_entry_ind = (*(bm_report_req_msg_t *)Radio_Packet_RX.PDU).ReportEntry; // Bring the sheep to a dry place
      Radio_Packet_TX.PDU = (uint8_t *)&(message_info[bm_message_info_entry_ind]);           // Prepare Sending
                                                                                             //      bm_cli_log("Message Index received: %u\n", bm_message_info_entry_ind);
                                                                                             //      bm_cli_log("Time before sending report %u\n", (uint32_t)synctimer_getSyncTime());
      bm_radio_send(Radio_Packet_TX);                   // Send out Report
      // Decrease channel index to keep channel the same
      i--;
      // Check if it was the last report
      if (message_info[bm_message_info_entry_ind].net_time == 0) {
        bm_cli_log("%u Reports send\n",bm_message_info_entry_ind);
        return true;
      }
    } else {
      Errcnt++;      
    }
    if (Errcnt > 170) { // Should be a timeout of 5 seconds
      bm_cli_log("Timeout Sending Reports... Is Master close enough?\n");
      return false;
    }
  }
  return false;
}