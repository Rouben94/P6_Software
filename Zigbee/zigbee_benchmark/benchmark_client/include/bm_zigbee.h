

#ifndef BM_ZIGBEE_H
#define BM_ZIGBEE_H

typedef struct light_switch_bulb_params_s {
  zb_uint8_t endpoint;
  zb_uint16_t short_addr;
} light_switch_bulb_params_t;

typedef struct light_switch_button_s {
  zb_bool_t in_progress;
  zb_time_t timestamp;
} light_switch_button_t;

typedef struct light_switch_ctx_s {
  light_switch_bulb_params_t bulb_params;
  light_switch_button_t button;
} light_switch_ctx_t;

/* Struct defining Benchmark Device Context structure. */
typedef struct
{
  zb_zcl_basic_attrs_ext_t basic_attr;
  zb_zcl_identify_attrs_t identify_attr;
  zb_zcl_scenes_attrs_t scenes_attr;
  zb_zcl_groups_attrs_t groups_attr;
  zb_zcl_on_off_attrs_t on_off_attr;
  zb_zcl_level_control_attrs_t level_control_attr;
} bm_client_device_ctx_t;

void bm_send_control_message_cb(zb_bufid_t bufid, zb_uint16_t level);

void bm_send_message_cb(zb_bufid_t bufid, zb_uint16_t level);

void bm_send_message(zb_uint8_t param);

/* Function to send Benchmark Reporting Message */
void bm_send_reporting_message(zb_uint8_t bufid);

void bm_reporting_message(zb_bufid_t bufid, zb_uint16_t level);

void bm_receive_config(zb_uint8_t bufid);

void bm_zigbee_init(void);

void bm_zigbee_enable(void);

#endif //BM_ZIGBEE_H