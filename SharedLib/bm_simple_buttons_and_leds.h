#ifdef __cplusplus
extern "C" {
#endif

#ifndef SIMPLE_BUTTONS_AND_LEDS_H
#define SIMPLE_BUTTONS_AND_LEDS_H

#include "bm_config.h"


#ifdef ZEPHYR_BLE_MESH

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/slist.h>

/*
 * Devicetree helper macro which gets the 'flags' cell from a 'gpios'
 * property, or returns 0 if the property has no 'flags' cell.
 */

#define FLAGS_OR_ZERO(node)						\
	COND_CODE_1(DT_PHA_HAS_CELL(node, gpios, flags),		\
		    (DT_GPIO_FLAGS(node, gpios)),			\
		    (0))



/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN_LED0	DT_GPIO_PIN(LED0_NODE, gpios)
#if DT_PHA_HAS_CELL(LED0_NODE, gpios, flags)
#define FLAGS_LED0	DT_GPIO_FLAGS(LED0_NODE, gpios)
#endif
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN_LED0	0
#endif

#ifndef FLAGS_LED0
#define FLAGS_LED0	0
#endif

/* The devicetree node identifier for the "led1" alias. Is the RED LED on THe nRF52 Donlge*/
#define LED1_NODE DT_ALIAS(led1)

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
#define LED1	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN_LED1	DT_GPIO_PIN(LED1_NODE, gpios)
#if DT_PHA_HAS_CELL(LED1_NODE, gpios, flags)
#define FLAGS_LED1	DT_GPIO_FLAGS(LED1_NODE, gpios)
#endif
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led1 devicetree alias is not defined"
#define LED1	""
#define PIN_LED1	0
#endif

#ifndef FLAGS_LED1
#define FLAGS_LED1	0
#endif


/* The devicetree node identifier for the "led2" alias. Is the GREEN LED on THe nRF52 Donlge*/
#define LED2_NODE DT_ALIAS(led2)

#if DT_NODE_HAS_STATUS(LED2_NODE, okay)
#define LED2	DT_GPIO_LABEL(LED2_NODE, gpios)
#define PIN_LED2	DT_GPIO_PIN(LED2_NODE, gpios)
#if DT_PHA_HAS_CELL(LED2_NODE, gpios, flags)
#define FLAGS_LED2	DT_GPIO_FLAGS(LED2_NODE, gpios)
#endif
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led2 devicetree alias is not defined"
#define LED2	""
#define PIN_LED2	0
#endif

#ifndef FLAGS_LED2
#define FLAGS_LED1	0
#endif

/* The devicetree node identifier for the "led3" alias. Is the BLUE LED on THe nRF52 Donlge*/
#define LED3_NODE DT_ALIAS(led3)

#if DT_NODE_HAS_STATUS(LED3_NODE, okay)
#define LED3	DT_GPIO_LABEL(LED3_NODE, gpios)
#define PIN_LED3	DT_GPIO_PIN(LED3_NODE, gpios)
#if DT_PHA_HAS_CELL(LED3_NODE, gpios, flags)
#define FLAGS_LED3	DT_GPIO_FLAGS(LED3_NODE, gpios)
#endif
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led3 devicetree alias is not defined"
#define LED3	""
#define PIN_LED3	0
#endif

#ifndef FLAGS_LED3
#define FLAGS_LED3	0
#endif


/*
 * Get button configuration from the devicetree sw0 alias.
 *
 * At least a GPIO device and pin number must be provided. The 'flags'
 * cell is optional.
 */

#define SW0_NODE	DT_ALIAS(sw0)

#if DT_NODE_HAS_STATUS(SW0_NODE, okay)
#define SW0_GPIO_LABEL	DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN	DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS	(GPIO_INPUT | FLAGS_OR_ZERO(SW0_NODE) | GPIO_INT_DEBOUNCE)
#else
#error "Unsupported board: sw0 devicetree alias is not defined"
#define SW0_GPIO_LABEL	""
#define SW0_GPIO_PIN	0
#define SW0_GPIO_FLAGS	0
#endif

/**
* Init the Buttons
*
*/
void bm_init_buttons(void (*button0_cb)());


/**
* Get the BUTTON0 Toggled State
*
* @return State of the BUTTON0 Toggled State (0/1)
*/
bool bm_button0_toggle_state_get();

/**
* Set the BUTTON0 Toggled State
*
* @param State of the BUTTON0 Toggled State (0/1)
*/
void bm_button0_toggle_state_set(bool newstate);

#elif defined NRF_SDK_Zigbee

#include "boards.h"
#include "bsp.h"


#endif


/**
* Init the Leds
*
*/
void bm_init_leds();

/**
* Set the LED0 State
*
* @param State to set the LED0 to (0/1) (GREEN no RGB on nRF52 Dongle / LED1 on nRF52 DK)
*/
void bm_led0_set(bool state);

/**
* Get the LED0 State
*
* @return State of the LED0 (0/1) (GREEN no RGB on nRF52 Dongle / LED1 on nRF52 DK)
*/
bool bm_led0_get();


/**
* Set the LED1 State
*
* @param State to set the LED1 to (0/1) (RED on nRF52 Dongle / LED2 on nRF52 DK)
*/
void bm_led1_set(bool state);

/**
* Get the LED1 State
*
* @return State of the LED1 (0/1) (RED on nRF52 Dongle / LED2 on nRF52 DK)
*/
bool bm_led1_get();

/**
* Set the LED2 State
*
* @param State to set the LED2 to (0/1) (GREEN on nRF52 Dongle / LED3 on nRF52 DK)
*/
void bm_led2_set(bool state);

/**
* Get the LED2 State
*
* @return State of the LED2 (0/1) (GREEN on nRF52 Dongle / LED3 on nRF52 DK)
*/
bool bm_led2_get();

/**
* Set the LED3 State
*
* @param State to set the LED3 to (0/1) (BLUE on nRF52 Dongle / LED4 on nRF52 DK)
*/
void bm_led3_set(bool state);

/**
* Get the LED3 State
*
* @return State of the LED3 (0/1) (BLUE on nRF52 Dongle / LED4 on nRF52 DK)
*/
bool bm_led3_get();



#endif

#ifdef __cplusplus
}
#endif