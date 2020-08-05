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

/* AUTHOR 	   :    Raffael Anklin       */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BM_RADIO_OPERATING_TIME_COUNTER_H
#define BM_RADIO_OPERATING_TIME_COUNTER_H




/**
* Init the Operation Counter Timer
*
*/
void bm_op_time_counter_init();

/**
* Clear the Operation Time Counter
*
*/
void bm_op_time_counter_clear() ;

/**
* Operation Time Counter enable
*
*/
void bm_op_time_counter_enable();
/**
* Operation Time Counter disable
*
*/
void bm_op_time_counter_disable();
/**
* Operation Time Counter get
*
*/
uint64_t bm_op_time_counter_getOPTime();


#endif

#ifdef __cplusplus
}
#endif