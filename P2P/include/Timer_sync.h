#ifdef __cplusplus
extern "C" {
#endif

#ifndef TIMER_SYNC_H
#define TIMER_SYNC_H

#include <stdlib.h>
#include <string.h>
#include <zephyr.h>

#include <nrfx_timer.h>
#include <hal/nrf_radio.h>

#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
#include <nrfx_dppi.h>
/** DPPI Channel from RADIO->EVENTS_END */
#define nRF53_DPPICH_Tx 0
/** DPPI Channel from RADIO->EVENTS_ADDRESS */
#define nRF53_DPPICH_Rx 1
#else
#include <nrfx_ppi.h>
/** PreDefined PPI Channel from RADIO->EVENTS_END */
#define nRF52_PreDefinedPPICH_Tx NRF_PPI_CHANNEL27
/** PreDefined PPI Channel from RADIO->EVENTS_ADDRESS */
#define nRF52_PreDefinedPPICH_Rx NRF_PPI_CHANNEL26
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)

/* Timer used for Synchronisation */
const nrfx_timer_t synctimer = NRFX_TIMER_INSTANCE(2);
extern uint32_t SyncTimerOffset;

/**
* Init the Sync Timer
*
*/
extern void synctimer_init();
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
extern uint32_t synctimer_getSync();
/**
* Synchronise the timer with the received offset Timestamp
*
* @param TxMasterTimeStamp Latest 
*/
extern void synctimer_setSync(uint32_t TxMasterTimeStamp);
/**
* Get Synchronised Timestamp
*
* @return Synchronised Timestamp
*/
extern uint32_t synctimer_getSyncTime();

#endif

#ifdef __cplusplus
}
#endif