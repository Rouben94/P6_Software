#ifndef BM_STATEMACHINE_H_
#define BM_STATEMACHINE_H_

#include <stdint.h>
#include <stdbool.h>

/* Typdef for benchmark state */
typedef enum
{
    BM_EMPTY_STATE,
    BM_DEFAULT_STATE,
    BM_STATE_1,
    BM_STATE_2,
    BM_STATE_3
} bm_state;

/* Struct for benchmark message information */
typedef struct
{
    uint64_t net_time;
    uint16_t message_id;
} bm_message_info;

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
void bm_save_message_info(uint16_t id);

#endif // BM_STATEMACHINE_H_