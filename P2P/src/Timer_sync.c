#include "Timer_sync.h"

uint32_t Timestamp_Slave = 0;
uint32_t Timestamp_Master = 0;
uint32_t Timestamp_Diff = 0;

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
}

/* Timestamp Capture Enable */
extern void synctimer_TimeStampCapture_enable(){
	#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
	/* Setup DPPI for Sending and Receiving Timesync Packet */
	NRF_DPPIC->CHEN |= 1 << 0; // Enable Channel 0 DPPI
	NRF_DPPIC->CHEN |= 1 << 1; // Enable Channel 1 DPPI
	nrf_timer_subscribe_set(synctimer.p_reg, NRF_TIMER_TASK_CAPTURE1, 0);
	NRF_RADIO->PUBLISH_END |= 1 << 31; // Enable Publishing on CH0
	nrf_timer_subscribe_set(synctimer.p_reg, NRF_TIMER_TASK_CAPTURE2, 1);
	NRF_RADIO->PUBLISH_CRCOK |= 1 << 31; // Enable Publishing
	NRF_RADIO->PUBLISH_CRCOK |= 1 << 0;	 // Enable Publishing on CH1
#else
	/* Setup PPI for Sending and Receiving Timesync Packet */
	//nrfx_ppi_channel_fork_assign(nRF52_PreDefinedPPICH_Tx, (uint32_t)synctimer.p_reg->TASKS_CAPTURE[1]);
	NRF_PPI->FORK[27].TEP = (uint32_t) & (synctimer.p_reg->TASKS_CAPTURE[1]);
	NRF_PPI->CHENSET = (PPI_CHENSET_CH27_Set << PPI_CHENSET_CH27_Pos);
	//nrfx_ppi_channel_enable(nRF52_PreDefinedPPICH_Tx);
	//nrfx_ppi_channel_fork_assign(nRF52_PreDefinedPPICH_Rx, (uint32_t)synctimer.p_reg->TASKS_START);
	//nrfx_ppi_channel_enable(nRF52_PreDefinedPPICH_Rx);
	//NRF_PPI->FORK[26].TEP = (uint32_t)&(synctimer.p_reg->TASKS_CAPTURE[1]);
	//NRF_PPI->CHENSET = (PPI_CHENSET_CH26_Set << PPI_CHENSET_CH26_Pos);
	NRF_PPI->CH[19].EEP = (uint32_t) & (NRF_RADIO->EVENTS_CRCOK);
	NRF_PPI->CH[19].TEP = (uint32_t) & (synctimer.p_reg->TASKS_CAPTURE[2]);
	NRF_PPI->CHENSET = (PPI_CHENSET_CH19_Set << PPI_CHENSET_CH19_Pos);
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
}

/* Timestamp Capture Disable */
extern void synctimer_TimeStampCapture_disable(){
	#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
	/* Setup DPPI for Sending and Receiving Timesync Packet */
	nrf_timer_subscribe_clear(synctimer.p_reg, NRF_TIMER_TASK_CAPTURE1);
	NRF_RADIO->PUBLISH_END |= 0 << 31; // Disable Publishing on CH0
	nrf_timer_subscribe_clear(synctimer.p_reg, NRF_TIMER_TASK_CAPTURE2);
	NRF_RADIO->PUBLISH_CRCOK |= 0 << 31; // Disable Publishing
	NRF_RADIO->PUBLISH_CRCOK |= 0 << 0;	 // Disable Publishing on CH1
	NRF_DPPIC->CHEN |= 0 << 0; // Disable Channel 0 DPPI
	NRF_DPPIC->CHEN |= 0 << 1; // Disable Channel 1 DPPI
#else
	/* Setup PPI for Sending and Receiving Timesync Packet */
	NRF_PPI->FORK[27].TEP = 0;
	NRF_PPI->CHENSET = (0 << PPI_CHENSET_CH27_Pos);
	NRF_PPI->CH[19].EEP = 0;
	NRF_PPI->CH[19].TEP = 0;
	NRF_PPI->CHENSET = (0 << PPI_CHENSET_CH19_Pos);
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
extern uint32_t synctimer_getTxTimeStamp()
{
	return nrfx_timer_capture_get(&synctimer, NRF_TIMER_CC_CHANNEL1);
}

/* Get previous Rx sync timestamp */
extern uint32_t synctimer_getRxTimeStamp()
{
	return nrfx_timer_capture_get(&synctimer, NRF_TIMER_CC_CHANNEL2);
}

/* Synchronise the timer offset with the received offset Timestamp */
extern void synctimer_setSync(uint32_t TxMasterTimeStamp)
{
	Timestamp_Slave = nrfx_timer_capture_get(&synctimer, NRF_TIMER_CC_CHANNEL2);
	Timestamp_Master = TxMasterTimeStamp;
	if (Timestamp_Slave > Timestamp_Master)
	{
		Timestamp_Diff = Timestamp_Slave - Timestamp_Master;
	}
	else if ((Timestamp_Master > Timestamp_Slave))
	{
		Timestamp_Diff = Timestamp_Master - Timestamp_Slave;
	}
	else
	{
		Timestamp_Diff = 0;
	}
}

/* Get Synchronised Timestamp */
extern uint32_t synctimer_getSyncTime()
{
	nrfx_timer_capture(&synctimer, NRF_TIMER_CC_CHANNEL0);
	u32_t timestamp_now = nrfx_timer_capture_get(&synctimer, NRF_TIMER_CC_CHANNEL0);
	if (Timestamp_Slave > Timestamp_Master)
	{
		return timestamp_now - Timestamp_Diff;
	}
	else if ((Timestamp_Master > Timestamp_Slave))
	{
		return timestamp_now + Timestamp_Diff;
	}
	else
	{
		return timestamp_now;
	}
}