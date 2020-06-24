#ifndef BM_MASTER_CLI_H_
#define BM_MASTER_CLI_H_


#include <stdint.h>
#include <stdbool.h>

/**@brief Function for initialize custom cli commands */
void bm_custom_cli_init(void);

/**@brief Function for initialize custom cli commands */
void bm_cli_write_result(uint64_t time, uint16_t ID);

#endif // BM_MASTER_CLI_H_