#include "Timer_sync.h"


uint32_t SyncTimerOffset = 0;

/* Timer Callback */
void synctimer_handler(nrf_timer_event_t event_type, void *context)
{
	if (event_type == NRF_TIMER_EVENT_COMPARE1)
	{
		/* For future use */
	}
}

/* Timer init */
extern void synctimer_init()
{
	nrfx_err_t err;
	// Takes 4294s / 71min to expire
	nrfx_timer_config_t synctimer_cfg = {
		.frequency = NRF_TIMER_FREQ_1MHz,
		.mode = NRF_TIMER_MODE_TIMER,
		.bit_width = NRF_TIMER_BIT_WIDTH_32
		//.p_context = (void *) config, // Callback context
	};

	err = nrfx_timer_init(&synctimer, &synctimer_cfg, synctimer_handler);
	if (err != NRFX_SUCCESS)
	{
		printk("nrfx_timer_init failed with: %d\n", err);
	}

	nrfx_timer_clear(&synctimer);
#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
	/* Setup DPPI for Sending and Receiving Timesync Packet */
	nrf_timer_subscribe_set(synctimer.p_reg, NRF_TIMER_TASK_CAPTURE0, nRF53_DPPICH_Tx);
	//NRF_RADIO->PUBLISH_END = 
	nrf_timer_subscribe_set(synctimer.p_reg, NRF_TIMER_TASK_START, nRF53_DPPICH_Rx);
#else
	/* Setup PPI for Sending and Receiving Timesync Packet */
	nrfx_ppi_channel_fork_assign(nRF52_PreDefinedPPICH_Tx, (uint32_t)synctimer.p_reg->TASKS_CAPTURE[1]);
	nrfx_ppi_channel_fork_assign(nRF52_PreDefinedPPICH_Rx, (uint32_t)synctimer.p_reg->TASKS_START);
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
}

/* Clears and starts the timer */
extern void synctimer_start()
{
	// Start the Synctimer
	nrfx_timer_clear(&synctimer);
	nrfx_timer_enable(&synctimer);
}

/* Stops and clears the timer */
extern void synctimer_stop()
{
	// Start the Synctimer
	nrfx_timer_disable(&synctimer);
	nrfx_timer_clear(&synctimer);	
}

/* Get previous Tx sync timestamp */
extern uint32_t synctimer_getSync()
{
	return nrfx_timer_capture_get(&synctimer,NRF_TIMER_CC_CHANNEL1);
}

/* Synchronise the timer with the received offset Timestamp */
extern void synctimer_setSync(uint32_t TxMasterTimeStamp)
{	
	if (nrfx_timer_is_enabled(&synctimer)){
		SyncTimerOffset = TxMasterTimeStamp;	
	}
}

/* Get Synchronised Timestamp */
extern uint32_t synctimer_getSyncTime()
{	
	nrfx_timer_capture(&synctimer,NRF_TIMER_CC_CHANNEL0);
	return nrfx_timer_capture_get(&synctimer,NRF_TIMER_CC_CHANNEL0) + SyncTimerOffset;
}