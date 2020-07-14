/*
 * Zigbee Benchmark Client
 * Acting as a Dimmer Light Switch
 */

#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <soc.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>
#include <drivers/gpio.h>

#include "bm_zigbee.h"

#define RUN_STATUS_LED DK_LED1		/* Node Status LED */
#define RUN_LED_BLINK_INTERVAL 1000 /* Blink Interval for the Node Status LED */

LOG_MODULE_REGISTER(app_bm_main);

void main(void)
{
	int blink_status = 0;

	LOG_INF("Starting BENCHMARK Client");

	bm_zigbee_init();

	bm_zigbee_enable();

	while (1)
	{
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}
