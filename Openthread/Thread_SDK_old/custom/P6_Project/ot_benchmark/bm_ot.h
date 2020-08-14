/*
This file is part of Benchamrk-Shared-Library.

Benchamrk-Shared-Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Benchamrk-Shared-Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Benchamrk-Shared-Library.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR 	   :    Robin Bobst       */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_OT_H
#define BM_OT_H

#include <stdbool.h>

/**@brief Thread CoAP utils configuration structure. */
typedef struct
{
    bool coap_server_enabled;                /**< Indicates if CoAP Server should be enabled. */
    bool coap_client_enabled;                /**< Indicates if CoAP Client should be enabled. */
} thread_coap_utils_configuration_t;

//Function for init the Openthread stack
void bm_ot_init();

//Function for sending a probe message
void bm_send_message();

#ifdef __cplusplus
}
#endif
#endif