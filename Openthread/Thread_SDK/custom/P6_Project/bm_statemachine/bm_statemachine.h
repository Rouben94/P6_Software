#ifndef BM_STATEMACHINE_H_
#define BM_STATEMACHINE_H_

#include <stdint.h>
#include <stdbool.h>
#include <openthread/ip6.h>

/* Typdef for benchmark state */
typedef enum
{
    BM_EMPTY_STATE,
    BM_STATE_1_CLIENT,
    BM_STATE_2_CLIENT,
    BM_STATE_1_SERVER,
    BM_STATE_2_SERVER,
    BM_STATE_3_SLAVE,
    BM_STATE_4_SLAVE,
    BM_STATE_1_MASTER,
    BM_STATE_2_MASTER,
    BM_STATE_3_MASTER,
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
    uint64_t net_time;
    uint64_t net_time_ack;
    uint16_t source_address;
    uint16_t dest_address;
    uint16_t grp_address;
    uint16_t data_size;
    uint16_t message_id;
    uint8_t  number_of_hops;
    int8_t   RSSI;
} __attribute__((packed)) bm_message_info;

/**@brief Benchmark configuration structure. */
typedef struct
{
  bool bm_status;                               /**<  Indicates if the benchmark should start or stop*/
  otIp6Address master_address;
  uint64_t bm_master_time_stamp;
  uint16_t bm_time;                             /**<  Tells the slave node how long the benchmark does take*/
  uint16_t bm_nbr_of_msg;
  uint16_t bm_msg_size;
} __attribute__((packed)) bm_master_message;

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
void bm_start_param_set(bm_master_message *start_param);

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
void bm_save_result(bm_message_info message[], uint16_t size);

/**@brief 
 *
 * @details 
 */
void bm_set_additional_payload(bool state);

/**@brief 
 *
 * @details 
 */
void bm_set_ack_received(uint8_t received);

/**@brief 
 *
 * @details 
 */
uint8_t bm_get_node_id(void);

#endif // BM_STATEMACHINE_H_