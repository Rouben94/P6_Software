/*
This file is part of P2P-Benchamrk.

P2P-Benchamrk is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

P2P-Benchamrk is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with P2P-Benchamrk.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Author: Raffael  */

#include "Timer_sync.h"

NRF_TIMER_Type * synctimer = NRF_TIMER4;
uint64_t Timestamp_Slave = 0;
uint64_t Timestamp_Master = 0;
uint64_t Timestamp_Diff = 0;
static uint32_t OverflowCNT = 0;
static uint32_t OverflowCNT_int_synced_ts = 0;
static const uint32_t RxChainDelay_us = 35; // Meassured Chain Delay (40 with IEEE803.4.15)

static void (*sync_compare_callback)();


ISR_DIRECT_DECLARE(timer_handler)
{
		// Overflow Handler
	if (synctimer->EVENTS_COMPARE[5] == true)
	{
		synctimer->EVENTS_COMPARE[5] = false;
		OverflowCNT++;
	}
	if (synctimer->EVENTS_COMPARE[4] == true)
	{
		synctimer->EVENTS_COMPARE[4] = false;
		if ((sync_compare_callback != NULL) && (OverflowCNT == OverflowCNT_int_synced_ts)){
			sync_compare_callback();
		}
	}
	ISR_DIRECT_PM();
	return 1;
}


/*
static void synctimer_handler(){
	// Overflow Handler
	if (synctimer->EVENTS_COMPARE[5] == true)
	{
		synctimer->EVENTS_COMPARE[5] = false;
		OverflowCNT++;
	}
	if (synctimer->EVENTS_COMPARE[4] == true)
	{
		synctimer->EVENTS_COMPARE[4] = false;
		if ((sync_compare_callback != NULL) && (OverflowCNT == OverflowCNT_int_synced_ts)){
			sync_compare_callback();
		}
	}
	printk("hoi\n");
}
*/

/* Timer init */
extern void synctimer_init()
{
	// Takes 4294s / 71min to expire
	nrf_timer_bit_width_set(synctimer,NRF_TIMER_BIT_WIDTH_32);
	nrf_timer_frequency_set(synctimer,NRF_TIMER_FREQ_1MHz);
	nrf_timer_mode_set(synctimer,NRF_TIMER_MODE_TIMER);
	//irq_connect_dynamic(TIMER3_IRQn, 7, synctimer_handler, NULL, 0); 	// Connect Radio ISR
	IRQ_DIRECT_CONNECT(TIMER4_IRQn, 6, timer_handler, 0);
    irq_enable(TIMER4_IRQn);                                        // Enable Radio ISR
	nrf_timer_task_trigger(synctimer, NRF_TIMER_TASK_CLEAR);
	nrf_timer_cc_set(synctimer,NRF_TIMER_CC_CHANNEL1,0);
	nrf_timer_cc_set(synctimer,NRF_TIMER_CC_CHANNEL2,0);
	nrf_timer_cc_set(synctimer,NRF_TIMER_CC_CHANNEL5,0xFFFFFFFF); // For Overflow Detection
	synctimer->INTENSET |= (uint32_t) NRF_TIMER_INT_COMPARE5_MASK; // Enable Compare Event 5 Interrupt
	OverflowCNT = 0;
}

/* Timestamp Capture Clear */
void synctimer_TimeStampCapture_clear()
{
	synctimer->CC[1] = 0;
	synctimer->CC[2] = 0;
}

/* Timestamp Capture Enable */
extern void synctimer_TimeStampCapture_enable()
{
#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
	/* Setup DPPI for Sending and Receiving Timesync Packet */
	NRF_DPPIC->CHEN |= 1 << 0; // Enable Channel 0 DPPI
	NRF_DPPIC->CHEN |= 1 << 1; // Enable Channel 1 DPPI
	nrf_timer_subscribe_set(synctimer, NRF_TIMER_TASK_CAPTURE1, 0);
	NRF_RADIO->PUBLISH_END |= 1 << 31; // Enable Publishing on CH0
	nrf_timer_subscribe_set(synctimer, NRF_TIMER_TASK_CAPTURE2, 1);
	NRF_RADIO->PUBLISH_CRCOK |= 1 << 31; // Enable Publishing
	NRF_RADIO->PUBLISH_CRCOK |= 1 << 0;	 // Enable Publishing on CH1
#else
	/* Setup PPI for Sending and Receiving Timesync Packet */
	//nrfx_ppi_channel_fork_assign(nRF52_PreDefinedPPICH_Tx, (uint32_t)synctimer.p_reg->TASKS_CAPTURE[1]);
	NRF_PPI->CH[18].EEP = (uint32_t) & (NRF_RADIO->EVENTS_END);
	NRF_PPI->CH[18].TEP = (uint32_t) & (synctimer->TASKS_CAPTURE[1]);
	NRF_PPI->CHENSET |= (PPI_CHENSET_CH18_Set << PPI_CHENSET_CH18_Pos);
	//nrfx_ppi_channel_enable(nRF52_PreDefinedPPICH_Tx);
	//nrfx_ppi_channel_fork_assign(nRF52_PreDefinedPPICH_Rx, (uint32_t)synctimer.p_reg->TASKS_START);
	//nrfx_ppi_channel_enable(nRF52_PreDefinedPPICH_Rx);
	//NRF_PPI->FORK[26].TEP = (uint32_t)&(synctimer.p_reg->TASKS_CAPTURE[1]);
	//NRF_PPI->CHENSET = (PPI_CHENSET_CH26_Set << PPI_CHENSET_CH26_Pos);
	NRF_PPI->CH[19].EEP = (uint32_t) & (NRF_RADIO->EVENTS_CRCOK); // 40us Delay
	NRF_PPI->CH[19].TEP = (uint32_t) & (synctimer->TASKS_CAPTURE[2]);
	NRF_PPI->CHENSET |= (PPI_CHENSET_CH19_Set << PPI_CHENSET_CH19_Pos);
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
}

/* Timestamp Capture Disable */
extern void synctimer_TimeStampCapture_disable()
{
#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
	/* Setup DPPI for Sending and Receiving Timesync Packet */
	nrf_timer_subscribe_clear(synctimer, NRF_TIMER_TASK_CAPTURE1);
	NRF_RADIO->PUBLISH_END &= ~(1 << 31); // Disable Publishing on CH0
	nrf_timer_subscribe_clear(synctimer, NRF_TIMER_TASK_CAPTURE2);
	NRF_RADIO->PUBLISH_CRCOK &= ~(1 << 31); // Disable Publishing
	NRF_RADIO->PUBLISH_CRCOK &= ~(1 << 0);	 // Disable Publishing on CH1
	NRF_DPPIC->CHEN &= ~(1 << 0);			 // Disable Channel 0 DPPI
	NRF_DPPIC->CHEN &= ~(1 << 1);			 // Disable Channel 1 DPPI
#else
	/* Reset PPI for Sending and Receiving Timesync Packet */
	NRF_PPI->CH[18].EEP = 0;
	NRF_PPI->CH[18].TEP = 0;
	NRF_PPI->CHENSET &= ~(1 << PPI_CHENSET_CH18_Pos);
	NRF_PPI->CH[19].EEP = 0;
	NRF_PPI->CH[19].TEP = 0;
	NRF_PPI->CHENSET &= ~(1 << PPI_CHENSET_CH19_Pos);
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
}

/* Clears and starts the timer */
extern void synctimer_start()
{
	// Start the Synctimer
	nrf_timer_task_trigger(synctimer, NRF_TIMER_TASK_CLEAR);
	nrf_timer_task_trigger(synctimer, NRF_TIMER_TASK_START);
	OverflowCNT = 0;
}

/* Stops and clears the timer */
extern void synctimer_stop()
{
	// Stop the Synctimer
	nrf_timer_task_trigger(synctimer, NRF_TIMER_TASK_SHUTDOWN);
	nrf_timer_task_trigger(synctimer, NRF_TIMER_TASK_CLEAR);
	OverflowCNT = 0;
}

/* Get previous Tx sync timestamp */
extern uint64_t synctimer_getTxTimeStamp()
{
	return ((uint64_t) OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL1));
}

/* Get previous Rx sync timestamp */
extern uint64_t synctimer_getRxTimeStamp()
{
	return ((uint64_t) OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL2));
}

/* Synchronise the timer offset with the received offset Timestamp */
extern void synctimer_setSync(uint64_t TxMasterTimeStamp)
{
	Timestamp_Slave = ((uint64_t) OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL2)) - RxChainDelay_us;
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
extern uint64_t synctimer_getSyncTime()
{
	if (Timestamp_Slave > Timestamp_Master)
	{
		nrf_timer_task_trigger(synctimer,nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
		return (((uint64_t) OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL0)) - Timestamp_Diff);
	}
	else if ((Timestamp_Master > Timestamp_Slave))
	{
		nrf_timer_task_trigger(synctimer,nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
		return (((uint64_t) OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL0)) + Timestamp_Diff);
	}
	else
	{
		nrf_timer_task_trigger(synctimer,nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
		return ((uint64_t) OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL0));
	}
}

/* Sets a synced Time Compare Interrupt (with respect of Synced Time) */
extern void synctimer_setSyncTimeCompareInt(uint64_t ts ,void (*cc_cb)())
{
	//printk("%llu\n",ts- synctimer_getSyncTime());
	if (Timestamp_Slave > Timestamp_Master)
	{
		OverflowCNT_int_synced_ts = (uint32_t)(ts>>32) + (uint32_t)(Timestamp_Diff>>32); 
		synctimer->CC[4] = (uint32_t)ts + (uint32_t)Timestamp_Diff; 
	}
	else if ((Timestamp_Master > Timestamp_Slave))
	{
		OverflowCNT_int_synced_ts = (uint32_t)(ts>>32) - (uint32_t)(Timestamp_Diff>>32); 
		synctimer->CC[4] = (uint32_t)ts - (uint32_t)Timestamp_Diff; 
	}
	else
	{
		OverflowCNT_int_synced_ts = (uint32_t)(ts>>32); 
		synctimer->CC[4] = (uint32_t)ts; 
	}
	//printk("%u\n",synctimer->CC[4]);
	//nrf_timer_task_trigger(synctimer,nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
	//printk("%u\n",synctimer->CC[4] - synctimer->CC[0]);
	synctimer->INTENSET |= (uint32_t) NRF_TIMER_INT_COMPARE4_MASK; // Enable Compare Event 4 Interrupt
	sync_compare_callback = cc_cb;
}

void config_debug_ppi_and_gpiote_radio_state()
{
	NRF_P0->PIN_CNF[14] = NRF_P0->PIN_CNF[15]; // Copy Configuration from LED
	NRF_P0->DIRSET |= 1 << 14;
	NRF_P0->DIR |= 1 << 14;
	//Radio Tx
	NRF_GPIOTE->CONFIG[0] = ((GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
							(14 << GPIOTE_CONFIG_PSEL_Pos) |
							(0 << GPIOTE_CONFIG_PORT_Pos) |
							(GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
							(GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos));
	
	//NRF_PPI->CH[7].EEP = (uint32_t) & NRF_RADIO->EVENTS_TXREADY;
	NRF_PPI->CH[7].EEP = (uint32_t) & NRF_TIMER4->EVENTS_COMPARE[4];
	NRF_PPI->CH[7].TEP = (uint32_t) & NRF_GPIOTE->TASKS_OUT[0];
	NRF_PPI->CHENSET |= (PPI_CHENSET_CH7_Set << PPI_CHENSET_CH7_Pos);
	//Radio Rx
	NRF_GPIOTE->CONFIG[1] = ((GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
							(1 << GPIOTE_CONFIG_PSEL_Pos) |
							(0 << GPIOTE_CONFIG_PORT_Pos) |
							(GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
							(GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos));
	//NRF_PPI->CH[8].EEP = (uint32_t) & NRF_RADIO->EVENTS_RXREADY;
	NRF_PPI->CH[8].EEP = (uint32_t) & NRF_TIMER4->EVENTS_COMPARE[4];
	NRF_PPI->CH[8].TEP = (uint32_t) & NRF_GPIOTE->TASKS_OUT[1];
	NRF_PPI->CHENSET |= (PPI_CHENSET_CH8_Set << PPI_CHENSET_CH8_Pos);

	//Reset Tx LEDs
	/*
	NRF_GPIOTE->CONFIG[2] = (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
							(0 << GPIOTE_CONFIG_PSEL_Pos) |
							(0 << GPIOTE_CONFIG_PORT_Pos) |
							(GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos) |
							(GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos);
	NRF_PPI->CH[9].EEP = (uint32_t) & NRF_RADIO->EVENTS_RXREADY;
	NRF_PPI->CH[9].TEP = (uint32_t) & NRF_GPIOTE->TASKS_OUT[2];
	NRF_PPI->CHENSET = (PPI_CHENSET_CH9_Set << PPI_CHENSET_CH9_Pos);
	//Reset Rx LEDs
	NRF_GPIOTE->CONFIG[3] = (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
							(1 << GPIOTE_CONFIG_PSEL_Pos) |
							(0 << GPIOTE_CONFIG_PORT_Pos) |
							(GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos) |
							(GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos);
	NRF_PPI->CH[10].EEP = (uint32_t) & NRF_RADIO->EVENTS_TXREADY;
	NRF_PPI->CH[10].TEP = (uint32_t) & NRF_GPIOTE->TASKS_OUT[3];
	NRF_PPI->CHENSET = (PPI_CHENSET_CH10_Set << PPI_CHENSET_CH10_Pos);
	*/
}
