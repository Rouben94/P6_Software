#include "bm_config.h"
#include "bm_cli.h"
#include "bm_radio.h"
#include "bm_rand.h"
#include "bm_timesync.h"

#include <stdlib.h>

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#include "bm_blemesh.h"
#include "bm_blemesh_model_handler.h"
#elif defined NRF_SDK_Zigbee
#include "bm_zigbee.h"
#endif

#include "bm_statemachine.h"

// Wait for Requests to Report or Start the Timesync after Reporting Done
#define ST_IDLE 10
// Master: Broadcast Timesync Packets with Parameters for the Benchmark ---- Slave: Listen for Timesync Broadcast. Relay with random Backof Delay for other Slaves
#define ST_TIMESYNC 50
// Init the Mesh Stack till all Nodes are "comissioned" / "provisioned" or whatever it is called in the current stack... -> Stack should be ready for Benchmark
#define ST_INIT_BENCHMARK 60
/* Benchmark the Mesh Stack: Since the Zigbee stack owns the CPU all the Time, do all the work in the Timer callback.
The Benchmark does simulate a button press, which triggers the sending of a benchmark message. This is done over a Timer interrupt callback (State Transition).
The callback structure should be the same over all Mesh Stacks. The Procedure is: 
I.    Log the button press event 
II.   call the stack specific button pressed callback (bm_simulate_button_press_cb) -> this should send the benchmark message (or schedule in Zigbee)
III.  check if end of benchmark is reached (increment counter)
IV.   if no -> schedule next Timer interrupt callback (aka "button press")
V.    if yess -> change to next state*/
#define ST_BENCHMARK 70
// Save the Logged Data to Flash and Reset Device to shut down the Mesh Stack
#define ST_SAVE_FLASH 80

// Timeslots for the Sates in ms. The Timesync has to be accurate enough.
#define ST_TIMESYNC_TIME_MS 3000        // -> Optimized for 50 Nodes, 3 Channels and BLE LR125kBit
#define ST_INIT_BENCHMARK_TIME_MS 10000 // Time required to init the Mesh Stack
// The Benchmark time is obtained by the arameters from Timesync
#define ST_BENCHMARK_MIN_GAP_TIME_US 1000 // Minimal Gap Time to not exit the interrupt context while waiting for another package.
#define ST_SAVE_FLASH_TIME_MS 1000        // Time required to Save Log to Flash

#define ST_MARGIN_TIME_MS 5            // Margin for State Transition (Let the State Terminate)
#define ST_TIMESYNC_BACKOFF_TIME_MS 30 // Backoff time maximal for retransmitt the Timesync Packet -> Should be in good relation to Timesync Timeslot

uint8_t currentState = ST_TIMESYNC; // Init the Statemachine in the Timesync State
uint64_t start_time_ts_us;          // Start Timestamp of a State
uint64_t next_state_ts_us;          // Next Timestamp for sheduled transition
bool wait_for_transition = false;   // Signal that a Waiting for a Transition is active
bool transition = false;            // Signal that a Transition has occured
uint16_t bm_rand_msg_ts_ind;        // Timewindow for a Benchmarkmessage = BenchmarkTime / BenchmarkPcktCnt

static void ST_transition_cb(void)
{
  if ((currentState == ST_TIMESYNC && (bm_state_synced && !isTimeMaster)) || (currentState == ST_TIMESYNC && isTimeMaster)) // Not Synced Slave Nodes stay in Timesync State.
  {
    currentState = ST_INIT_BENCHMARK;
  }
  else if (currentState == ST_INIT_BENCHMARK)
  {
    currentState = ST_BENCHMARK;
  }
  else if (currentState == ST_BENCHMARK)
  {
    /* Call the Benchmark send_message function */
    bm_send_message();
    /* Schedule Next Message */
    bm_rand_msg_ts_ind++;
    if (bm_rand_msg_ts_ind < sizeof(bm_rand_msg_ts) / sizeof(uint32_t))
    {
      //Next Package -> Check if Timestamp is too close or the same
      while (start_time_ts_us + bm_rand_msg_ts[bm_rand_msg_ts_ind] <= synctimer_getSyncTime() + ST_BENCHMARK_MIN_GAP_TIME_US)
      {
        /* Call the Benchmark send_message function */
        bm_send_message();
        bm_rand_msg_ts_ind++;
      }
      next_state_ts_us = start_time_ts_us + bm_rand_msg_ts[bm_rand_msg_ts_ind];
      synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb); // Shedule the next Timestamp event
      return;                                                              // Return immidiatly to save time and prevent wait for transition errors
    }
    else
    {
      //Finish Benchmark
      wait_for_transition = true; // To Prevent Statmachine Error
      currentState = ST_SAVE_FLASH;
    }
  }
  if (!wait_for_transition)
  {
    bm_cli_log("ERROR: Statemachine out of Sync !!!\n");
    bm_cli_log("Please reset Device\n");
  }
  transition = true;
  bm_cli_log("%u%u\n", (uint32_t)(synctimer_getSyncTime() >> 32), (uint32_t)synctimer_getSyncTime()); // For Debug
  bm_cli_log("Current State: %d\n", currentState);
}

void ST_TIMESYNC_fn(void)
{
  next_state_ts_us = (synctimer_getSyncTime() + ST_TIMESYNC_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000);
  synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb);
  if (isTimeMaster)
  {
    bm_timesync_Publish(ST_TIMESYNC_TIME_MS, next_state_ts_us, false);
  }
  else
  {
    if (bm_timesync_Subscribe(ST_TIMESYNC_TIME_MS, ST_transition_cb))
    {
      while (synctimer_getSyncTimeCompareIntTS() > (synctimer_getSyncTime() + ST_MARGIN_TIME_MS * 1000 + ST_TIMESYNC_BACKOFF_TIME_MS * 1000))
      {                                                                    // Check if there is Time Left for Propagating Timesync further
        bm_sleep(bm_rand_32 % ST_TIMESYNC_BACKOFF_TIME_MS);                // Sleep from 0 till Random Backoff Time
        bm_timesync_Publish(0, synctimer_getSyncTimeCompareIntTS(), true); // Propagate the Timesync further just once for each Channel
      }
    }
  }
  return;
}

void ST_INIT_BENCHMARK_fn(void)
{
#ifdef ZEPHYR_BLE_MESH
  next_state_ts_us = (synctimer_getSyncTime() + ST_INIT_BENCHMARK_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000);
#elif defined NRF_SDK_Zigbee
  uint64_t next_state_ts_us = (synctimer_getSyncTime() + ST_INIT_BENCHMARK_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000 + ZBOSS_MAIN_LOOP_ITERATION_TIME_MARGIN_MS * 1000);
#endif
  synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb);
  start_time_ts_us = synctimer_getSyncTime(); // Get the current Timestamp

  // init the Random timestamps for Messaging
  free(bm_rand_msg_ts);                                                                    // Un-reserve the array first
  bm_rand_msg_ts = calloc(bm_params.benchmark_packet_cnt, sizeof(uint32_t));               // Allocate Array in RAM
  bm_rand_get(bm_rand_msg_ts, bm_params.benchmark_packet_cnt * sizeof(uint32_t));          // Genrate Random Values
  bm_rand32_bubbleSort(bm_rand_msg_ts, bm_params.benchmark_packet_cnt * sizeof(uint32_t)); // Sort Random Array
  // Convert to Timesstamps relativ to benchmark Time
  for (int i = 0; i < sizeof(bm_rand_msg_ts) / sizeof(uint32_t); i++)
  {
    bm_rand_msg_ts[i] = (uint32_t)(((double)bm_rand_msg_ts[i] / UINT32_MAX) * (double)bm_params.benchmark_time_s * 1e6); // Be aware of not loosing accuracy
  }

#ifdef ZEPHYR_BLE_MESH
  bm_blemesh_enable(); // Will return faster than the Stack is realy ready... keep on waiting in the transition.
#elif defined NRF_SDK_Zigbee
  /* Initialize Zigbee stack. */
  bm_zigbee_init();
  /** Start Zigbee Stack. */
  bm_zigbee_enable();
  /* Zigbee or Zboss has its own RTOS Scheduler. He owns all the CPU so we let him work while we wait. 
  Note that the check for time left slows down the Zboss stack a bit. Since we are still init the stack this shouldnt be a big deal. */
  while ((synctimer_getSyncTime() - start_time_ts_us) < ST_INIT_BENCHMARK_TIME_MS * 1000)
  {
    zboss_main_loop_iteration();
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
  }
#endif
  return;
}

void ST_BENCHMARK_fn(void)
{
  bm_rand_msg_ts_ind = 0;                                                          // Init the Random Timestamp Array INdex
  start_time_ts_us = synctimer_getSyncTime();                                      // Get the current Timestamp
  next_state_ts_us = start_time_ts_us + bm_rand_msg_ts[bm_rand_msg_ts_ind] + 1000; // Add a satfy margin of 1000us incase Random value was 0
  synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb);             // Shedule the Timestamp event

#ifdef ZEPHYR_BLE_MESH
// The Benchmark is Timer Interrupt Driven. do Nothing here and wait for transition
#elif defined NRF_SDK_Zigbee
  /* Zigbee or Zboss has its own RTOS Scheduler. He owns all the CPU so we let him work while we wait. 
  Note that the check forcurrent slows down the Zboss stack a bit... this shouldnt be a big deal. */
  while (currentState == ST_BENCHMARK)
  {
    zboss_main_loop_iteration();
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
  }
#endif
  return;
}


void ST_SAVE_FLASH_fn(void)
{
  next_state_ts_us = (synctimer_getSyncTime() + ST_SAVE_FLASH_TIME_MS * 1000 + ST_MARGIN_TIME_MS * 1000);
  synctimer_setSyncTimeCompareInt(next_state_ts_us, ST_transition_cb);             // Shedule the Timestamp event
  start_time_ts_us = synctimer_getSyncTime();                                      // Get the current Timestamp
  

#ifdef ZEPHYR_BLE_MESH
// The Benchmark is Timer Interrupt Driven. do Nothing here and wait for transition
#elif defined NRF_SDK_Zigbee
  /* Zigbee or Zboss has its own RTOS Scheduler. He owns all the CPU so we let him work while we wait. 
  Note that the check forcurrent slows down the Zboss stack a bit... this shouldnt be a big deal. */
  while (currentState == ST_BENCHMARK)
  {
    zboss_main_loop_iteration();
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
  }
#endif
  return;
}


void ST_WAIT_FOR_TRANSITION_fn()
{
  wait_for_transition = true;
  while (!(transition))
  {
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

void bm_statemachine()
{
  synctimer_init();
  synctimer_start();
  bm_rand_init();
  bm_radio_init();
  while (true)
  {
    transition = false;
    switch (currentState)
    {
    case ST_TIMESYNC:
      ST_TIMESYNC_fn();
      break;
    case ST_INIT_BENCHMARK:
      ST_INIT_BENCHMARK_fn();
      break;
    case ST_BENCHMARK:
      ST_BENCHMARK_fn();
      break;
    case ST_SAVE_FLASH:
      ST_SAVE_FLASH_fn();
      break;
    }
    ST_WAIT_FOR_TRANSITION_fn();
  }
}