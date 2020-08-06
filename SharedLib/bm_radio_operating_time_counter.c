/*
This file is part of Benchmark-Shared-Library.

Benchmark-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchmark-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchmark-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR 	   :    Raffael Anklin       */


/* =================== README =======================*/
/* The Radio Operating Time Counter requires a HW-Timer Instance (eg. TIMER3-5 on the NRF52840 or TIMER1-2 on the nRF5340_NETCORE)*/
/* Please enable the Timer in Zephyr prj.conf file (CONFIG_NRFX_TIMER2=y) */

#include <stdlib.h>
#include <string.h>

#include <hal/nrf_timer.h>
#include <hal/nrf_radio.h>

#include "bm_cli.h"
#include "bm_config.h"
#include "bm_radio.h"
#include "bm_radio_operating_time_counter.h"


#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_ZIGBEE
#endif

#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
#include <nrfx_dppi.h>
#else
#include <nrfx_ppi.h>
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)

static NRF_TIMER_Type *op_time_counter = NRF_TIMER2; // SHould be free in Zigbee and BLE_MESH

static uint32_t OverflowCNT = 0;

#ifdef ZEPHYR_BLE_MESH

static k_tid_t wakeup_thread_tid;

/* Zephyr Way */
ISR_DIRECT_DECLARE(bm_op_timer_handler) {
  // Overflow Handler
  if (op_time_counter->EVENTS_COMPARE[0] == true) {
    op_time_counter->EVENTS_COMPARE[0] = false;
    OverflowCNT++;
  }
  k_wakeup(wakeup_thread_tid);
  ISR_DIRECT_PM();
  return 1;
}

#elif defined NRF_SDK_ZIGBEE

//NRF SDK WAY
void TIMER2_IRQHandler(void) {
  // Overflow Handler
  if (op_time_counter->EVENTS_COMPARE[0] == true) {
    op_time_counter->EVENTS_COMPARE[0] = false;
    OverflowCNT++;
  }
}

#endif

/* Timer init */
void bm_op_time_counter_init() {  
  // Takes 4294s / 71min to expire
  nrf_timer_bit_width_set(op_time_counter, NRF_TIMER_BIT_WIDTH_32);
  nrf_timer_frequency_set(op_time_counter, NRF_TIMER_FREQ_1MHz);
  nrf_timer_mode_set(op_time_counter, NRF_TIMER_MODE_TIMER);
#ifdef ZEPHYR_BLE_MESH
  wakeup_thread_tid = k_current_get();
  IRQ_DIRECT_CONNECT(TIMER2_IRQn, 6, bm_op_timer_handler, 0); // Connect Timer ISR Zephyr WAY
  irq_enable(TIMER2_IRQn);                                 // Enable Timer ISR Zephyr WAY
#elif defined NRF_SDK_ZIGBEE
  NVIC_EnableIRQ(TIMER2_IRQn); // Enable Timer ISR NRF SDK WAY
#endif
  nrf_timer_task_trigger(op_time_counter, NRF_TIMER_TASK_CLEAR);
  op_time_counter->CC[0] = 0xFFFFFFFF;  // For Overflow Detection
  op_time_counter->INTENSET |= (uint32_t)NRF_TIMER_INT_COMPARE0_MASK; // Enable Compare Event 5 Interrupt
  OverflowCNT = 0;
}

/* Stop Timer */
void bm_op_time_counter_stop() {
  nrf_timer_task_trigger(op_time_counter, NRF_TIMER_TASK_STOP);
}

/* Clear Timer */
void bm_op_time_counter_clear() {
  nrf_timer_task_trigger(op_time_counter, NRF_TIMER_TASK_CLEAR);
  OverflowCNT = 0;
}

/* Enable Operation Time Capturing */
void bm_op_time_counter_enable() {
#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
  /* Setup DPPI for Tracing Radio Activity */
  NRF_DPPIC->CHEN |= 1 << 0; // Enable Channel 0 DPPI
  NRF_DPPIC->CHEN |= 1 << 1; // Enable Channel 1 DPPI
  NRF_RADIO->PUBLISH_READY |= 1 << 31; // Enable Publishing on CH0
  nrf_timer_subscribe_set(op_time_counter, NRF_TIMER_TASK_START, 0);    
  NRF_RADIO->PUBLISH_DISABLED |= 1 << 31; // Enable Publishing
  NRF_RADIO->PUBLISH_DISABLED|= 1 << 0;  // Enable Publishing on CH1
  nrf_timer_subscribe_set(op_time_counter, NRF_TIMER_TASK_STOP, 1);  
#else
  /* Setup PPI for Tracing Radio Activity */
  NRF_PPI->CH[16].EEP = (uint32_t) & (NRF_RADIO->EVENTS_READY);
  NRF_PPI->CH[16].TEP = (uint32_t) & (op_time_counter->TASKS_START);
  NRF_PPI->CHENSET |= (PPI_CHENSET_CH16_Set << PPI_CHENSET_CH16_Pos);
  NRF_PPI->CH[17].EEP = (uint32_t) & (NRF_RADIO->EVENTS_DISABLED); 
  NRF_PPI->CH[17].TEP = (uint32_t) & (op_time_counter->TASKS_STOP);
  NRF_PPI->CHENSET |= (PPI_CHENSET_CH17_Set << PPI_CHENSET_CH17_Pos);
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
}

/* Disable Operation Time Capturing */
void bm_op_time_counter_disable() {
#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
  nrf_timer_subscribe_clear(synctimer, NRF_TIMER_TASK_START);
  NRF_RADIO->PUBLISH_END &= ~(1 << 31); // Disable Publishing on CH0
  nrf_timer_subscribe_clear(synctimer, NRF_TIMER_TASK_STOP);
  NRF_RADIO->PUBLISH_READY &= ~(1 << 31); // Disable Publishing
  NRF_RADIO->PUBLISH_DISABLED &= ~(1 << 0);  // Disable Publishing on CH1
  NRF_DPPIC->CHEN &= ~(1 << 0);           // Disable Channel 0 DPPI
  NRF_DPPIC->CHEN &= ~(1 << 1);           // Disable Channel 1 DPPI
#else
  NRF_PPI->CH[16].EEP = 0;
  NRF_PPI->CH[16].TEP = 0;
  NRF_PPI->CHENSET &= ~(1 << PPI_CHENSET_CH16_Pos);
  NRF_PPI->CH[17].EEP = 0;
  NRF_PPI->CH[17].TEP = 0;
  NRF_PPI->CHENSET &= ~(1 << PPI_CHENSET_CH17_Pos);
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
}


/* Get Operation Time */
uint64_t bm_op_time_counter_getOPTime() {
  nrf_timer_task_trigger(op_time_counter, nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
  return (((uint64_t)OverflowCNT << 32 | op_time_counter->CC[0]));
}

