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

/* AUTHOR   :   Raffael Anklin        */
/* Co-AUTHOR   :   Cyrill Horath        */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_REPORT_H
#define BM_REPORT_H

#include "bm_log.h"


bool bm_report_msg_publish(bm_message_info *message_info);

bool bm_report_msg_subscribe(bm_message_info * bm_message_info);

#endif

#ifdef __cplusplus
}
#endif