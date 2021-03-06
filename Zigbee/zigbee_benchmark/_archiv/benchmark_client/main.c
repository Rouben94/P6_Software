/*
This file is part of Zigbee-Benchmark.

Zigbee-Benchmark is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Zigbee-Benchmark is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Zigbee-Benchmark. If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR 	   :    Cyrill Horath      */

#include "bm_cli.h"
#include "bm_config.h"
#include "bm_statemachine.h"

/**@brief Function for application main entry.
 */
int main(void) {

#ifdef NRF_SDK_ZIGBEE
  // Init the LOG Modul
  // To Enable Loging before the Mesh stack is run change in sdk_config -> NRF_LOG_DEFERRED = 0 (Performance is worse but logging is possibel)
  bm_cli_log_init(); /* Initialize the Zigbee LOG subsystem */
#endif
  bm_statemachine();
}