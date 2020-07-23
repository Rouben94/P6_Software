#include "bm_cli.h"
#include "bm_config.h"
#include "bm_control.h"
#include "bm_log.h"
#include "bm_radio.h"
#include "bm_rand.h"
#include "bm_report.h"
#include "bm_timesync.h"

#include <stdlib.h>
#include <string.h>

#ifdef ZEPHYR_BLE_MESH
#include "bm_blemesh.h"
#include "bm_blemesh_model_handler.h"
#include "bm_simple_buttons_and_leds.h"
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#include "bm_zigbee.h"
#include "bm_simple_buttons_and_leds.h"
#endif

#include "bm_statemachine.h"

// Initilize all the Devices and Functions
#define ST_INIT 10
// Master: Wait for cli commands and send out corresponding control packets to Slaves. Slaves: Listen for control packets and relay them. (One Way Communication Master -> Slaves)
#define ST_CONTROL 20
// Slave send Reports back to Master (no relaying)
#define ST_REPORT 100
// Master: Broadcast Timesync Packets with Parameters for the Benchmark ---- Slave: Listen for Timesync Broadcast. Relay with random Backof Delay for other Slaves
#define ST_TIMESYNC 50
// Init the Mesh Stack till all Nodes are "comissioned" / "provisioned" or whatever it is called in the current stack... -> Stack should be ready for Benchmark
#define ST_INIT_BENCHMARK 60
/* Benchmark the Mesh Stack: Since the Zigbee stack owns the CPU all the Time, do all the work in the Timer callback.
The Benchmark does simulate a button press, which triggers the sending of a benchmark message. This is done over a Timer interrupt callback (State Transition).
The callback structure should be the same over all Mesh Stacks. The Procedure is:
I.   call the stack specific button pressed callback (bm_simulate_button_press_cb) -> this should send the benchmark message (or schedule in Zigbee)
II.  check if end of benchmark is reached (increment counter)
III.   if no -> schedule next Timer interrupt callback (aka "button press")
IV.    if yess -> change to next state*/
#define ST_BENCHMARK 70
// Save the Logged Data to Flash and Reset Device to shut down the Mesh Stack
#define ST_SAVE_FLASH 80
// Timeslots for the Sates in ms. The Timesync has to be accurate enough.
#define ST_TIMESYNC_TIME_MS 5000        // -> Optimized for 50 Nodes, 3 Channels and BLE LR125kBit
#define ST_INIT_BENCHMARK_TIME_MS 10000 // Time required to init the Mesh Stack
// The Benchmark time is obtained by the arameters from Timesync
#define ST_BENCHMARK_MIN_GAP_TIME_US 1000 // Minimal Gap Time to not exit the interrupt context while waiting for another package.
#define ST_SAVE_FLASH_TIME_MS 1000        // Time required to Save Log to Flash

#define ST_MARGIN_TIME_MS 5            // Margin for State Transition (Let the State Terminate)
#define ST_TIMESYNC_BACKOFF_TIME_MS 30 // Backoff time maximal for retransmitt the Timesync Packet -> Should be in good relation to Timesync Timeslot

uint32_t LSB_MAC_Address;            // Preprogrammed Randomly Static MAC-Address (LSB)
uint8_t currentState = ST_INIT;      // Init the Statemachine in the Timesync State
uint64_t start_time_ts_us;           // Start Timestamp of a State
uint64_t next_state_ts_us;           // Next Timestamp for sheduled transition
bool wait_for_transition = false;    // Signal that a Waiting for a Transition is active
bool transition = false;             // Signal that a Transition has occured
uint16_t bm_rand_msg_ts_ind;         // Timewindow for a Benchmarkmessage = BenchmarkTime / BenchmarkPcktCnt
bm_control_msg_t bm_control_msg;     // Control Message Buffer
bool transition_to_timesync = false; // Switch Flag for a Transition Request to Timesync
bool transition_to_report = false;   // Switch Flag for a Transition Request to Report
bool benchmark_messageing_done = false;   // Switch Flag for signaling that a Benchmark Client has Done all its Messaging

#ifdef BENCHMARK_CLIENT
static void ST_BENCHMARK_msg_cb(void);
#endif

static void ST_transition_cb(void) {
  if (currentState == ST_INIT) {
    currentState = ST_CONTROL;
  } else if (currentState == ST_CONTROL && transition_to_timesync) {
    transition_to_timesync = false;
    currentState = ST_TIMESYNC;
  } else if (currentState == ST_CONTROL && transition_to_report) {
    transition_to_report = false;
    currentState = ST_REPORT;
  } else if (currentState == ST_REPORT) {
    currentState = ST_CONTROL;
  } else if (currentState == ST_TIMESYNC) // Not Synced Slave Nodes stay in Timesync State.
  {
    #ifdef BENCHMARK_MASTER
    currentState = ST_INIT_BENCHMARK;
    #else
    if (bm_state_synced){
      currentState = ST_INIT_BENCHMARK;
    } else {
      bm_led1_set(true); // Switch ON the RED LED
      currentState = ST_CONTROL;
    }
    #endif
  } else if (currentState == ST_INIT_BENCHMARK) {
    currentState = ST_BENCHMARK;
  } else if (currentState == ST_BENCHMARK) {
    #ifdef BENCHMARK_CLIENT
    if(!benchmark_messageing_done){
      /* Call the Benchmark send_message callback */
      ST_BENCHMARK_msg_cb();
      return; 
    }
    #endif
    currentState = ST_SAVE_FLASH;
  }
  if (!wait_for_transition) {
    bm_cli_log("ERROR: Statemachine out of Sync !!!\n");
    bm_cli_log("Please reset Device\n");
  }
  transition = true;
  bm_cli_log("%u%u\n", (uint32_t)(synctimer_getSyncTime() >> 32), (uint32_t)synctimer_getSyncTime()); // For Debug
  bm_cli_log("Current State: %d\n", currentState);
}

void ST_INIT_fn(void) {

    // Init MAC Address
  LSB_MAC_Address = NRF_FICR->DEVICEADDR[0];
  bm_cli_log("Preprogrammed Randomly Static MAC-Address (LSB): %x\n", LSB_MAC_Address);
 

  bm_init_leds();
  synctimer_init();
  synctimer_start();
  bm_rand_init();
  bm_radio_init();
  bm_log_init();
#ifdef NRF_SDK_Zigbee
  bm_cli_init();
#endif

#ifdef BENCHMARK_MASTER
bm_cli_log("Master Started\n");
#elif defined BENCHMARK_CLIENT
bm_cli_log("Client Started\n");
#elif defined BENCHMARK_SERVER
bm_cli_log("Server Started\n");
#else
bm_cli_log("Error no Role defined. Please define a Role in the bm_config.h (BENCHMARK_MASTER - CLIENT - SERVER)\n");
#endif

  /* Test read FLASH Data */
  uint32_t restored_cnt = bm_log_load_from_flash(); // Restor Log Data from FLASH
  bm_cli_log("Restored %u entries from Flash\n", restored_cnt);
  bm_cli_log("First Log Entry: %u %u ...\n", message_info[0].message_id, (uint32_t)message_info[0].net_time);
  bm_cli_log("<report> %u %u%u %u%u %u %d %x %x %x\r\n",message_info[0].message_id,(uint32_t)message_info[0].net_time,message_info[0].net_time,(uint32_t)message_info[0].ack_net_time,message_info[0].ack_net_time,message_info[0].number_of_hops,message_info[0].rssi,message_info[0].src_addr,message_info[0].dst_addr,message_info[0].group_addr);

  wait_for_transition = true; // Self trigger Transition
  ST_transition_cb();
}
void ST_CONTROL_fn(void) {
  bm_cli_log("Ready for Control Message\n");
  bm_led0_set(true);
  bm_sleep(200);
  bm_led0_set(false);
  while (currentState == ST_CONTROL) {
#ifdef BENCHMARK_MASTER
    // Poll for CLI Commands
    if (bm_cli_cmd_getNodeReport.req) {
      bm_control_msg.MACAddressDst = bm_cli_cmd_getNodeReport.MAC;
      bm_control_msg.NextStateNr = ST_REPORT;
      bm_control_msg.GroupAddress = 0;
      bm_control_msg.benchmark_time_s = 0;
      bm_control_msg.benchmark_packet_cnt = 0;
      bm_control_msg_publish(bm_control_msg);
      bm_cli_cmd_getNodeReport.req = false;
      // DO get Node Report
      transition_to_report = true;
      bm_cli_log("Node Reporting initiated\n");
      break;
    } else if (bm_cli_cmd_setNodeSettings.req) {
      bm_control_msg.MACAddressDst = bm_cli_cmd_setNodeSettings.MAC;
      bm_control_msg.NextStateNr = 0;
      bm_control_msg.GroupAddress = bm_cli_cmd_setNodeSettings.GroupAddress;
      bm_control_msg.benchmark_time_s = 0;
      bm_control_msg.benchmark_packet_cnt = 0;
      bm_control_msg_publish(bm_control_msg);
      bm_cli_cmd_setNodeSettings.req = false;
      bm_cli_log("Ready for Control Message\n");
    } else if (bm_cli_cmd_startBM.req) {
      bm_control_msg.MACAddressDst = 0xFFFFFFFF; //Broadcast Address
      bm_control_msg.NextStateNr = ST_TIMESYNC;
      bm_control_msg.GroupAddress = 0;
      bm_control_msg.benchmark_time_s = bm_cli_cmd_startBM.benchmark_time_s;
      bm_control_msg.benchmark_packet_cnt = bm_cli_cmd_startBM.benchmark_packet_cnt;
      bm_control_msg_publish(bm_control_msg);
      bm_cli_cmd_startBM.req = false;
      // DO Start Benchmark
      bm_params.benchmark_time_s = bm_control_msg.benchmark_time_s;
      bm_params.benchmark_packet_cnt = bm_control_msg.benchmark_packet_cnt;
      transition_to_timesync = true;
      bm_cli_log("Benchmark Start initiatet\n");
      break;
    }
#ifdef NRF_SDK_Zigbee
    bm_cli_process();
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
#endif
    bm_sleep(100); // Poll Interval is 100ms
#else              // If not MASTER then Server or Client
    if (bm_control_msg_subscribe(&bm_control_msg)) {
      if (bm_control_msg.benchmark_time_s > 0 && bm_control_msg.benchmark_packet_cnt > 0 && bm_control_msg.benchmark_packet_cnt <= 1000 && bm_control_msg.NextStateNr == ST_TIMESYNC && bm_control_msg.MACAddressDst == 0xFFFFFFFF) {
        // DO Start Benchmark
        bm_params.benchmark_time_s = bm_control_msg.benchmark_time_s;
        bm_params.benchmark_packet_cnt = bm_control_msg.benchmark_packet_cnt;
        transition_to_timesync = true;
        bm_cli_log("Benchmark Start initiated: Time: %us Packet Count: %u\n", bm_params.benchmark_time_s, bm_params.benchmark_packet_cnt);
        bm_led3_set(true);
        bm_sleep(200);
        bm_led3_set(false);
        break;
      } else if (bm_control_msg.MACAddressDst == LSB_MAC_Address && bm_control_msg.GroupAddress > 0) {
        // DO set Node Settings
        bm_params.GroupAddress = bm_control_msg.GroupAddress;
        bm_cli_log("New Settings Saved Group: %u\n", bm_params.GroupAddress);
        bm_cli_log("Ready for Control Message\n");
        bm_led3_set(true);
        bm_sleep(200);
        bm_led3_set(false);
      } else if (bm_control_msg.MACAddressDst == LSB_MAC_Address && bm_control_msg.NextStateNr == ST_REPORT) {
        // DO get Node Report
        transition_to_report = true;
        bm_cli_log("Node Reporting initiated\n");
        bm_led3_set(true);
        bm_sleep(200);
        bm_led3_set(false);
        break;
      }
    }
#endif
  }
  wait_for_transition = true; // Self trigger Transition
  ST_transition_cb();
}

void ST_REPORT_fn(void) {
#ifdef BENCHMARK_MASTER                          // Master wait for Reports
  memset(message_info, 0, sizeof(message_info)); // Erase old Log Buffer Content
  bm_report_msg_subscribe(message_info);         // Get the Reports
#else                                            // Servers and Clients wait for Reports
  bm_report_msg_publish(message_info);           // Send out Reports
  memset(message_info, 0, sizeof(message_info)); // Erase old Log Buffer Content
  bm_log_clear_flash();                          // Clear Flash Area
#endif
  wait_for_transition = true; // Self trigger Transition
  ST_transition_cb();
}

void ST_TIMESYNC_fn(void) {
  next_state_ts_us = (synctimer_getSyncTime() + ST_TIMESYNC_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000);
  synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb);
#ifdef BENCHMARK_MASTER
  while(next_state_ts_us > synctimer_getSyncTime() + ST_MARGIN_TIME_MS * 1000 + 1500 * 1000) // Do Publishing while ther is enough time left
  {
    bm_timesync_msg_publish(false);
  }
#else
  while(next_state_ts_us > synctimer_getSyncTime() + ST_MARGIN_TIME_MS * 1000 + 250 * 1000 && !bm_state_synced) // Do Subscribing while ther is enough time and not already synced
  {
    if (bm_timesync_msg_subscribe(ST_transition_cb)) {
      break; // Only relay once and wait for next State if synced
    }
  }
#endif
  return;
}

void ST_INIT_BENCHMARK_fn(void) {
#ifdef ZEPHYR_BLE_MESH
  next_state_ts_us = (synctimer_getSyncTime() + ST_INIT_BENCHMARK_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000);
#elif defined NRF_SDK_Zigbee
  uint64_t next_state_ts_us = (synctimer_getSyncTime() + ST_INIT_BENCHMARK_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000 + ZBOSS_MAIN_LOOP_ITERATION_TIME_MARGIN_MS * 1000);
#endif
  synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb);
  start_time_ts_us = synctimer_getSyncTime(); // Get the current Timestamp

  bm_rand_init_message_ts();
  bm_log_clear_ram();
  bm_log_clear_flash();

#ifdef ZEPHYR_BLE_MESH
  bm_blemesh_enable(); // Will return faster than the Stack is realy ready... keep on waiting in the transition.
#elif defined NRF_SDK_Zigbee
  /* Initialize Zigbee stack. */
  bm_zigbee_init();
  /** Start Zigbee Stack. */
  bm_zigbee_enable();
  /* Zigbee or Zboss has its own RTOS Scheduler. He owns all the CPU so we let him work while we wait. 
  Note that the check for time left slows down the Zboss stack a bit. Since we are still init the stack this shouldnt be a big deal. */
  while ((synctimer_getSyncTime() - start_time_ts_us) < ST_INIT_BENCHMARK_TIME_MS * 1000) {
    zboss_main_loop_iteration();
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
#ifdef BENCHMARK_MASTER
    bm_cli_process();
#endif
  }
#endif
  return;
}

void ST_BENCHMARK_fn(void) {
  bm_rand_msg_ts_ind = 0;                                                            // Init the Random Timestamp Array INdex
  benchmark_messageing_done = false;
  start_time_ts_us = synctimer_getSyncTime();                                        // Get the current Timestamp
  #ifdef BENCHMARK_CLIENT
  next_state_ts_us = (start_time_ts_us + bm_rand_msg_ts[bm_rand_msg_ts_ind] + 1000); // Add a satfy margin of 1000us incase Random value was 0
  synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb);               // Shedule the Timestamp event
  bm_cli_log("Sheduled first Message at %u, now is %u, start time was %u\n", (uint32_t)next_state_ts_us, (uint32_t)synctimer_getSyncTime(), (uint32_t)start_time_ts_us);
  #else
  next_state_ts_us = (start_time_ts_us + bm_params.benchmark_time_s * 1e6 + ST_MARGIN_TIME_MS * 1000);
  synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb);               // Shedule the Timestamp event
  benchmark_messageing_done = true;
  #endif
#ifdef ZEPHYR_BLE_MESH
// The Benchmark is Timer Interrupt Driven. do Nothing here and wait for transition
#elif defined NRF_SDK_Zigbee
  /* Zigbee or Zboss has its own RTOS Scheduler. He owns all the CPU so we let him work while we wait. 
  Note that the check forcurrent slows down the Zboss stack a bit... this shouldnt be a big deal. */
  while (currentState == ST_BENCHMARK) {
    zboss_main_loop_iteration();
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
#ifdef BENCHMARK_MASTER
    bm_cli_process();
#endif
  }
#endif
  return;
}


#ifdef BENCHMARK_CLIENT
void ST_BENCHMARK_msg_cb(void) {
  /* Call the Benchmark send_message function */
  bm_send_message();
  /* Schedule Next Message */
  bm_rand_msg_ts_ind++;
  //Next Package -> Check if Timestamp is too close or the same and that not end reached
  while ((start_time_ts_us + bm_rand_msg_ts[bm_rand_msg_ts_ind] <= synctimer_getSyncTime() + ST_BENCHMARK_MIN_GAP_TIME_US) && (bm_rand_msg_ts_ind < bm_params.benchmark_packet_cnt)) {
    /* Call the Benchmark send_message function */
    bm_send_message();
    bm_rand_msg_ts_ind++;
  }
  if (bm_rand_msg_ts_ind < bm_params.benchmark_packet_cnt) {
    next_state_ts_us = start_time_ts_us + bm_rand_msg_ts[bm_rand_msg_ts_ind];
    synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb); // Shedule the next Timestamp event
    return;                                                        // Return immidiatly to save time and prevent wait for transition errors
  } else {
    //Finish Benchmark
    next_state_ts_us = (start_time_ts_us + bm_params.benchmark_time_s * 1e6 + ST_MARGIN_TIME_MS * 1000);
    synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb);               // Shedule the Timestamp event
    benchmark_messageing_done = true;
  }
  return;
}
#endif


void ST_SAVE_FLASH_fn(void) {
  next_state_ts_us = (synctimer_getSyncTime() + ST_SAVE_FLASH_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000);
  synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb); // Shedule the Timestamp event
  start_time_ts_us = synctimer_getSyncTime();                          // Get the current Timestamp
  bm_log_save_to_flash();                                              // Save the log to FLASH;

  
  /* Test read FLASH Data 
  uint32_t restored_cnt = bm_log_load_from_flash(); // Restor Log Data from FLASH
  bm_cli_log("Restored %u Bytes from Flash\n", restored_cnt);
  bm_cli_log("First Log Entry: %u %u ...\n", message_info[0].message_id, (uint32_t)message_info[0].net_time);
*/
  /* Do a System Reset */
  NVIC_SystemReset();
  return;
}

void ST_WAIT_FOR_TRANSITION_fn() {
  wait_for_transition = true;
  while (!(transition)) {
#ifdef ZEPHYR_BLE_MESH
    k_sleep(K_FOREVER); // Zephyr Way
#elif defined NRF_SDK_Zigbee
    __SEV();
    __WFE();
    __WFE(); // Wait for Timer Interrupt nRF5SDK Way
#endif
  }
  wait_for_transition = false;
  transition = false;
  return;
}

void bm_statemachine() {
  while (true) {
    transition = false;
    switch (currentState) {
    case ST_INIT:
      ST_INIT_fn();
      break;
    case ST_CONTROL:
      ST_CONTROL_fn();
      break;
    case ST_REPORT:
      ST_REPORT_fn();
      break;
    case ST_TIMESYNC:
      ST_TIMESYNC_fn();
      ST_WAIT_FOR_TRANSITION_fn();
      break;
    case ST_INIT_BENCHMARK:
      ST_INIT_BENCHMARK_fn();
      ST_WAIT_FOR_TRANSITION_fn();
      break;
    case ST_BENCHMARK:
      ST_BENCHMARK_fn();
      ST_WAIT_FOR_TRANSITION_fn();
      break;
    case ST_SAVE_FLASH:
      ST_SAVE_FLASH_fn();
      ST_WAIT_FOR_TRANSITION_fn();
      break;
    }
  }
}