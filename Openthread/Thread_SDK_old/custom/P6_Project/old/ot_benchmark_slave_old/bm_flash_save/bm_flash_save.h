#include "fds.h"


/**@brief   Struct of Measurement
 *
 * This struct repesents all measured values of the network test
 *
 * @note 
 */
typedef struct {
  uint16_t MAC;
}Measurement;

/**@brief   Callback pointer for the read function. 
 *
 * This Callback has to be initialized with the flash_save_init() function.
 * The callback will be called as many as records are present in the flash.
 *
 * @note 
 */
typedef void (*flash_cb_t)(Measurement *data);

/**@brief   Write to flash function
 *
 * This function writes the actual falues of the Measurement struct to the flash.
 *
 * @note 
 */
void flash_write(Measurement measure);

/**@brief   Read from flash
 *
 * This function read the Measurement struct records from the flash an passes through 
 * the callback function.
 *
 * @note 
 */
void flash_read(void);

/**@brief   Delete the records in Flash
 *
 * This Function delets all saved records in flash.
 *
 * @note 
 */
void flash_delete(void);

/**@brief   Init the Flash save module
 *
 * This function inits the flash save module. The callback should be passed 
 * thorugh from the user 
 *
 * @note 
 */
void flash_save_init(flash_cb_t evt_handler);
