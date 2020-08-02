/**
 * Copyright (c) 2017-2019 - 2020, Nordic Semiconductor ASA
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

#ifndef THREAD_COAP_UTILS_H__
#define THREAD_COAP_UTILS_H__

#include <stdbool.h>
#include <openthread/coap.h>

#include "thread_utils.h"
#include "bm_statemachine.h"

/**@brief Benchmark configuration structure. */
typedef struct
{
  bool bm_status;                               /**<  Indicates if the benchmark should start or stop*/
  otIp6Address bm_master_ip6_address;   /**<  Tells the slave node which Ip6 address the master node has*/
  uint32_t bm_time;                             /**<  Tells the slave node how long the benchmark does take*/
} bm_master_message;

/**@brief Thread CoAP utils configuration structure. */
typedef struct
{
    bool coap_server_enabled;                /**< Indicates if CoAP Server should be enabled. */
    bool coap_client_enabled;                /**< Indicates if CoAP Client should be enabled. */
} thread_coap_utils_configuration_t;

/***************************************************************************************************
 * @section CoAP utils core functions.
 **************************************************************************************************/

/**@brief Function for initializing the CoAP service with specified resources.
 *
 * @details The @p thread_init function needs to be executed before calling this function.
 *
 * @param[in] p_config A pointer to the Thread CoAP configuration structure.
 *
 */
void thread_coap_utils_init(const thread_coap_utils_configuration_t * p_config);


/**@brief Function for stopping the CoAP service. */
void thread_coap_utils_deinit(void);

/***************************************************************************************************
 * @section CoAP client function proptypes.
 **************************************************************************************************/

/**@brief Function for sending the benchmark start request message to the multicast IPv6 address.
 *
 * @param[in] message  message struct with benchmark start / Ip6 address from master node / benchmark time.
 * @param[in] scope    IPv6 multicast address scope.
 *
 */
void bm_coap_multicast_start_send(bm_master_message message);

/**@brief Function for sending the benchmark test message to the peered unicast IPv6 address.
 *
 * @param[in] message  message state
 *
 */
void bm_coap_probe_message_send(uint8_t state);

/**@brief Function for sending the benchmark test message to the peered unicast IPv6 address.
 *
 * @param[in] message  message state
 *
 */
void bm_coap_results_send(bm_message_info message_info[], uint16_t size);

/**@brief Function for sending the benchmark test message to the peered unicast IPv6 address.
 *
 * @param[in] message  message state
 *
 */
void bm_coap_result_request_send(otIp6Address address);

/**@brief Function for sending the benchmark test message to the peered unicast IPv6 address.
 *
 * @param[in] message  message state
 *
 */
void bm_increment_group_address(void);

/**@brief Function for sending the benchmark test message to the peered unicast IPv6 address.
 *
 * @param[in] message  message state
 *
 */
void bm_decrement_group_address(void);


#endif /* THREAD_COAP_UTILS_H__ */
