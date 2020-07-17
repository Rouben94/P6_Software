/* =================== README =======================*/
/* The Timesync requires a HW-Timer Instance with at least 5 CC Registers (eg. TIMER3-5 on the NRF52840 or TIMER1-2 on the nRF5340_NETCORE)*/
/* Please enable the Timer in Zephyr prj.conf file (CONFIG_NRFX_TIMER2=y) */

#include <stdlib.h>
#include <string.h>
#include <zephyr.h>

#include <nrfx_timer.h>
#include <hal/nrf_timer.h>
#include <hal/nrf_radio.h>

#include "bm_timesync.h"
#include "bm_cli.h"
#include "bm_config.h"

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

/* NRF SDK WAY 
void TIMER4_IRQHandler(void){
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
}
*/

/* Timer init */
extern void synctimer_init()
{
    wakeup_thread_tid = k_current_get();
    // Takes 4294s / 71min to expire
    nrf_timer_bit_width_set(synctimer, NRF_TIMER_BIT_WIDTH_32);
    nrf_timer_frequency_set(synctimer, NRF_TIMER_FREQ_1MHz);
    nrf_timer_mode_set(synctimer, NRF_TIMER_MODE_TIMER);
    IRQ_DIRECT_CONNECT(TIMER4_IRQn, 6, bm_timer_handler, 0); // Connect Timer ISR Zephyr WAY
    irq_enable(TIMER4_IRQn);                                 // Enable Timer ISR Zephyr WAY
                                                             // NVIC_EnableIRQ(TIMER4_IRQn);                                 // Enable Timer ISR NRF SDK WAY
    nrf_timer_task_trigger(synctimer, NRF_TIMER_TASK_CLEAR);
    nrf_timer_cc_set(synctimer, NRF_TIMER_CC_CHANNEL1, 0);
    nrf_timer_cc_set(synctimer, NRF_TIMER_CC_CHANNEL2, 0);
    nrf_timer_cc_set(synctimer, NRF_TIMER_CC_CHANNEL5, 0xFFFFFFFF); // For Overflow Detection
    synctimer->INTENSET |= (uint32_t)NRF_TIMER_INT_COMPARE5_MASK;   // Enable Compare Event 5 Interrupt
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
    return ((uint64_t)OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL1)) + Timestamp_Diff;
}

/* Get previous Tx sync timestamp with respect to Timediff to Master */
extern uint64_t synctimer_getSyncedTxTimeStamp()
{
    if (Timestamp_Slave > Timestamp_Master)
    {
        //Timestamp_Diff = Timestamp_Slave - Timestamp_Master;
        return ((uint64_t)OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL1)) - Timestamp_Diff;
    }
    else if ((Timestamp_Master > Timestamp_Slave))
    {
        //Timestamp_Diff = Timestamp_Master - Timestamp_Slave;
        return ((uint64_t)OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL1)) + Timestamp_Diff;
    }
    else
    {
        //Timestamp_Diff = 0;
        return ((uint64_t)OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL1));
    }
}

/* Get previous Rx sync timestamp */
extern uint64_t synctimer_getRxTimeStamp()
{
    return ((uint64_t)OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL2));
}

/* Synchronise the timer offset with the received offset Timestamp */
extern void synctimer_setSync(uint64_t TxMasterTimeStamp)
{
    Timestamp_Slave = ((uint64_t)OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL2)) - RxChainDelay_us;
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
        return (((uint64_t)OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL0)) - Timestamp_Diff);
    }
    else if ((Timestamp_Master > Timestamp_Slave))
    {
        nrf_timer_task_trigger(synctimer, nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
        return (((uint64_t)OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL0)) + Timestamp_Diff);
    }
    else
    {
        nrf_timer_task_trigger(synctimer, nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
        return ((uint64_t)OverflowCNT << 32 | nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL0));
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
extern uint64_t synctimer_getSyncTimeCompareIntTS()
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
    return (uint64_t) msb_ts << 32 | lsb_ts;
}

/* Sets a Compare Interrupt which occurs after the specified time in us */
void synctimer_setCompareInt(uint32_t timeout_ms)
{
    synctimer->EVENTS_COMPARE[3] = false;
    nrf_timer_task_trigger(synctimer, nrf_timer_capture_task_get(NRF_TIMER_CC_CHANNEL0));
    synctimer->CC[3] = nrf_timer_cc_get(synctimer, NRF_TIMER_CC_CHANNEL0) + timeout_ms * 1000;
    synctimer->INTENSET |= (uint32_t)NRF_TIMER_INT_COMPARE3_MASK; // Enable Compare Event 3 Interrupt
}

/* Sleeps for the given Timeout in ms */
void bm_sleep(uint32_t timeout_ms)
{
    bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
    synctimer_setCompareInt(timeout_ms);
    while (!(bm_synctimer_timeout_compare_int))
    {
        //__SEV();__WFE();__WFE(); // Wait for Timer Interrupt nRF5SDK Way
        k_sleep(K_FOREVER); // Zephyr Way
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

/* ---------------------- RADIO AREA ------------------------ */

/** Maximum radio RX or TX payload. */
#define RADIO_MAX_PAYLOAD_LEN 256
/** IEEE 802.15.4 maximum payload length. */
#define IEEE_MAX_PAYLOAD_LEN 127

typedef struct
{
    uint8_t length;
    u8_t *PDU;    // Pointer to PDU
    u8_t Rx_RSSI; // Received RSSI of Packet
} RADIO_PACKET;

RADIO_PACKET Radio_Packet_TX, Radio_Packet_RX;

typedef struct
{
    uint8_t length;
    u8_t PDU[RADIO_MAX_PAYLOAD_LEN - 1];      // Max Size of PDU
} __attribute__((packed)) PACKET_PDU_ALIGNED; // Attribute informing compiler to pack all members to 8bits or 1byte

static PACKET_PDU_ALIGNED tx_pkt_aligned;

typedef struct /* For IEEE The address has to be transmitted explicit */
{
    uint8_t length;
    u32_t address;
    u8_t PDU[IEEE_MAX_PAYLOAD_LEN - 4 - 1];        // Max Size of PDU
} __attribute__((packed)) PACKET_PDU_ALIGNED_IEEE; // Attribute informing compiler to pack all members to 8bits or 1byte

static PACKET_PDU_ALIGNED_IEEE tx_pkt_aligned_IEEE;

static int const BLE_CH_freq[40] = {2404, 2406, 2408, 2410, 2412, 2414, 2416, 2418, 2420, 2422, 2424, 2428, 2430, 2432, 2434, 2436, 2438, 2440, 2442, 2444, 2446, 2448, 2450, 2452, 2454, 2456, 2458, 2460, 2462, 2464, 2466, 2468, 2470, 2472, 2474, 2476, 2478, 2402, 2426, 2480};
static int const IEEE802_15_4_CH_freq[16] = {2405, 2410, 2415, 2420, 2425, 2430, 2435, 2440, 2445, 2450, 2455, 2460, 2465, 2470, 2475, 2480}; // List of IEEE802.15.4 Channels

/* Define Packet Buffers */
static u8_t *tx_buf;                                // Pointer to the used Tx Buffer Payload
static uint8_t tx_buf_len;                          // Length of the Tx Buffer
static u8_t rx_buf_ieee[IEEE_MAX_PAYLOAD_LEN] = {}; // Rx Buffer for IEEE operation
static u8_t rx_buf[RADIO_MAX_PAYLOAD_LEN] = {};     // Rx Buffer for BLE operation
static u8_t *rx_buf_ptr;
static u32_t address; // Storing the Access Address

void bm_radio_clock_init()
{
    NRF_CLOCK->TASKS_HFCLKSTART = 1;    //Start high frequency clock
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0; //Clear event
}

void bm_radio_disable(void)
{
    nrf_radio_shorts_set(NRF_RADIO, 0);
    nrf_radio_int_disable(NRF_RADIO, ~0);
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_DISABLED);

    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_DISABLE);
    while (!nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_DISABLED))
    {
        /* Do nothing */
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_DISABLED);
}

void bm_radio_setMode(nrf_radio_mode_t m)
{
    // Set the desired Radio Mode
    nrf_radio_mode_set(NRF_RADIO, m);
    // Enable Fast Ramp Up (no TIFS) and set Tx Default mode to Center
    nrf_radio_modecnf0_set(NRF_RADIO, true, RADIO_MODECNF0_DTX_Center);
    // CRC16-CCITT Conform
    nrf_radio_crc_configure(NRF_RADIO, RADIO_CRCCNF_LEN_Three, NRF_RADIO_CRC_ADDR_SKIP, 0x00065B);
    nrf_radio_crcinit_set(NRF_RADIO, 0x555555);
    // Packet configuration
    nrf_radio_packet_conf_t packet_conf;
    memset(&packet_conf, 0, sizeof(packet_conf));
    packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_8BIT; // 8-bit preamble
    packet_conf.lflen = 8;                             // lengthfield size = 8 bits
    packet_conf.s0len = 0;                             // S0 size = 0 bytes
    packet_conf.s1len = 0;                             // S1 size = 0 bits
    packet_conf.maxlen = 255;                          // max 255-byte payload
    packet_conf.statlen = 0;                           // 0-byte static length
    packet_conf.balen = 3;                             // 3-byte base address length (4-byte full address length)
    packet_conf.big_endian = false;                    // Bit 24: 1 Small endian
    packet_conf.whiteen = true;                        // Bit 25: 1 Whitening enabled
    packet_conf.crcinc = 0;                            // Indicates if LENGTH field contains CRC or not
    switch (m)
    {
    case NRF_RADIO_MODE_BLE_1MBIT:
        // Nothing to be done :)
        break;
    case NRF_RADIO_MODE_BLE_2MBIT:
        packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_16BIT; // 16-bit preamble
        break;
    case NRF_RADIO_MODE_BLE_LR500KBIT:
    case NRF_RADIO_MODE_BLE_LR125KBIT:
        packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_LONG_RANGE; // 10-bit preamble
        packet_conf.cilen = 2;                                   // Length of code indicator (Bits) - long range
        packet_conf.termlen = 3;                                 // Length of TERM field (Bits) in Long Range operation
        packet_conf.maxlen = IEEE_MAX_PAYLOAD_LEN;               // max byte payload
        // Enable Fast Ramp Up (no TIFS) and set Tx Defalut mode to Center
        nrf_radio_modecnf0_set(NRF_RADIO, true, RADIO_MODECNF0_DTX_Center);
        // CRC24-Bit
        nrf_radio_crc_configure(NRF_RADIO, RADIO_CRCCNF_LEN_Three, NRF_RADIO_CRC_ADDR_SKIP, 0);
        break;
    case NRF_RADIO_MODE_IEEE802154_250KBIT:
        packet_conf.plen = NRF_RADIO_PREAMBLE_LENGTH_32BIT_ZERO; // 32-bit preamble
        packet_conf.balen = 0;                                   // --> Address Matching is not suppported in IEEE802.15.4 mode
        packet_conf.maxlen = IEEE_MAX_PAYLOAD_LEN;               // max 127-byte payload
        packet_conf.lflen = 8;                                   // lengthfield size = 8 bits
        packet_conf.crcinc = 1;                                  // Indicates if LENGTH field contains CRC or not
        // Enable Fast Ramp Up (no TIFS) and set Tx Defalut mode to Center
        nrf_radio_modecnf0_set(NRF_RADIO, true, RADIO_MODECNF0_DTX_Center);
        // CRC 16-bit ITU-T conform
        nrf_radio_crc_configure(NRF_RADIO, RADIO_CRCCNF_LEN_Two, NRF_RADIO_CRC_ADDR_IEEE802154, 0x11021);
        nrf_radio_crcinit_set(NRF_RADIO, 0x5555);
    default:
        break;
    }
    nrf_radio_packet_configure(NRF_RADIO, &packet_conf);
}

void bm_radio_setCH(u8_t CH)
{
    // Accept only numbers within 0-39 for BLE Channels
    if ((39 >= CH && CH >= 0) && nrf_radio_mode_get(NRF_RADIO) != NRF_RADIO_MODE_IEEE802154_250KBIT)
    {
        nrf_radio_frequency_set(NRF_RADIO, BLE_CH_freq[CH]);
        nrf_radio_datawhiteiv_set(NRF_RADIO, CH);
        //bm_cli_log("Set Frequency to %d\n", BLE_CH_freq[CH]);
    }
    else if (26 >= CH && CH >= 11) // Accept only numbers within 11-26 for IEEE802.15.4 Channels
    {
        nrf_radio_frequency_set(NRF_RADIO, IEEE802_15_4_CH_freq[CH - 11]);
    }
}

void bm_radio_setTxP(nrf_radio_txpower_t TxP)
{
    nrf_radio_txpower_set(NRF_RADIO, TxP);
}

void bm_radio_setAA(u32_t aa)
{
    address = aa; // Store the Access Address
    /* Set the device address 0 to use when transmitting. */
    nrf_radio_txaddress_set(NRF_RADIO, 0);
    /* Enable the device address 0 to use to select which addresses to
	 	* receive
	 	*/
    nrf_radio_rxaddresses_set(NRF_RADIO, 1);
    /* Set the access address */
    nrf_radio_prefix0_set(NRF_RADIO, (aa >> 24) & RADIO_PREFIX0_AP0_Msk);
    nrf_radio_base0_set(NRF_RADIO, (aa << 8) & 0xFFFFFF00);
    //nrf_radio_sfd_set(NRF_RADIO, (aa >> 24) & RADIO_PREFIX0_AP0_Msk); // Set the SFD for the IEEE Radio Mode to the Prefix (first byte of Address) -> Dont do because of CCA Carrier Mode
}

bool bm_radio_crcok_int = false;
/* Zephyr Way */
/*
static void bm_radio_handler(){
    bm_cli_log("Interrupt\n");
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_CRCOK)){
        bm_radio_crcok_int = true;
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
    k_wakeup(wakeup_thread_tid);
}
*/
/*
ISR_DIRECT_DECLARE(bm_radio_handler)
{
    //bm_cli_log("Interrupt\n");
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_CRCOK)){
        bm_radio_crcok_int = true;
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
    k_wakeup(wakeup_thread_tid);
    ISR_DIRECT_PM();
    return 1;
}
*/

/* NRF SDK WAY 
void RADIO_IRQHandler(void){
    //bm_cli_log("Interrupt\n");
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_CRCOK)){
        bm_radio_crcok_int = true;
    }
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
}
*/

void bm_radio_init()
{
    // Enable the High Frequency clock on the processor. This is a pre-requisite for
    // the RADIO module. Without this clock, no communication is possible.
    wakeup_thread_tid = k_current_get();
    bm_radio_clock_init();
    nrf_radio_power_set(NRF_RADIO, 1);          // Power ON Radio
    bm_radio_disable();                         // Disable Radio
    bm_radio_setMode(NRF_RADIO_MODE_BLE_1MBIT); // Set Mode to BLE 1MBITS
    bm_radio_setAA(0x8E89BED6);                 // Default Advertisment Address BLE 0x8E89BED6
    bm_radio_setCH(11);                         // Default Advertisment Channel
    bm_radio_setTxP(NRF_RADIO_TXPOWER_0DBM);    // Set Tx Power to 0dbm
    //IRQ_DIRECT_CONNECT(RADIO_IRQn, 6, bm_radio_handler, 0); // Connect Radio ISR Zephyr WAY
    //irq_connect_dynamic(RADIO_IRQn, 6, bm_radio_handler, NULL, 0); // Connect Radio ISR Zephyr WAY
    //irq_enable(RADIO_IRQn);                                 // Enable Radio ISR Zephyr WAY
    // NVIC_EnableIRQ(RADIO_IRQn);                               // Enable Radio ISR NRF SDK WAY
}

typedef struct
{
    uint64_t LastTxTimestamp;
    uint64_t ST_INIT_MESH_STACK_TS;
    uint32_t MAC_Address_LSB;
    uint8_t seq;
} __attribute__((packed)) TimesyncPkt;

void bm_radio_send(RADIO_PACKET tx_pkt)
{
    bm_radio_disable(); // Disable the Radio
    /* Setup Paket */
    if (nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_IEEE802154_250KBIT)
    {
        tx_pkt_aligned_IEEE.length = tx_pkt.length + 2 + sizeof(address);                  // Because Length includes CRC Field
        tx_pkt_aligned_IEEE.address = address;                                             // Save  address because IEEE wont transmit it by itself
        memset(tx_pkt_aligned_IEEE.PDU, 0, sizeof(tx_pkt_aligned_IEEE.PDU));               // Initialize Data Structure
        memcpy(tx_pkt_aligned_IEEE.PDU, tx_pkt.PDU, tx_pkt.length);                        // Copy the MAC PDU to the RAM PDU
        tx_buf = (u8_t *)&tx_pkt_aligned_IEEE;                                             // Set the Tx Buffer Pointer
        tx_buf_len = tx_pkt.length + sizeof(address) + sizeof(tx_pkt_aligned_IEEE.length); // Save the Size of the tx_buf
    }
    else
    {
        tx_pkt_aligned.length = tx_pkt.length;
        memset(tx_pkt_aligned.PDU, 0, sizeof(tx_pkt_aligned.PDU));          // Initialize Data Structure
        memcpy(tx_pkt_aligned.PDU, tx_pkt.PDU, tx_pkt.length);              // Copy the MAC PDU to the RAM PDU
        tx_buf = (u8_t *)&tx_pkt_aligned;                                   // Set the Tx Buffer Pointer
        tx_buf_len = tx_pkt_aligned.length + sizeof(tx_pkt_aligned.length); // Save the Size of the tx_buf
    }
    nrf_radio_packetptr_set(NRF_RADIO, tx_buf);
    nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_PHYEND_DISABLE_MASK); // Start after Ready and Disable after END or PHY-End
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_DISABLED);
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_TXEN);
    while (!(nrf_radio_state_get(NRF_RADIO) == 0)) // Check for Disabled State
    {
        __NOP(); // Since the LLL Driver owns the Interrupt we have to Poll
        //__WFE(); // Wait for Timer Interrupt nRF5SDK Way
        // k_sleep(K_FOREVER); // Zephyr Way
    }
    bm_radio_disable();
}

bool bm_radio_receive(RADIO_PACKET *rx_pkt, uint32_t timeout_ms)
{
    bm_radio_disable();
    /* Initialize Rx Buffer */
    if (nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_IEEE802154_250KBIT)
    {
        memset(rx_buf_ieee, 0, sizeof(rx_buf_ieee)); // Erase old Rx buffer content
        rx_buf_ptr = rx_buf_ieee;
    }
    else
    {
        memset(rx_buf, 0, sizeof(rx_buf)); // Erase old Rx buffer content
        rx_buf_ptr = rx_buf;
    }
    nrf_radio_packetptr_set(NRF_RADIO, rx_buf_ptr);
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK); // Clear CRCOK Event                                                                                                                                         // Set Interrupt Thread to wakeup
    nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_ADDRESS_RSSISTART_MASK | NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_PHYEND_DISABLE_MASK); // Start after Ready and Start after END -> wait for CRCOK Event otherwise keep on receiving
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RXEN);
    bm_synctimer_timeout_compare_int = false;
    bm_radio_crcok_int = false; // Reset Interrupt Flags
    synctimer_setCompareInt(timeout_ms);
    while (!(bm_synctimer_timeout_compare_int))
    {
        //__SEV();__WFE();__WFE(); // Wait for Timer Interrupt nRF5SDK Way
        __NOP(); // Since the LLL Driver owns the Interrupt we have to Poll
        if (nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_CRCOK) && nrf_radio_state_get(NRF_RADIO) == NRF_RADIO_STATE_DISABLED)
        {
            rx_pkt->PDU = ((PACKET_PDU_ALIGNED *)rx_buf_ptr)->PDU;
            rx_pkt->length = ((PACKET_PDU_ALIGNED *)rx_buf_ptr)->length;
            bm_radio_disable();
            nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RSSISTOP);
            nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);
            bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
            return true;
        }
    }
    bm_synctimer_timeout_compare_int = false; // Reset Interrupt Flags
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RSSISTOP);
    bm_radio_disable();
    return false;
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

#define CHslice 12 // Dividing for Timeslot for each CHannel (must be a Divisor of the Number of Channels)

TimesyncPkt Tsync_pkt_TX, Tsync_pkt_RX, Tsync_pkt_RX_2;

void bm_timesync_Publish(uint32_t timeout_ms, uint64_t ST_INIT_MESH_STACK_TS)
{
    uint64_t start_time = synctimer_getSyncTime(); // Get the current Timestamp
    uint8_t ch = CommonStartCH;                    // init Channel
    bm_radio_init();
    bm_radio_setMode(CommonMode);
    bm_radio_setAA(TimesyncAddress);
    bm_radio_setTxP(CommonTxPower);
    synctimer_TimeStampCapture_clear();
    synctimer_TimeStampCapture_enable();
    Radio_Packet_TX.length = sizeof(Tsync_pkt_TX);
    Radio_Packet_TX.PDU = (u8_t *)&Tsync_pkt_TX;
    Tsync_pkt_TX.MAC_Address_LSB = LSB_MAC_Address;
    Tsync_pkt_TX.seq = 0;
    uint8_t CH_slicer = CHslice;
    while ((start_time + timeout_ms * 1000) > synctimer_getSyncTime())
    { // Do while timeout not exceeded
        bm_radio_setCH(ch);
        // Timeslot for each Channel should grant at least 2 Subsequent Packets to be Sent...
        while ((start_time + ((timeout_ms / CH_slicer) * 1000)) > synctimer_getSyncTime())
        { // Dividing for Timeslot for each CHannel (must be a Divisor of the Number of Channels)
            Tsync_pkt_TX.LastTxTimestamp = synctimer_getSyncedTxTimeStamp();
            Tsync_pkt_TX.ST_INIT_MESH_STACK_TS = ST_INIT_MESH_STACK_TS;
            bm_radio_send(Radio_Packet_TX);
            Tsync_pkt_TX.seq++; // Increase Sequence Number
        }
        CH_slicer--; // Decrease Channel Slicer
        if (CH_slicer == 0)
        {
            break;
        }
        ch++; // Increase channel
        if (ch > CommonEndCH)
        {
            ch = CommonStartCH;
        }
    }
    synctimer_TimeStampCapture_disable();
}

bool bm_timesync_Subscribe(uint32_t timeout_ms, void (*cc_cb)())
{
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
    while (((start_time + timeout_ms * 1000) > synctimer_getSyncTime()) && !bm_state_synced)
    { // Do while timeout not exceeded or not synced
        bm_radio_setCH(ch);
        synctimer_TimeStampCapture_clear();
        synctimer_TimeStampCapture_enable();
        if (bm_radio_receive(&Radio_Packet_RX, timeout_ms / CH_slicer) && ((start_time + timeout_ms * 1000) > synctimer_getSyncTime()))
        { // Dividing for Timeslot for each CHannel (must be a Divisor of the Number of Channels)
            synctimer_TimeStampCapture_disable();
            Tsync_pkt_RX = *(TimesyncPkt *)Radio_Packet_RX.PDU; // Bring the sheep to a dry place
            // Keep on Receiving for the last Tx Timestamp
            if (bm_radio_receive(&Radio_Packet_RX, timeout_ms / CH_slicer) && ((start_time + timeout_ms * 1000) > synctimer_getSyncTime()))
            {                                                         // Dividing for Timeslot for each CHannel (must be a Divisor of the Number of Channels)
                Tsync_pkt_RX_2 = *(TimesyncPkt *)Radio_Packet_RX.PDU; // Bring the sheep to a dry place
                if ((Tsync_pkt_RX.MAC_Address_LSB == Tsync_pkt_RX_2.MAC_Address_LSB) && (Tsync_pkt_RX.seq == (Tsync_pkt_RX_2.seq - 1)))
                {
                    if (Tsync_pkt_RX.MAC_Address_LSB == 0xE4337238 || false) // For Debug (1)
                    //if (Tsync_pkt_RX.MAC_Address_LSB == 0x8ec92859 || false) // For Debug (2)
                    {                        
                        synctimer_setSync(Tsync_pkt_RX_2.LastTxTimestamp);
                        bm_cli_log("Synced Time: %u\n",(uint32_t)synctimer_getSyncTime());
                        uint64_t Next_State_TS = Tsync_pkt_RX_2.ST_INIT_MESH_STACK_TS;
                        if (Next_State_TS > synctimer_getSyncTime()){
                            bm_state_synced = true; 
                            synctimer_setSyncTimeCompareInt(Tsync_pkt_RX_2.ST_INIT_MESH_STACK_TS,cc_cb);                           
                            bm_cli_log("Synced with Time Master: %x\n", Tsync_pkt_RX_2.MAC_Address_LSB);
                            return true;
                        } else {
                            bm_cli_log("TS of next state is in the past (%u < %u)\n",(uint32_t)Next_State_TS,(uint32_t)synctimer_getSyncTime());
                        }                     
                    }
                }
            }
        };
        CH_slicer--; // Decrease Channel Slicer
        if (CH_slicer == 0)
        {
            break;
        }
        // Increase channel
        ch++;
        if (ch > CommonEndCH)
        {
            ch = CommonStartCH;
        }
    }
    synctimer_TimeStampCapture_disable();
    return bm_state_synced;
}