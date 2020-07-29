/*
This file is part of Benchamrk-Shared-Library.

Benchamrk-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchamrk-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchamrk-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR 	   :    Raffael Anklin       */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_TIMESYNC_H
#define BM_TIMESYNC_H

extern bool bm_state_synced; // Signal the Synced State of the Timesync
extern bool bm_synctimer_timeout_compare_int; // Signal that a Timeout Compare Interrupt on Synctimer is Occured



/**
* Init the Sync Timer
*
*/
extern void synctimer_init();

/**
* Timestamp Capture clear
*
*/
extern void synctimer_TimeStampCapture_clear();

/**
* Timestamp Capture enable
*
*/
extern void synctimer_TimeStampCapture_enable();
/**
* Timestamp Capture disable
*
*/
extern void synctimer_TimeStampCapture_disable();


/**
* Clears and starts the timer
*/
extern void synctimer_start();

/**
* Stops and clears the timer
*/
extern void synctimer_stop();
/**
* Get previous Tx sync timestamp
*
* @return Latest Tx Timestamp
*/
extern uint64_t synctimer_getTxTimeStamp();
/* Get previous Tx sync timestamp with respect to Timediff to Master */
extern uint64_t synctimer_getSyncedTxTimeStamp();
/**
* Get previous Rx sync timestamp
*
* @return Latest Rx Timestamp
*/
extern uint64_t synctimer_getRxTimeStamp();
/**
* Synchronise the timer with the received offset Timestamp
*
* @param TxMasterTimeStamp Latest 
*/
extern void synctimer_setSync(uint64_t TxMasterTimeStamp);
/**
* Get Synchronised Timestamp
*
* @return Synchronised Timestamp
*/
extern uint64_t synctimer_getSyncTime();
/**
* Sets a synced Time Compare Interrupt (with respect of Synced Time)
*
* @param ts Timestamp for Interrupt
* @param cc_cb Callback on Interrupt
*/
extern void synctimer_setSyncTimeCompareInt(uint64_t ts ,void (*cc_cb)());

/* Gets a synced Time Compare Interrupt Timestamp (with respect of Synced Time) */
uint64_t synctimer_getSyncTimeCompareIntTS();

/* Sets a Compare Interrupt which occurs after the specified time in us */
void synctimer_setCompareInt(uint32_t timeout_ms);

/* Sleeps for the given Timeout in ms */
void bm_sleep(uint32_t timeout_ms);

extern void config_debug_ppi_and_gpiote_radio_state();


/** Publush a Timesync Message 
 * @param relaying Is the publish called from a relaying event or not (sleeps longer)
 * 
*/
void bm_timesync_msg_publish(bool relaying);
/** Subscribe to a Timesync Message 
 * @param transition_cb transition Callback for registring the next State
 * 
*/
bool bm_timesync_msg_subscribe(void (*transition_cb)());

#endif

#ifdef __cplusplus
}
#endif