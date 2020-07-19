#ifdef __cplusplus
extern "C" {
#endif

#ifndef SIMPLE_BUTTONS_AND_LEDS_H
#define SIMPLE_BUTTONS_AND_LEDS_H

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
* Init the Leds and Buttons
*
*/
extern void init_leds_buttons();

/**
* Set the LED0 State
*
* @param State to set the LED0 to (0/1)
*/
extern void led0_set(bool state);

/**
* Get the LED0 State
*
* @return State of the LED0 (0/1)
*/
extern bool led0_get();

/**
* Get the BUTTON0 Toggled State
*
* @return State of the BUTTON0 Toggled State (0/1)
*/
extern bool button0_toggle_state_get();

void button0_toggle_state_set(bool newstate);


#endif

#ifdef __cplusplus
}
#endif