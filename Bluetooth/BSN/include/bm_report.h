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