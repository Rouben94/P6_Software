#include "bm_simple_buttons_and_leds.h"

struct device *dev_led0, *dev_button0;
bool led0_is_on = true;
bool button0_toggel_state = false;
int ret;

static struct gpio_callback button_cb_data;

static struct k_delayed_work buttons_debounce;

static void (*button0_callback)();

static void buttons_debounce_fn(struct k_work *work)
{
	button0_toggel_state = !button0_toggel_state;
	button0_callback();
}

void button_pressed(struct device *dev, struct gpio_callback *cb,
					u32_t pins)
{
	if (dev == dev_button0)
	{
		k_delayed_work_submit(&buttons_debounce, K_MSEC(170)); // Debounce the Button
															 //button0_callback();
	}
	//printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

/* Init the Leds and Buttons */
extern void init_leds_buttons(void (*button0_cb)())
{
	dev_led0 = device_get_binding(LED0);
	if (dev_led0 == NULL)
	{
		printk("Error no LED0 found");
		return;
	}
	ret = gpio_pin_configure(dev_led0, PIN_LED0, GPIO_OUTPUT_ACTIVE | FLAGS_LED0);
	if (ret < 0)
	{
		printk("Error in configuration of led0 pin (err %d)", ret);
		return;
	}
	dev_button0 = device_get_binding(SW0_GPIO_LABEL);
	if (dev_button0 == NULL)
	{
		printk("Error: didn't find %s device\n", SW0_GPIO_LABEL);
		return;
	}
	ret = gpio_pin_configure(dev_button0, SW0_GPIO_PIN, SW0_GPIO_FLAGS);
	if (ret != 0)
	{
		printk("Error %d: failed to configure %s pin %d\n",
			   ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}
	ret = gpio_pin_interrupt_configure(dev_button0,
									   SW0_GPIO_PIN,
									   GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0)
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			   ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(SW0_GPIO_PIN));
	gpio_add_callback(dev_button0, &button_cb_data);
	printk("Set up button at %s pin %d\n", SW0_GPIO_LABEL, SW0_GPIO_PIN);
	button0_callback = button0_cb;

	k_delayed_work_init(&buttons_debounce, buttons_debounce_fn);
}

/* Set LED0 */
extern void led0_set(bool state)
{
	gpio_pin_set(dev_led0, PIN_LED0, (int)state);
	led0_is_on = state;
}

/* Get LED0 */
extern bool led0_get()
{
	//gpio_pin_get(dev_led0, PIN_LED0, (int)state);
	return led0_is_on;
}

/* Get BUTTON0 Toggled State*/
extern bool button0_toggle_state_get()
{
	//gpio_pin_get(dev_led0, PIN_LED0, (int)state);
	return button0_toggel_state;
}
