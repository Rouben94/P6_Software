/*
 * Zigbee Benchmark Master
 * Acting as a Zigbee Coordinator
 */

#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <soc.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>

#include <shell/shell.h>

#include "bm_zigbee.h"

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

LOG_MODULE_REGISTER(app_bm_main);

void main(void)
{
	int blink_status = 0;

	LOG_INF("Starting BENCHMARK Master / Zigbee Coordinator");

	bm_zigbee_init();

	bm_zigbee_enable();

	while (1)
	{
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}
