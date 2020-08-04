#ifndef ZB_MEM_CONFIG_CUSTOM_H
#define ZB_MEM_CONFIG_CUSTOM_H 1

//#define ZB_CONFIG_ROLE_ZED
#define ZB_CONFIG_ROLE_ZR
//#define ZB_CONFIG_ROLE_ZC

/*#define ZB_CONFIG_OVERALL_NETWORK_SIZE 128*/
/*#define ZB_CONFIG_OVERALL_NETWORK_SIZE 32*/
#define ZB_CONFIG_OVERALL_NETWORK_SIZE 60

/*#define ZB_CONFIG_HIGH_TRAFFIC*/
/*#define ZB_CONFIG_MODERATE_TRAFFIC*/
/**
   Light routing and application traffic from/to that device.
 */
#define ZB_CONFIG_HIGH_TRAFFIC

/*#define ZB_CONFIG_APPLICATION_COMPLEX*/
/*#define ZB_CONFIG_APPLICATION_MODERATE*/
/**
   Simple user's application at that device: not too many relations to other devices.
 */
//#define ZB_CONFIG_APPLICATION_SIMPLE
#define ZB_CONFIG_APPLICATION_COMPLEX

/**
   @}
*/

/* Now common logic derives numerical parameters from the defined configuration. */
#include "zb_mem_config_common.h"

/* Now if you REALLY know what you do, you can study zb_mem_config_common.h and redefine some configuration parameters, like: */
#undef ZB_CONFIG_SCHEDULER_Q_SIZE
#define ZB_CONFIG_SCHEDULER_Q_SIZE 56

#undef ZB_CONFIG_IOBUF_POOL_SIZE
#define ZB_CONFIG_IOBUF_POOL_SIZE 96

#undef ZB_CONFIG_MAC_PENDING_QUEUE_SIZE
#define ZB_CONFIG_MAC_PENDING_QUEUE_SIZE (ZB_CONFIG_IOBUF_POOL_SIZE / 4)

/* Memory context definitions */
#include "zb_mem_config_context.h"

#endif /* ZB_MEM_CONFIG_CUSTOM_H */