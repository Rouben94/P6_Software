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

/* AUTHOR	   :   Robin Bobst        */
/* AUTHOR	   :   Cyrill Horath        */

#include "bm_config.h"
#ifdef NRF_SDK_ZIGBEE

#include "fds.h"
#include "zboss_api.h"


/**@brief   Struct of Measurement
 *
 * This struct repesents all measured values of the network test
 *
 * @note 
 */
typedef struct {
  zb_uint16_t message_id;
  zb_uint64_t net_time;
  zb_uint64_t ack_net_time;
  zb_uint8_t number_of_hops;
  zb_uint8_t rssi;
  zb_uint16_t src_addr;
  zb_uint16_t dst_addr;
  zb_uint16_t group_addr;
  zb_uint16_t data_size;
} Measurement;

/**@brief   Callback pointer for the read function. 
 *
 * This Callback has to be initialized with the flash_save_init() function.
 * The callback will be called as many as records are present in the flash.
 *
 * @note 
 */
typedef void (*flash_cb_t)(Measurement *data);

/**@brief   Write to flash function
 *
 * This function writes the actual falues of the Measurement struct to the flash.
 *
 * @note 
 */
void flash_write(Measurement measure);

/**@brief   Read from flash
 *
 * This function read the Measurement struct records from the flash an passes through 
 * the callback function.
 *
 * @note 
 */
void flash_read(void);

/**@brief   Delete the records in Flash
 *
 * This Function delets all saved records in flash.
 *
 * @note 
 */
void flash_delete(void);

/**@brief   Init the Flash save module
 *
 * This function inits the flash save module. The callback should be passed 
 * thorugh from the user 
 *
 * @note 
 */
void flash_save_init(flash_cb_t evt_handler);

#endif