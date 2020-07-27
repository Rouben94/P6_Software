#ifdef __cplusplus
extern "C" {
#endif

#ifndef TIMER_SYNC_H
#define TIMER_SYNC_H

#include <stdlib.h>
#include <string.h>
//#include <zephyr.h>

#include <nrfx_timer.h>
#include <hal/nrf_timer.h>
#include <hal/nrf_radio.h>

#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
#include <nrfx_dppi.h>
#else
#include <nrfx_ppi.h>
/** PreDefined PPI Channel from RADIO->EVENTS_END */
#define nRF52_PreDefinedPPICH_Tx NRF_PPI_CHANNEL27
/** PreDefined PPI Channel from RADIO->EVENTS_ADDRESS */
#define nRF52_PreDefinedPPICH_Rx NRF_PPI_CHANNEL26
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)

/* Timer used for Synchronisation */
extern NRF_TIMER_Type * synctimer;
extern uint64_t Timestamp_Slave;
extern uint64_t Timestamp_Master;
extern uint64_t Timestamp_Diff;

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

extern void config_debug_ppi_and_gpiote_radio_state();

#endif

#ifdef __cplusplus
}
#endif