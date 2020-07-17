#include "bm_config.h"
#include "bm_cli.h"
#include "bm_radio.h"
#include "bm_rand.h"
#include "bm_timesync.h"


#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#include "bm_zigbee.h"
#endif

#include "bm_statemachine.h"

// Wait for Requests to Report or Start the Timesync after Reporting Done
#define ST_IDLE 10
// Master: Broadcast Timesync Packets ---- Slave: Listen for Broadcasts. If synced retransmit Timesync Packets with random Backof Delay for other Slaves
#define ST_TIMESYNC 50
// Benchmark the Mesh Stack and Log the Data
#define ST_BENCHMARK 60
// Save the Logged Data to Flash and Reset Device to shut down the Mesh Stack
#define ST_SAVE_FLASH 70


// Timeslots for the Sates in ms. The Timesync has to be accurate enough.
#define ST_TIMESYNC_TIME_MS 3000 // -> Optimized for 50 Nodes, 3 Channels and BLE LR125kBit
#define ST_BENCHMARK_TIME_MS 10000
#define ST_SAVE_FLASH_TIME_MS 1000

#define ST_MARGIN_TIME_MS 5            // Margin for State Transition (Let the State Terminate)
#define ST_TIMESYNC_BACKOFF_TIME_MS 30 // Backoff time maximal for retransmitt the Timesync Packet -> Should be in good relation to Timesync Timeslot

uint8_t currentState = ST_TIMESYNC; // Init the Statemachine in the Timesync State
bool wait_for_transition = false;   // Signal that a Waiting for a Transition is active
bool transition = false;            // Signal that a Transition has occured

static void ST_transition_cb(void) {
  bm_cli_log("%u%u\n", (uint32_t)(synctimer_getSyncTime() >> 32), (uint32_t)synctimer_getSyncTime());                       // For Debug
  if ((currentState == ST_TIMESYNC && (bm_state_synced && !isTimeMaster)) || (currentState == ST_TIMESYNC && isTimeMaster)) // Not Synced Slave Nodes stay in Timesync State.
  {
    currentState = ST_BENCHMARK;
  } else if (currentState == ST_BENCHMARK) {
    currentState = ST_TIMESYNC;
  }
  if (!wait_for_transition) {
    bm_cli_log("ERROR: Statemachine out of Sync !!!\n");
    bm_cli_log("Please reset Device\n");
  }
  transition = true;
  bm_cli_log("Current State: %d\n", currentState);
}

void ST_TIMESYNC_fn(void) {
  uint64_t ST_BENCHMARK_TS = (synctimer_getSyncTime() + ST_TIMESYNC_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000);
  synctimer_setSyncTimeCompareInt(ST_BENCHMARK_TS, ST_transition_cb);
  if (isTimeMaster) {
    bm_timesync_Publish(ST_TIMESYNC_TIME_MS, ST_BENCHMARK_TS, false);
  } else {
    if (bm_timesync_Subscribe(ST_TIMESYNC_TIME_MS, ST_transition_cb)) {
      while (synctimer_getSyncTimeCompareIntTS() > (synctimer_getSyncTime() + ST_MARGIN_TIME_MS * 1000 + ST_TIMESYNC_BACKOFF_TIME_MS * 1000)) { // Check if there is Time Left for Propagating Timesync further
        bm_sleep(bm_rand_32 % ST_TIMESYNC_BACKOFF_TIME_MS);                                                                                     // Sleep from 0 till Random Backoff Time
        bm_timesync_Publish(0, synctimer_getSyncTimeCompareIntTS(), true);                                                                      // Propagate the Timesync further just once for each Channel
      }
    }
  }
  return;
}

void ST_BENCHMARK_fn(void) {
  uint64_t ST_TIMESYNC_TS = (synctimer_getSyncTime() + ST_BENCHMARK_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000);
  synctimer_setSyncTimeCompareInt(ST_TIMESYNC_TS, ST_transition_cb);
  uint64_t start_time = synctimer_getSyncTime(); // Get the current Timestamp

  #ifdef ZEPHYR_BLE_MESH
  bm_sleep(ST_BENCHMARK_TIME_MS); // Sleep the Rest of the Time
  #elif defined NRF_SDK_Zigbee
  /* Initialize Zigbee stack. */
  bm_zigbee_init();
  /** Start Zigbee Stack. */
  bm_zigbee_enable();
  while (1) {
    zboss_main_loop_iteration();
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
    if (ST_TIMESYNC_TS < (synctimer_getSyncTime() + ST_MARGIN_TIME_MS * 1000 + 1000 * 1000)) { // With additional Time for Exit the Stack
      break;
    }
  }
  #endif
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
  bm_rand_init();
  bm_radio_init();
  synctimer_init();
  synctimer_start();
  while (true) {
    transition = false;
    switch (currentState) {
    case ST_TIMESYNC:
      ST_TIMESYNC_fn();
      break;
    case ST_BENCHMARK:
      ST_BENCHMARK_fn();
      break;
    }
    ST_WAIT_FOR_TRANSITION_fn();
  }
}