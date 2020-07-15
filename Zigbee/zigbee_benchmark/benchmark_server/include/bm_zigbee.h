

#ifndef BM_ZIGBEE_H
#define BM_ZIGBEE_H

/* Struct for benchmark message information */
typedef struct
{
  zb_ieee_addr_t ieee_dst_addr;
  zb_uint16_t src_addr;
  zb_uint16_t dst_addr;
  zb_uint16_t group_addr;
  zb_uint64_t net_time;
  zb_uint16_t number_of_hops;
  zb_uint16_t message_id;
  zb_uint8_t RSSI;
  bool data_size;
} bm_message_info;


void bm_zigbee_init(void);

void bm_zigbee_enable(void);

void bm_save_message_info(bm_message_info message);


#endif //BM_ZIGBEE_H

