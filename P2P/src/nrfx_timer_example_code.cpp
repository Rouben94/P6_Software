#include <nrfx_timer.h>

/* proj.conf

    
CONFIG_NRFX_TIMER0=y
CONFIG_NRFX_RTC0=y

*/
#include <nrfx_rtc.h>
// For RTC see: https://devzone.nordicsemi.com/f/nordic-q-a/53519/zephyr-rtc


/* Timer used for ... */
static const nrfx_timer_t rtc = NRFX_TIMER_INSTANCE(0);

/* Timer Callback */
static void timer_handler(nrf_timer_event_t event_type, void *context)
{	
	if (event_type == NRF_TIMER_EVENT_COMPARE1) {
		/* For future use */
	}
}
/* Timer init */
static void timer_init()
{
	nrfx_err_t          err;
	// Takes 16.77s to expire
	nrfx_timer_config_t timer_cfg = {
		.frequency = NRF_TIMER_FREQ_1MHz,
		.mode      = NRF_TIMER_MODE_TIMER,
		.bit_width = NRF_TIMER_BIT_WIDTH_24,
		//.p_context = (void *) config, // Callback context
	};

	err = nrfx_timer_init(&timer, &timer_cfg, timer_handler);
	if (err != NRFX_SUCCESS) {
		printk("nrfx_timer_init failed with: %d\n", err);
	}
}

void main(){

	/* Init Timer0 */	
	timer_init();
	IRQ_CONNECT(TIMER0_IRQn, NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
	nrfx_timer_0_irq_handler, NULL, 0);

}
