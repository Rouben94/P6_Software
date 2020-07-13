#include "Simple_nrf_radio.h"

/**
	 * Init the Radio
	 *
	 */
Simple_nrf_radio::Simple_nrf_radio()
{
    // Enable the High Frequency clock on the processor. This is a pre-requisite for
    // the RADIO module. Without this clock, no communication is possible.
    clock_init();
    nrf_radio_power_set(NRF_RADIO, 1);                            // Power ON Radio
    radio_disable();                                              // Disable Radio
    setMode(NRF_RADIO_MODE_BLE_1MBIT);                            // Set Mode to BLE 1MBITS
    setAA(0x8E89BED6);                                            // Default Advertisment Address BLE 0x8E89BED6
    setCH(11);                                                    // Default Advertisment Channel
    setTxP(NRF_RADIO_TXPOWER_0DBM);                               // Set Tx Power to 0dbm
    rhctx.thread_id = &ISR_Thread_ID;                             // store address of ISR_Thread_ID in pointer variable
    irq_connect_dynamic(RADIO_IRQn, 7, radio_handler, &rhctx, 0); // Connect Radio ISR
    irq_enable(RADIO_IRQn);                                       // Enable Radio ISR
}
/**
	 * Radio ISR caused by radio interrupts
	 *
	 * @param context Data Container for the ISR.
	 */
void Simple_nrf_radio::radio_handler(void *context)
{
    Radio_Handler_Context *c_rhctx = (Radio_Handler_Context *)context;
    //printk("Interrupt\n");
    /* Check Address in payload of IEEE802.15.4 */
    if ((nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_IEEE802154_250KBIT))
    {
        u8_t *payload = ((u8_t *)nrf_radio_packetptr_get(NRF_RADIO));
        if (payload[4] == (nrf_radio_prefix0_get(NRF_RADIO) & 0x000000FF) and payload[3] == (nrf_radio_base0_get(NRF_RADIO) >> 24) and payload[2] == ((nrf_radio_base0_get(NRF_RADIO) & 0x00FF0000) >> 16) and payload[1] == ((nrf_radio_base0_get(NRF_RADIO) & 0x0000FF00) >> 8))
        {
            //printk("IEEE Address match\n");
            if (nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_CRCOK) && c_rhctx->rx_stat.act)
            {
                c_rhctx->rx_stat.CRCOKcnt++;
            }
            else if (nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR) && c_rhctx->rx_stat.act)
            {
                c_rhctx->rx_stat.CRCErrcnt++;
            }
        }
        else
        {
            //printk("Wrong Address Received\n");
        }
    }
    else
    {
        if (nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_CRCOK) && c_rhctx->rx_stat.act)
        {
            c_rhctx->rx_stat.CRCOKcnt++;
        }
        else if (nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR) && c_rhctx->rx_stat.act) 
        {
            c_rhctx->rx_stat.CRCErrcnt++;
        }
    }
    c_rhctx->rx_stat.RSSI_Sum_Avg += nrf_radio_rssi_sample_get(NRF_RADIO); // RSSI should be done 
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RSSISTOP);
    if ((nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_END) || nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_PHYEND)) && c_rhctx->tx_stat.act){
        c_rhctx->tx_stat.Pktcnt++;
    }
    /*
    if (nrf_radio_event_check(NRF_RADIO,NRF_RADIO_EVENT_FRAMESTART) && c_rhctx->rx_stat.act){
        printk("hoi\n");
    }
    */
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);      // Clear ISR Event END
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_PHYEND);   // Clear ISR Event PHYEND
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);    // Clear ISR Event CRCOK
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear ISR Event CRCError
    //nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_FRAMESTART); // Clear ISR Event FRAMESTART
    if (c_rhctx->rx_stat.act || c_rhctx->tx_stat.act){
        return; //Keep on receiving till timeout
    }
    k_wakeup((k_tid_t) * (c_rhctx->thread_id));                 // Wake up the sleeping Thread waiting for the ISR
}
/**
	 * Enable HFCLK 
	 *
	 */
void Simple_nrf_radio::clock_init(void)
{
    NRF_CLOCK->TASKS_HFCLKSTART = 1;    //Start high frequency clock
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0; //Clear event
}
/**
	 * Meassure RSSI Value
	 *
	 * @param cycles Number of samples to average
	 */
int Simple_nrf_radio::RSSI(u8_t cycles)
{
    if (cycles < 1)
    {
        cycles = 1;
    };
    int RSSIres = 0;
    nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK); // Start after Ready
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RXEN);
    while (nrf_radio_state_get(NRF_RADIO) != NRF_RADIO_STATE_RX)
        ; // Wait for RX State
    for (u8_t i = 1; i <= cycles; i++)
    {
        nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RSSISTART); // Start RSSI Measurment
        while (nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_RSSIEND) != true)
            ;                                                       // Wait for RSSI end
        nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RSSISTOP); // Stop RSSI Measurment
        RSSIres = RSSIres + nrf_radio_rssi_sample_get(NRF_RADIO);
        nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_RSSIEND); // Clear Event
    }
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_DISABLE);
    while (nrf_radio_state_get(NRF_RADIO) != NRF_RADIO_STATE_DISABLED)
        ; // Wait for Disable State
    radio_disable();
    return RSSIres / cycles;
    ; // Average the RSSI over cycles
}
/**
	 * Send a Payload
	 *
	 * @param tx_pkt Radio Paket to Send
	 */
void Simple_nrf_radio::Send(RADIO_PACKET tx_pkt)
{
    radio_disable(); // Disable the Radio
    /* Setup Paket */
    if (nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_IEEE802154_250KBIT)
    {
        //memset((u8_t *)&tx_pkt_aligned_IEEE, 0, sizeof((u8_t *)&tx_pkt_aligned_IEEE));     // Initialize Data Structure
        tx_pkt_aligned_IEEE.length = tx_pkt.length + 2 + sizeof(address);                  // Because Length includes CRC Field
        tx_pkt_aligned_IEEE.address = address;                                             // Save  address because IEEE wont transmit it by itself
        memset(tx_pkt_aligned_IEEE.PDU, 0, sizeof(tx_pkt_aligned_IEEE.PDU));               // Initialize Data Structure
        memcpy(tx_pkt_aligned_IEEE.PDU, tx_pkt.PDU, tx_pkt.length);                        // Copy the MAC PDU to the RAM PDU
        tx_buf = (u8_t *)&tx_pkt_aligned_IEEE;                                             // Set the Tx Buffer Pointer
        tx_buf_len = tx_pkt.length + sizeof(address) + sizeof(tx_pkt_aligned_IEEE.length); // Save the Size of the tx_buf
    }
    else
    {
        //memset((u8_t *)&tx_pkt_aligned, 0, sizeof((u8_t *)&tx_pkt_aligned)); // Initialize Data Structure
        tx_pkt_aligned.length = tx_pkt.length;
        memset(tx_pkt_aligned.PDU, 0, sizeof(tx_pkt_aligned.PDU));          // Initialize Data Structure
        memcpy(tx_pkt_aligned.PDU, tx_pkt.PDU, tx_pkt.length);              // Copy the MAC PDU to the RAM PDU
        tx_buf = (u8_t *)&tx_pkt_aligned;                                   // Set the Tx Buffer Pointer
        tx_buf_len = tx_pkt_aligned.length + sizeof(tx_pkt_aligned.length); // Save the Size of the tx_buf
    }
    nrf_radio_packetptr_set(NRF_RADIO, tx_buf);
    //nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);                                                                                         // Clear END Event
    //nrf_radio_int_enable(NRF_RADIO, NRF_RADIO_INT_END_MASK);                                                                                       // Enable END Event interrupt
    //ISR_Thread_ID = k_current_get();                                                                                                               // Set Interrupt Thread to wakeup
    nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_PHYEND_DISABLE_MASK); // Start after Ready and Disable after END or PHY-End
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_TXEN);
    while (!nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_DISABLED)){
        k_sleep(K_MSEC(1));
    }
    //if (timeout.ticks < K_MSEC(1).ticks){timeout = K_MSEC(1);} //Prevent from Zero Timeout...
    //k_sleep(timeout); // Wait for interrupt
    radio_disable();
}
/**
	 * Burst send out the same packet till timeout
	 *
	 * @param tx_pkt Radio Paket to Send
	 * @param timeout Blocktimeout in ms for bursting
     * @return Number of Packeets sent
	 */
u16_t Simple_nrf_radio::BurstCntPkt(RADIO_PACKET tx_pkt, u8_t CCA_Mode, k_timeout_t timeout)
{
    radio_disable(); // Disable the Radio
    /* Setup Paket */
    if (nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_IEEE802154_250KBIT)
    {
        //memset((u8_t *)&tx_pkt_aligned_IEEE, 0, sizeof((u8_t *)&tx_pkt_aligned_IEEE));     // Initialize Data Structure
        tx_pkt_aligned_IEEE.length = tx_pkt.length + 2 + sizeof(address);                  // Because Length includes CRC Field
        tx_pkt_aligned_IEEE.address = address;                                             // Save  address because IEEE wont transmit it by itself
        memset(tx_pkt_aligned_IEEE.PDU, 0, sizeof(tx_pkt_aligned_IEEE.PDU));               // Initialize Data Structure
        memcpy(tx_pkt_aligned_IEEE.PDU, tx_pkt.PDU, tx_pkt.length);                        // Copy the MAC PDU to the RAM PDU
        tx_buf = (u8_t *)&tx_pkt_aligned_IEEE;                                             // Set the Tx Buffer Pointer
        tx_buf_len = tx_pkt.length + sizeof(address) + sizeof(tx_pkt_aligned_IEEE.length); // Save the Size of the tx_buf
    }
    else
    {
        //memset((u8_t *)&tx_pkt_aligned, 0, sizeof((u8_t *)&tx_pkt_aligned)); // Initialize Data Structure
        tx_pkt_aligned.length = tx_pkt.length;
        memset(tx_pkt_aligned.PDU, 0, sizeof(tx_pkt_aligned.PDU));          // Initialize Data Structure
        memcpy(tx_pkt_aligned.PDU, tx_pkt.PDU, tx_pkt.length);              // Copy the MAC PDU to the RAM PDU
        tx_buf = (u8_t *)&tx_pkt_aligned;                                   // Set the Tx Buffer Pointer
        tx_buf_len = tx_pkt_aligned.length + sizeof(tx_pkt_aligned.length); // Save the Size of the tx_buf
    }
    nrf_radio_packetptr_set(NRF_RADIO, tx_buf);
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);                                                                                         // Clear END Event
    nrf_radio_int_enable(NRF_RADIO, NRF_RADIO_INT_END_MASK);                                                                                       // Enable END Event interrupt
    ISR_Thread_ID = k_current_get();                                                                                                               // Set Interrupt Thread to wakeup
    if (nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_BLE_LR125KBIT || nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_BLE_LR500KBIT){
        nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_PHYEND_DISABLE_MASK | NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_DISABLED_TXEN_MASK); // Slow things Down for LR Becasuse using regular short leads to Packet loss
    } else {
        nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_PHYEND_START_MASK | NRF_RADIO_SHORT_END_START_MASK); // Start after Ready and Disable after END or PHY-End
    }
    rhctx.tx_stat.Pktcnt = 0;
    rhctx.tx_stat.act = true;
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_TXEN);
    k_sleep(timeout); // Wait for interrupt
    nrf_radio_shorts_set(NRF_RADIO, 0);
    nrf_radio_int_disable(NRF_RADIO, ~0);
    k_sleep(K_MSEC(1)); // Wait for last transmission done
    rhctx.tx_stat.act = false;
    radio_disable();
    return rhctx.tx_stat.Pktcnt;
}
/**
	 * Receive a Payload
	 *
	 * @param payload Pointer where to store received payload
	 * @param timeout Waittimeout in ms for a packet to be received
     * 
	 * @return Zero of timeout occured or number of miliseconds till timeout occurs
	 */
s32_t Simple_nrf_radio::Receive(RADIO_PACKET *rx_pkt, k_timeout_t timeout)
{
    radio_disable();
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
    //memset(rx_buf_ptr, 0, sizeof(rx_buf_ptr)); // Erase old Rx buffer content
    nrf_radio_packetptr_set(NRF_RADIO, rx_buf_ptr);
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK);                                                                                                                            // Clear CRCOK Event
    nrf_radio_int_enable(NRF_RADIO, NRF_RADIO_INT_CRCOK_MASK);                                                                                                                          // Enable CRCOK Event interrupt
    ISR_Thread_ID = k_current_get();                                                                                                                                                    // Set Interrupt Thread to wakeup
    nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_ADDRESS_RSSISTART_MASK | NRF_RADIO_SHORT_PHYEND_START_MASK | NRF_RADIO_SHORT_END_START_MASK); // Start after Ready and Start after END -> wait for CRCOK Event otherwise keep on receiving
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RXEN);
    s32_t ret = k_sleep(timeout); // Wait for interrupt and check if it was a timeout
    if (ret > 0)
    {
        //printk("Received Sample RSSI: %d \n", nrf_radio_rssi_sample_get(NRF_RADIO));
        rx_pkt->Rx_RSSI = nrf_radio_rssi_sample_get(NRF_RADIO);
        /* Parse Paket */
        if (nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_IEEE802154_250KBIT)
        {
            PACKET_PDU_ALIGNED_IEEE *rx_pkt_pdu_ieee;                       // Create new Pointer
            rx_pkt_pdu_ieee = (PACKET_PDU_ALIGNED_IEEE *)rx_buf_ptr;        // Revert Received Payload
            rx_pkt->length = rx_pkt_pdu_ieee->length - 2 - sizeof(address); // Copy length field -2 because includes CRC Field
            //memcpy(rx_pkt.PDU, rx_pkt_pdu_ieee->PDU, rx_pkt_pdu_ieee->length); // Copy the PDU field
            rx_pkt->PDU = rx_pkt_pdu_ieee->PDU;
        }
        else
        {
            PACKET_PDU_ALIGNED *rx_pkt_pdu;                // Create new Pointer
            rx_pkt_pdu = (PACKET_PDU_ALIGNED *)rx_buf_ptr; // Revert Received Payload
            rx_pkt->length = rx_pkt_pdu->length;           // Copy length field
            //memcpy(rx_pkt.PDU, rx_pkt_pdu->PDU, rx_pkt_pdu->length); // Copy the PDU field
            rx_pkt->PDU = rx_pkt_pdu->PDU;
        }
    }
    else
    {
        //printk("Timeout nothing received \n");
    }
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RSSISTOP);
    radio_disable();
    return ret;
}
/**
	 * Receive Packets and only Log Data
	 *
	 * @param timeout Waittimeout in ms packet to be received and logged
     * 
	 * @return Struct of Log Data CRCOKcnt, CRCErrcnt, RSSIAvg
	 */
RxPktStatLog Simple_nrf_radio::ReceivePktStatLog(k_timeout_t timeout)
{
    radio_disable();
    nrf_radio_packetptr_set(NRF_RADIO, rx_buf);
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCOK); // Clear CRCOK Event
    nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_CRCERROR); // Clear CRCERROR Event
    nrf_radio_int_enable(NRF_RADIO, NRF_RADIO_INT_CRCOK_MASK | NRF_RADIO_INT_CRCERROR_MASK);                                                                                                                         // Enable CRCOK Event interrupt
    ISR_Thread_ID = k_current_get();   // Set Interrupt Thread to wakeup
    if (nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_BLE_LR125KBIT || nrf_radio_mode_get(NRF_RADIO) == NRF_RADIO_MODE_BLE_LR500KBIT){
        nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_ADDRESS_RSSISTART_MASK | NRF_RADIO_SHORT_PHYEND_DISABLE_MASK | NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_DISABLED_RXEN_MASK); // Slow things Down for LR Becasuse using regular short leads to Packet loss
    } else {
        nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK | NRF_RADIO_SHORT_ADDRESS_RSSISTART_MASK | NRF_RADIO_SHORT_PHYEND_START_MASK | NRF_RADIO_SHORT_END_START_MASK); // Start after Ready and Start after END -> wait for CRCOK Event otherwise keep on receiving
    }
    rhctx.rx_stat.CRCErrcnt = 0;
    rhctx.rx_stat.CRCOKcnt = 0;
    rhctx.rx_stat.RSSI_Sum_Avg = 174; //Thermal Noise
    rhctx.rx_stat.act = true;
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RXEN);
    k_sleep(timeout); // Wait for interrupt and check if it was a timeout
    radio_disable();
    rhctx.rx_stat.act = false;
    if ((rhctx.rx_stat.CRCOKcnt + rhctx.rx_stat.CRCErrcnt) != 0)
    {
        rhctx.rx_stat.RSSI_Sum_Avg = (rhctx.rx_stat.RSSI_Sum_Avg-174) / (rhctx.rx_stat.CRCOKcnt + rhctx.rx_stat.CRCErrcnt);
    }
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RSSISTOP);
    return rhctx.rx_stat;
}
/**
	 * Set the Radio Mode.
	 *
	 * @param m NRF Radio Mode to select (BLE or IEEE etc.)
	 */
void Simple_nrf_radio::setMode(nrf_radio_mode_t m)
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
    packet_conf.big_endian = false;                        // Bit 24: 1 Small endian
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
        packet_conf.maxlen = IEEE_MAX_PAYLOAD_LEN;                // max byte payload
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
/**
	 * Set the Radio Channel
	 *
	 * @param CH Radio Channel to set (for BLE from 0-39 and IEEE802.15.4 from 11-26)
	 */
void Simple_nrf_radio::setCH(u8_t CH)
{
    // Accept only numbers within 0-39 for BLE Channels
    if ((39 >= CH and CH >= 0) and nrf_radio_mode_get(NRF_RADIO) != NRF_RADIO_MODE_IEEE802154_250KBIT)
    {
        nrf_radio_frequency_set(NRF_RADIO, BLE_CH_freq[CH]);
        nrf_radio_datawhiteiv_set(NRF_RADIO, CH);
        //printk("Set Frequency to %d\n", BLE_CH_freq[CH]);
    }
    else if (26 >= CH and CH >= 11) // Accept only numbers within 11-26 for IEEE802.15.4 Channels
    {
        nrf_radio_frequency_set(NRF_RADIO, IEEE802_15_4_CH_freq[CH - 11]);
    }
}
/**
	 * Set the Tx Power of the Radio
	 *
	 * @param TxP TxPower to select
	 */
void Simple_nrf_radio::setTxP(nrf_radio_txpower_t TxP)
{
    nrf_radio_txpower_set(NRF_RADIO, TxP);
}
/**
	 * Set the AccessAddress of the Radio
	 *
	 * @param aa AccessAddress to send or receive from. 
	 */
void Simple_nrf_radio::setAA(u32_t aa)
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
/**
	 * Disable the Radio
	 *
	 */
void Simple_nrf_radio::radio_disable(void)
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
