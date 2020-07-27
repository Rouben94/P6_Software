#ifndef BM_ZIGBEE_H
#define BM_ZIGBEE_H

#include "zboss_api.h"
#include "zcl/zb_zcl_basic_addons.h"
#include "zcl/zb_zcl_groups_addons.h"
#include "zcl/zb_zcl_level_control_addons.h"
#include "zcl/zb_zcl_on_off_addons.h"
#include "zcl/zb_zcl_scenes_addons.h"

void bm_zigbee_init(void);

void bm_zigbee_enable(void);

void bm_report_data(zb_uint8_t param);

#endif //BM_ZIGBEE_H