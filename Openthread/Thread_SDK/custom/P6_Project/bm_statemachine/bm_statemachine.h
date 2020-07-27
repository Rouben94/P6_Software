#ifndef BM_STATEMACHINE_H_
#define BM_STATEMACHINE_H_

#include <stdint.h>
#include <stdbool.h>
#include <openthread/ip6.h>

/* Typdef for benchmark state */
typedef enum
{
    BM_EMPTY_STATE,
    BM_STATE_1_SLAVE,
    BM_STATE_2_SLAVE,
    BM_STATE_1_MASTER,
    BM_STATE_2_MASTER,
} bm_state;

/* Typdef for benchmark state */
typedef enum
{
    BM_1bit = 1,
    BM_1024Bytes
} bm_data_size;

/* Struct for benchmark message information */
typedef struct
{
    otIp6Address source_address;
    otIp6Address dest_address;
    otIp6Address grp_address;
    uint64_t net_time;
    uint64_t net_time_ack;
    uint16_t number_of_hops;
    uint16_t message_id;
    uint8_t  index;
    int8_t   RSSI;
    bool     data_size;
    
} bm_message_info;

/**@brief Function for processing the benchmark pending tasks.
 *
 * @details This function must be periodically executed to process the benchmark pending tasks.
 */
void bm_statemachine_init(void);

/**@brief Function for processing the benchmark pending tasks.
 *
 * @details This function must be periodically executed to process the benchmark pending tasks.
 */
void bm_sm_process(void);

/**@brief 
 *
 * @details 
 */
void bm_sm_time_set(uint32_t time);

/**@brief 
 *
 * @details 
 */
void bm_sm_new_state_set(uint8_t state);

/**@brief 
 *
 * @details 
 */
void bm_save_message_info(bm_message_info message);

/**@brief 
 *
 * @details 
 */
void bm_save_result(bm_message_info message);

/**@brief 
 *
 * @details 
 */
void bm_save_slave_address(otIp6Address slave_address);

/**@brief 
 *
 * @details 
 */
void bm_stop_set(bool state);

/**@brief 
 *
 * @details 
 */
void bm_set_data_size(uint8_t size);

#endif // BM_STATEMACHINE_H_