/*
 * Zigbee Benchmark Server
 * Acting as a Light Bulb
 */

#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <soc.h>
#include <drivers/pwm.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>
#include <drivers/gpio.h>

#include "bm_zigbee.h"

LOG_MODULE_REGISTER(app_bm_main);


void main(void)
{
	LOG_INF("Starting BENCHMARK Server");

	bm_zigbee_init();

	bm_zigbee_enable();
}
