/**
 * Copyright (c) 2012-2018 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef RADIO_TEST_H
#define RADIO_TEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define RADIO_MAX_PAYLOAD_LEN     256   /**< Maximum radio RX or TX payload. */
#define IEEE_MAX_PAYLOAD_LEN      127   /**< IEEE 802.15.4 maximum payload length. */
#define IEEE_MIN_CHANNEL          11    /**< IEEE 802.15.4 minimum channel. */
#define IEEE_MAX_CHANNEL          26    /**< IEEE 802.15.4 maximum channel. */

/**@brief Radio transmit and address pattern.
 */
typedef enum
{
    TRANSMIT_PATTERN_RANDOM,   /**< Random pattern generated by RNG. */
    TRANSMIT_PATTERN_11110000, /**< Pattern 11110000(F0). */
    TRANSMIT_PATTERN_11001100, /**< Pattern 11001100(CC). */
} transmit_pattern_t;


/**
 * @brief Function for turning on the TX carrier test mode.
 *
 * @param[in] tx_power Radio output power.
 * @param[in] mode     Radio mode.
 * @param[in] channel  Radio channel(frequency).
 */
void radio_unmodulated_tx_carrier(uint8_t tx_power, uint8_t mode, uint8_t channel);


/**
 * @brief Function for starting the modulated TX carrier by repeatedly sending a packet with a random address and
 * a random payload.
 *
 * @param[in] tx_power Radio output power.
 * @param[in] mode     Radio mode. Data rate and modulation.
 * @param[in] channel  Radio channel (frequency).
 */
void radio_modulated_tx_carrier(uint8_t tx_power, uint8_t mode, uint8_t channel);


/**
 * @brief Function for turning on the RX receive mode.
 *
 * @param[in] mode    Radio mode. Data rate and modulation.
 * @param[in] channel Radio channel (frequency).
 */
void radio_rx(uint8_t mode, uint8_t channel);


/**
 * @brief Function for turning on the TX carrier sweep. This test uses Timer 0 to restart the TX carrier at different channels.
 *
 * @param[in] tx_power      Radio output power.
 * @param[in] mode          Radio mode. Data rate and modulation.
 * @param[in] channel_start Radio start channel (frequency).
 * @param[in] channel_end   Radio end channel (frequency).
 * @param[in] delay_ms      Delay time in milliseconds.
 */
void radio_tx_sweep_start(uint8_t tx_power,
                          uint8_t mode,
                          uint8_t channel_start,
                          uint8_t channel_end,
                          uint8_t delay_ms);


/**
 * @brief Function for turning on the RX carrier sweep. This test uses Timer 0 to restart the RX carrier at different channels.
 *
 * @param[in] mode          Radio mode. Data rate and modulation.
 * @param[in] channel_start Radio start channel (frequency).
 * @param[in] channel_end   Radio end channel (frequency).
 * @param[in] delay_ms      Delay time in milliseconds.
 */
void radio_rx_sweep_start(uint8_t mode,
                          uint8_t channel_start,
                          uint8_t channel_end,
                          uint8_t delay_ms);


/**
 * @brief Function for stopping Timer 0.
 */
void radio_sweep_end(void);


/**
 * @brief Function for starting the duty-cycled modulated TX carrier by repeatedly sending a packet with a random address and
 * a random payload.
 *
 * @param[in] tx_power   Radio output power.
 * @param[in] mode       Radio mode. Data rate and modulation.
 * @param[in] channel    Radio start channel (frequency).
 * @param[in] duty_cycle Duty cycle.
 */
void radio_modulated_tx_carrier_duty_cycle(uint8_t tx_power,
                                           uint8_t mode,
                                           uint8_t channel,
                                           uint8_t duty_cycle);


/**
 * @brief Function for toggling the DC/DC converter state.
 *
 * @param[in] dcdc_state  DC/DC converter state.
 */
void toggle_dcdc_state(uint8_t dcdc_state);


#ifdef __cplusplus
}
#endif

#endif
