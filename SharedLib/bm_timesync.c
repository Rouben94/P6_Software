/* =================== README =======================*/
/* The Timesync requires a HW-Timer Instance with at least 5 CC Registers (eg. TIMER3-5 on the NRF52840 or TIMER1-2 on the nRF5340_NETCORE)*/
/* Please enable the Timer in Zephyr prj.conf file (CONFIG_NRFX_TIMER2=y) */

#include <stdlib.h>
#include <string.h>

#include <hal/nrf_timer.h>
#include <nrfx_timer.h>

#include "bm_config.h"
#include "bm_cli.h"
#include "bm_radio.h"
#include "bm_timesync.h"
#include "bm_rand.h"

#ifdef ZEPHYR_BLE_MESH
#include <zephyr.h>
#elif defined NRF_SDK_Zigbee
#endif

#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
#include <nrfx_dppi.h>
#else
#include <nrfx_ppi.h>
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)

static NRF_TIMER_Type *synctimer = NRF_TIMER4;
static uint64_t Timestamp_Slave = 0;
static uint64_t Timestamp_Master = 0;
static uint64_t Timestamp_Diff = 0;
static uint32_t OverflowCNT = 0;
static uint32_t OverflowCNT_int_synced_ts = 0;
static const uint32_t RxChainDelay_us = 35; // Meassured Chain Delay (40 with IEEE803.4.15)

static void (*sync_compare_callback)();
bool bm_synctimer_timeout_compare_int = false; // Simple Flag for signaling the Compare interrupt

bool bm_state_synced = false; // Init the Synced State

#ifdef ZEPHYR_BLE_MESH

k_tid_t wakeup_thread_tid;

/* Zephyr Way */
ISR_DIRECT_DECLARE(bm_timer_handler)
{
  // Overflow Handler
  if (synctimer->EVENTS_COMPARE[5] == true)
  {
    synctimer->EVENTS_COMPARE[5] = false;
    OverflowCNT++;
  }
  // Transistion Handler
  if (synctimer->EVENTS_COMPARE[4] == true)
  {
    synctimer->EVENTS_COMPARE[4] = false;
    if ((sync_compare_callback != NULL) && (OverflowCNT == OverflowCNT_int_synced_ts))
    {
      sync_compare_callback();
    }
  }
  // Sleep Handler
  if (synctimer->EVENTS_COMPARE[3] == true)
  {
    //bm_cli_log("Interrupt CC3\n");
    bm_synctimer_timeout_compare_int = true;
    synctimer->EVENTS_COMPARE[3] = false;
    synctimer->INTENSET &= ~((uint32_t)NRF_TIMER_INT_COMPARE3_MASK); // Disable Compare Event 0 Interrupt
  }
  k_wakeup(wakeup_thread_tid);
  ISR_DIRECT_PM();
  return 1;
}

#elif defined NRF_SDK_Zigbee

//NRF SDK WAY
void TIMER4_IRQHandler(void)
{
  // Overflow Handler
  if (synctimer->EVENTS_COMPARE[5] == true)
  {
    synctimer->EVENTS_COMPARE[5] = false;
    OverflowCNT++;
  }
  // Transistion Handler
  if (synctimer->EVENTS_COMPARE[4] == true)
  {
    synctimer->EVENTS_COMPARE[4] = false;
    if ((sync_compare_callback != NULL) && (OverflowCNT == OverflowCNT_int_synced_ts))
    {
      sync_compare_callback();
    }
  }
  // Sleep Handler
  if (synctimer->EVENTS_COMPARE[3] == true)
  {
    //bm_cli_log("Interrupt CC3\n");
    bm_synctimer_timeout_compare_int = true;
    synctimer->EVENTS_COMPARE[3] = false;
    synctimer->INTENSET &= ~((uint32_t)NRF_TIMER_INT_COMPARE3_MASK); // Disable Compare Event 0 Interrupt
  }
}

#endif

/* Timer init */
extern void synctimer_init()
{

  // Takes 4294s / 71min to expire
  nrf_timer_bit_width_set(synctimer, NRF_TIMER_BIT_WIDTH_32);
  nrf_timer_frequency_set(synctimer, NRF_TIMER_FREQ_1MHz);
  nrf_timer_mode_set(synctimer, NRF_TIMER_MODE_TIMER);
#ifdef ZEPHYR_BLE_MESH
  wakeup_thread_tid = k_current_get();
  IRQ_DIRECT_CONNECT(TIMER4_IRQn, 6, bm_timer_handler, 0); // Connect Timer ISR Zephyr WAY
  irq_enable(TIMER4_IRQn);                                 // Enable Timer ISR Zephyr WAY
#elif defined NRF_SDK_Zigbee

  NVIC_EnableIRQ(TIMER4_IRQn); // Enable Timer ISR NRF SDK WAY
#endif
  nrf_timer_task_trigger(synctimer, NRF_TIMER_TASK_CLEAR);
  synctimer->CC[1] = 0;
  synctimer->CC[2] = 0;
  synctimer->CC[5] = 0xFFFFFFFF;                                // For Overflow Detection
  synctimer->INTENSET |= (uint32_t)NRF_TIMER_INT_COMPARE5_MASK; // Enable Compare Event 5 Interrupt
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
  NRF_RADIO->PUBLISH_CRCOK |= 1 << 0;  // Enable Publishing on CH1
#else
  /* Setup PPI for Sending and Receiving Timesync Packet */
  NRF_PPI->CH[18].EEP = (uint32_t) & (NRF_RADIO->EVENTS_END);
  NRF_PPI->CH[18].TEP = (uint32_t) & (synctimer->TASKS_CAPTURE[1]);
  NRF_PPI->CHENSET |= (PPI_CHENSET_CH18_Set << PPI_CHENSET_CH18_Pos);
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
  NRF_RADIO->PUBLISH_CRCOK &= ~(1 << 0);  // Disable Publishing on CH1
  NRF_DPPIC->CHEN &= ~(1 << 0);           // Disable Channel 0 DPPI
  NRF_DPPIC->CHEN &= ~(1 << 1);           // Disable Channel 1 DPPI
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
  return ((uint64_t)OverflowCNT << 32 | synctimer->CC[1]) + Timestamp_Diff;
}

/* Get previous Tx sync timestamp with respect to Timediff to Master */
extern uint64_t synctimer_getSyncedTxTimeStamp()
{
  if (Timestamp_Slave > Timestamp_Master)
  {
    //Timestamp_Diff = Timestamp_Slave - Timestamp_Master;
    return ((uint64_t)OverflowCNT << 32 | synctimer->CC[1]) - Timestamp_Diff;
  }
  else if ((Timestamp_Master > Timestamp_Slave))
  {
    //Timestamp_Diff = Timestamp_Master - Timestamp_Slave;
    return ((uint64_t)OverflowCNT << 32 | synctimer->CC[1]) + Timestamp_Diff;
  }
  else
  {
    //Timestamp_Diff = 0;
    return ((uint64_t)OverflowCNT << 32 | synctimer->CC[1]);
  }
}

/* Get previous Rx sync timestamp */
extern uint64_t synctimer_getRxTimeStamp()
{
  return ((uint64_t)OverflowCNT << 32 | synctimer->CC[2]);
}

/* Synchronise the timer offset with the received offset Timestamp */
extern void synctimer_setSync(uint64_t TxMasterTimeStamp)
{
  Timestamp_Slave = ((uint64_t)OverflowCNT << 32 | synctimer->CC[2]) - RxChainDelay_us;
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
    nrf_timer_task_trigger(synctimer, nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
    return (((uint64_t)OverflowCNT << 32 | synctimer->CC[0]) - Timestamp_Diff);
  }
  else if ((Timestamp_Master > Timestamp_Slave))
  {
    nrf_timer_task_trigger(synctimer, nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
    return (((uint64_t)OverflowCNT << 32 | synctimer->CC[0]) + Timestamp_Diff);
  }
  else
  {
    nrf_timer_task_trigger(synctimer, nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
    return ((uint64_t)OverflowCNT << 32 | synctimer->CC[0]);
  }
}

/* Sets a synced Time Compare Interrupt (with respect of Synced Time) */
extern void synctimer_setSyncTimeCompareInt(uint64_t ts, void (*cc_cb)())
{
  //bm_cli_log("%llu\n",ts- synctimer_getSyncTime());
  if (Timestamp_Slave > Timestamp_Master)
  {
    OverflowCNT_int_synced_ts = (uint32_t)(ts >> 32) + (uint32_t)(Timestamp_Diff >> 32);
    synctimer->CC[4] = (uint32_t)ts + (uint32_t)Timestamp_Diff;
  }
  else if ((Timestamp_Master > Timestamp_Slave))
  {
    OverflowCNT_int_synced_ts = (uint32_t)(ts >> 32) - (uint32_t)(Timestamp_Diff >> 32);
    synctimer->CC[4] = (uint32_t)ts - (uint32_t)Timestamp_Diff;
  }
  else
  {
    OverflowCNT_int_synced_ts = (uint32_t)(ts >> 32);
    synctimer->CC[4] = (uint32_t)ts;
  }
  //bm_cli_log("%u\n",synctimer->CC[4]);
  //nrf_timer_task_trigger(synctimer,nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
  //bm_cli_log("%u\n",synctimer->CC[4] - synctimer->CC[0]);
  synctimer->INTENSET |= (uint32_t)NRF_TIMER_INT_COMPARE4_MASK; // Enable Compare Event 4 Interrupt
  sync_compare_callback = cc_cb;
}

/* Gets a synced Time Compare Interrupt Timestamp (with respect of Synced Time) */
uint64_t synctimer_getSyncTimeCompareIntTS()
{
  //bm_cli_log("%llu\n",ts- synctimer_getSyncTime());
  uint32_t msb_ts, lsb_ts;
  if (Timestamp_Slave > Timestamp_Master)
  {
    msb_ts = OverflowCNT_int_synced_ts - (uint32_t)(Timestamp_Diff >> 32);
    //OverflowCNT_int_synced_ts = (uint32_t)(ts >> 32) + (uint32_t)(Timestamp_Diff >> 32);
    lsb_ts = synctimer->CC[4] - (uint32_t)Timestamp_Diff;
    //synctimer->CC[4] = (uint32_t)ts + (uint32_t)Timestamp_Diff;
  }
  else if ((Timestamp_Master > Timestamp_Slave))
  {
    msb_ts = OverflowCNT_int_synced_ts + (uint32_t)(Timestamp_Diff >> 32);
    //OverflowCNT_int_synced_ts = (uint32_t)(ts >> 32) - (uint32_t)(Timestamp_Diff >> 32);
    lsb_ts = synctimer->CC[4] + (uint32_t)Timestamp_Diff;
    //synctimer->CC[4] = (uint32_t)ts - (uint32_t)Timestamp_Diff;
  }
  else
  {
    msb_ts = OverflowCNT_int_synced_ts;
    //OverflowCNT_int_synced_ts = (uint32_t)(ts >> 32);
    lsb_ts = synctimer->CC[4];
    //synctimer->CC[4] = (uint32_t)ts;
  }
  return (uint64_t)msb_ts << 32 | lsb_ts;
}

/* Sets a Compare Interrupt which occurs after the specified time in us */
void synctimer_setCompareInt(uint32_t timeout_ms)
{
  synctimer->EVENTS_COMPARE[3] = false;
  nrf_timer_task_trigger(synctimer, nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
  synctimer->CC[3] = synctimer->CC[0] + timeout_ms * 1000;
  synctimer->INTENSET |= (uint32_t)NRF_TIMER_INT_COMPARE3_MASK; // Enable Compare Event 3 Interrupt
}

/* Sleeps for the given Timeout in ms */
void bm_sleep(uint32_t timeout_ms)
{
  bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
  synctimer_setCompareInt(timeout_ms);
  while (!(bm_synctimer_timeout_compare_int))
  {
#ifdef ZEPHYR_BLE_MESH
    k_sleep(K_FOREVER); // Zephyr Way
#elif defined NRF_SDK_Zigbee
    __SEV();
    __WFE();
    __WFE(); // Wait for Timer Interrupt nRF5SDK Way
#endif
  }
  bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
  return;
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
  NRF_PPI->CH[7].EEP = (uint32_t)&NRF_TIMER4->EVENTS_COMPARE[4];
  NRF_PPI->CH[7].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[0];
  NRF_PPI->CHENSET |= (PPI_CHENSET_CH7_Set << PPI_CHENSET_CH7_Pos);
  //Radio Rx
  NRF_GPIOTE->CONFIG[1] = ((GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
                           (1 << GPIOTE_CONFIG_PSEL_Pos) |
                           (0 << GPIOTE_CONFIG_PORT_Pos) |
                           (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
                           (GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos));
  //NRF_PPI->CH[8].EEP = (uint32_t) & NRF_RADIO->EVENTS_RXREADY;
  NRF_PPI->CH[8].EEP = (uint32_t)&NRF_TIMER4->EVENTS_COMPARE[4];
  NRF_PPI->CH[8].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[1];
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

/* ============================ Timesync Process ===============================*/

#define TimesyncAddress 0x9CE74F9A
#define CommonMode NRF_RADIO_MODE_BLE_1MBIT         // Common Mode
#define CommonStartCH 37                            // Common Start Channel
#define CommonEndCH 39                              // Common End Channel
#define CommonCHCnt CommonEndCH - CommonStartCH + 1 // Common Channel Count
// Selection Between nrf52840 and nrf5340 of TxPower -> The highest available for Common Channel is recomended
#if defined(RADIO_TXPOWER_TXPOWER_Pos8dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_POS8DBM // Common Tx Power < 8 dBm
#elif defined(RADIO_TXPOWER_TXPOWER_0dBm) || defined(__NRFX_DOXYGEN__)
#define CommonTxPower NRF_RADIO_TXPOWER_0DBM // Common Tx Power < 0 dBm
#endif

#define msg_time_ms 5            // Time needed for one message
#define msg_cnt 5                // Messages count used to transmit
#define backoff_time_timessync_max_ms 1000 // Calculate with probability of collisions


static RADIO_PACKET Radio_Packet_TX, Radio_Packet_RX;

typedef struct
{
  uint64_t LastTxTimestamp;
  uint64_t NextState_TS_us;
  uint32_t MAC_Address_LSB;
  uint8_t seq;
} __attribute__((packed)) TimesyncPkt;

TimesyncPkt Tsync_pkt_TX, Tsync_pkt_RX, Tsync_pkt_RX_2;

/** Takes ~1500ms if not relayed / Takes ~250ms if relayed*/
void bm_timesync_msg_publish(bool relaying)
{
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(TimesyncAddress);
  bm_radio_setTxP(CommonTxPower);
  synctimer_TimeStampCapture_clear();
  synctimer_TimeStampCapture_enable();
  Radio_Packet_TX.length = sizeof(Tsync_pkt_TX);
  Radio_Packet_TX.PDU = (uint8_t *)&Tsync_pkt_TX;
  Tsync_pkt_TX.MAC_Address_LSB = LSB_MAC_Address;
  for (int ch_rx = CommonStartCH; ch_rx <= CommonEndCH; ch_rx++)
  {
    for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
    {
      bm_radio_setCH(ch);
      for (int i = 0; i < msg_cnt; i++)
      {
        Tsync_pkt_TX.LastTxTimestamp = synctimer_getSyncedTxTimeStamp();
        Tsync_pkt_TX.NextState_TS_us = synctimer_getSyncTimeCompareIntTS();
        bm_radio_send(Radio_Packet_TX);
        Tsync_pkt_TX.seq++;
      }
    }
  }
  if(!relaying){
    bm_sleep(backoff_time_timessync_max_ms + msg_time_ms * msg_cnt * CommonCHCnt * CommonCHCnt); // Sleep till all relays neerby should be done
  }
}

/** Takes ~250ms to sync + ~250ms for realying */
bool bm_timesync_msg_subscribe(void (*transition_cb)())
{
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(TimesyncAddress);
  bm_radio_setTxP(CommonTxPower);
  bm_state_synced = false;
  synctimer_TimeStampCapture_clear();
  synctimer_TimeStampCapture_enable();
  for (int ch = CommonStartCH; ch <= CommonEndCH; ch++)
  {
    bm_radio_setCH(ch);
    if (bm_radio_receive(&Radio_Packet_RX, msg_time_ms * msg_cnt * CommonCHCnt))
    {
      synctimer_TimeStampCapture_disable();
      Tsync_pkt_RX = *(TimesyncPkt *)Radio_Packet_RX.PDU;      // Bring the sheep to a dry place
      if (bm_radio_receive(&Radio_Packet_RX, msg_time_ms * 2)) // The Next Timesync Packet should arrive right after the first
      {
        Tsync_pkt_RX_2 = *(TimesyncPkt *)Radio_Packet_RX.PDU; // Bring the sheep to a dry place
        if ((Tsync_pkt_RX.MAC_Address_LSB == Tsync_pkt_RX_2.MAC_Address_LSB) && (Tsync_pkt_RX.seq == (Tsync_pkt_RX_2.seq - 1)))
        {
          if (Tsync_pkt_RX.MAC_Address_LSB == 0xE4337238 || true) // For Debug (1)
          {
            synctimer_setSync(Tsync_pkt_RX_2.LastTxTimestamp);
            bm_cli_log("Synced Time: %u\n", (uint32_t)synctimer_getSyncTime());
            if (Tsync_pkt_RX_2.NextState_TS_us > synctimer_getSyncTime() - 5 * 1000)
            { // Add a minimal gap time of 5ms
              bm_state_synced = true;
              synctimer_setSyncTimeCompareInt(Tsync_pkt_RX_2.NextState_TS_us, transition_cb);
              bm_cli_log("Synced with Time Master: %x\n", Tsync_pkt_RX_2.MAC_Address_LSB);
              if (Tsync_pkt_RX_2.NextState_TS_us > synctimer_getSyncTime() - 250 * 1000 - bm_rand_32 % backoff_time_timessync_max_ms * 1000){ // Relay if enough time is left ~250ms + random backoff time
                bm_timesync_msg_publish(true);
              }
              return true;
            }
            else
            {
              bm_cli_log("TS of next state is in the past (%u < %u)\n", (uint32_t)Tsync_pkt_RX_2.NextState_TS_us, (uint32_t)synctimer_getSyncTime() - 5 * 1000);
            }
          }
        }
      }
      synctimer_TimeStampCapture_clear();
      synctimer_TimeStampCapture_enable();
    }
  }
  return false;
}


/*

#define CHslice 12 // Dividing for Timeslot for each CHannel (must be a Divisor of the Number of Channels)

typedef struct
{
  uint64_t LastTxTimestamp;
  uint64_t ST_INIT_MESH_STACK_TS;
  uint32_t MAC_Address_LSB;
  uint8_t seq;
} __attribute__((packed)) TimesyncPkt;

static RADIO_PACKET Radio_Packet_TX, Radio_Packet_RX;
TimesyncPkt Tsync_pkt_TX, Tsync_pkt_RX, Tsync_pkt_RX_2;

void bm_timesync_Publish(uint32_t timeout_ms, uint64_t ST_INIT_MESH_STACK_TS, bool Just_Once) {
  uint64_t start_time = synctimer_getSyncTime(); // Get the current Timestamp
  uint8_t ch = CommonStartCH;                    // init Channel
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(TimesyncAddress);
  bm_radio_setTxP(CommonTxPower);
  synctimer_TimeStampCapture_clear();
  synctimer_TimeStampCapture_enable();
  Radio_Packet_TX.length = sizeof(Tsync_pkt_TX);
  Radio_Packet_TX.PDU = (uint8_t *)&Tsync_pkt_TX;
  Tsync_pkt_TX.MAC_Address_LSB = LSB_MAC_Address;
  Tsync_pkt_TX.seq = 0;
  uint8_t CH_slicer = CHslice;
  uint8_t Just_Once_cnt = 0;
  while ((start_time + timeout_ms * 1000) > synctimer_getSyncTime() || Just_Once) { // Do while timeout not exceeded
    bm_radio_setCH(ch);
    // Timeslot for each Channel should grant at least 2 Subsequent Packets to be Sent...
    while ((start_time + ((timeout_ms / CH_slicer) * 1000)) > synctimer_getSyncTime() || Just_Once) { // Dividing for Timeslot for each CHannel (must be a Divisor of the Number of Channels)
      Tsync_pkt_TX.LastTxTimestamp = synctimer_getSyncedTxTimeStamp();
      Tsync_pkt_TX.ST_INIT_MESH_STACK_TS = ST_INIT_MESH_STACK_TS;
      bm_radio_send(Radio_Packet_TX);
      Tsync_pkt_TX.seq++; // Increase Sequence Number
      if (Just_Once) {
        Just_Once_cnt++;
        if (Just_Once_cnt > 10) { // Should not take too long but at least to Transmissions are required
          Just_Once_cnt = 0;
          break;
        }
      }
    }
    CH_slicer--; // Decrease Channel Slicer
    if (CH_slicer == 0) {
      break;
    }
    ch++; // Increase channel
    if (ch > CommonEndCH) {
      ch = CommonStartCH;
      Just_Once = false;
    }
  }
  synctimer_TimeStampCapture_disable();
}

bool bm_timesync_Subscribe(uint32_t timeout_ms, void (*cc_cb)()) {
  uint64_t start_time = synctimer_getSyncTime(); // Get the current Timestamp
  uint8_t ch = CommonStartCH;                    // init Channel
  bm_radio_init();
  bm_radio_setMode(CommonMode);
  bm_radio_setAA(TimesyncAddress);
  bm_radio_setTxP(CommonTxPower);
  synctimer_TimeStampCapture_clear();
  synctimer_TimeStampCapture_enable();
  bm_state_synced = false;
  uint8_t CH_slicer = CHslice;  
  uint32_t time_left;  
  while (((start_time + timeout_ms * 1000) > synctimer_getSyncTime()) && !bm_state_synced) { // Do while timeout not exceeded or not synced
    bm_radio_setCH(ch);
    synctimer_TimeStampCapture_clear();
    synctimer_TimeStampCapture_enable();    
    time_left = ((start_time + timeout_ms * 1000) - synctimer_getSyncTime())/1000;
    if (bm_radio_receive(&Radio_Packet_RX, time_left / CH_slicer) && ((start_time + timeout_ms * 1000) > synctimer_getSyncTime())) { // Dividing for Timeslot for each CHannel (must be a Divisor of the Number of Channels)
      synctimer_TimeStampCapture_disable();
      Tsync_pkt_RX = *(TimesyncPkt *)Radio_Packet_RX.PDU; // Bring the sheep to a dry place
      // Keep on Receiving for the last Tx Timestamp
      time_left = ((start_time + timeout_ms * 1000) - synctimer_getSyncTime())/1000;
      if (bm_radio_receive(&Radio_Packet_RX, time_left / CH_slicer) && ((start_time + timeout_ms * 1000) > synctimer_getSyncTime())) { // Dividing for Timeslot for each CHannel (must be a Divisor of the Number of Channels)
        Tsync_pkt_RX_2 = *(TimesyncPkt *)Radio_Packet_RX.PDU;                                                                           // Bring the sheep to a dry place
        if ((Tsync_pkt_RX.MAC_Address_LSB == Tsync_pkt_RX_2.MAC_Address_LSB) && (Tsync_pkt_RX.seq == (Tsync_pkt_RX_2.seq - 1))) {
          if (Tsync_pkt_RX.MAC_Address_LSB == 0xE4337238 || true) // For Debug (1)
          //if (Tsync_pkt_RX.MAC_Address_LSB == 0x8ec92859 || false) // For Debug (2)
          {
            synctimer_setSync(Tsync_pkt_RX_2.LastTxTimestamp);
            bm_cli_log("Synced Time: %u\n", (uint32_t)synctimer_getSyncTime());
            uint64_t Next_State_TS = Tsync_pkt_RX_2.ST_INIT_MESH_STACK_TS;
            if (Next_State_TS > synctimer_getSyncTime() - 5 * 1000) { // Add a minimal gap time of 5ms
              bm_state_synced = true;
              synctimer_setSyncTimeCompareInt(Tsync_pkt_RX_2.ST_INIT_MESH_STACK_TS, cc_cb);
              bm_cli_log("Synced with Time Master: %x\n", Tsync_pkt_RX_2.MAC_Address_LSB);
              return true;
            } else {
              bm_cli_log("TS of next state is in the past (%u < %u)\n", (uint32_t)Next_State_TS, (uint32_t)synctimer_getSyncTime()- 5 * 1000);
            }
          }
        }
      }
    };
    CH_slicer--; // Decrease Channel Slicer
    if (CH_slicer == 0) {
      break;
    }
    // Increase channel
    ch++;
    if (ch > CommonEndCH) {
      ch = CommonStartCH;
    }
  }
  synctimer_TimeStampCapture_disable();
  return bm_state_synced;
}

*/