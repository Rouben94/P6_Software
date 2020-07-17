#ifndef BM_ZIGBEE_H
#define BM_ZIGBEE_H

/* Struct for benchmark message information */
typedef struct
{
  zb_uint16_t message_id;
  zb_uint64_t net_time;
  zb_uint64_t ack_net_time;
  zb_uint8_t number_of_hops;
  zb_uint8_t rssi;
  zb_uint16_t src_addr;
  zb_uint16_t dst_addr;
  zb_uint16_t group_addr;
  zb_uint16_t data_size;
} bm_message_info;

void bm_zigbee_init(void);

void bm_zigbee_enable(void);

#endif //BM_ZIGBEE_H

