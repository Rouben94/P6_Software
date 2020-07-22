#include "bm_simple_buttons_and_leds.h"

#ifdef ZEPHYR_BLE_MESH

struct device *dev_led0, *dev_led1, *dev_led2, *dev_led3, *dev_button0;
bool led0_is_on, led1_is_on, led2_is_on, led3_is_on = true;
bool button0_toggel_state = false;
int ret;

static struct gpio_callback button_cb_data;

static struct k_delayed_work buttons_debounce;

static void (*button0_callback)();

static void buttons_debounce_fn(struct k_work *work) {
  button0_toggel_state = !button0_toggel_state;
  button0_callback();
}

void button_pressed(struct device *dev, struct gpio_callback *cb,
    u32_t pins) {
  if (dev == dev_button0) {
    k_delayed_work_submit(&buttons_debounce, K_MSEC(170)); // Debounce the Button
                                                           //button0_callback();
  }
  //printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

/* Init the Buttons */
void bm_init_buttons(void (*button0_cb)()) {
  dev_button0 = device_get_binding(SW0_GPIO_LABEL);
  if (dev_button0 == NULL) {
    printk("Error: didn't find %s device\n", SW0_GPIO_LABEL);
    return;
  }
  ret = gpio_pin_configure(dev_button0, SW0_GPIO_PIN, SW0_GPIO_FLAGS);
  if (ret != 0) {
    printk("Error %d: failed to configure %s pin %d\n",
        ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
    return;
  }
  ret = gpio_pin_interrupt_configure(dev_button0,
      SW0_GPIO_PIN,
      GPIO_INT_EDGE_TO_ACTIVE);
  if (ret != 0) {
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

/* Init the Leds */
void bm_init_leds() {
  dev_led0 = device_get_binding(LED0);
  if (dev_led0 == NULL) {
    printk("Error no LED0 found");
    return;
  }
  ret = gpio_pin_configure(dev_led0, PIN_LED0, GPIO_OUTPUT_INACTIVE | FLAGS_LED0);
  if (ret < 0) {
    printk("Error in configuration of led0 pin (err %d)", ret);
    return;
  }
  dev_led1 = device_get_binding(LED1);
  if (dev_led1 == NULL) {
    printk("Error no LED1 found");
    return;
  }
  ret = gpio_pin_configure(dev_led1, PIN_LED1, GPIO_OUTPUT_INACTIVE | FLAGS_LED1);
  if (ret < 0) {
    printk("Error in configuration of led1 pin (err %d)", ret);
    return;
  }
  dev_led2 = device_get_binding(LED2);
  if (dev_led2 == NULL) {
    printk("Error no LED2 found");
    return;
  }
  ret = gpio_pin_configure(dev_led2, PIN_LED2, GPIO_OUTPUT_INACTIVE | FLAGS_LED2);
  if (ret < 0) {
    printk("Error in configuration of led2 pin (err %d)", ret);
    return;
  }
  dev_led3 = device_get_binding(LED3);
  if (dev_led3 == NULL) {
    printk("Error no LED3 found");
    return;
  }
  ret = gpio_pin_configure(dev_led3, PIN_LED3, GPIO_OUTPUT_INACTIVE | FLAGS_LED3);
  if (ret < 0) {
    printk("Error in configuration of led3 pin (err %d)", ret);
    return;
  }
}

#elif defined NRF_SDK_Zigbee

/* Init the Leds */
void bm_init_leds() {
  ret_code_t error_code;

  /* Initialize LEDs and buttons - use BSP to control them. */
  error_code = bsp_init(BSP_INIT_LEDS, NULL);
  APP_ERROR_CHECK(error_code);
  /* By default the bsp_init attaches BSP_KEY_EVENTS_{0-4} to the PUSH events of the corresponding buttons. */

  bsp_board_leds_off();
  return;
}

#endif

/* Set LED0 */
void bm_led0_set(bool state) {
#ifdef ZEPHYR_BLE_MESH
  gpio_pin_set(dev_led0, PIN_LED0, (int)state);
  led0_is_on = state;
#elif defined NRF_SDK_Zigbee
  if (state) {
    bsp_board_led_on(BSP_BOARD_LED_0);
  } else {
    bsp_board_led_off(BSP_BOARD_LED_0);
  }
#endif
}

/* Get LED0 */
bool bm_led0_get() {
#ifdef ZEPHYR_BLE_MESH
  return led0_is_on;
#elif defined NRF_SDK_Zigbee
  return bsp_board_led_state_get(BSP_BOARD_LED_0);
#endif
}

/* Set LED1 */
void bm_led1_set(bool state) {
#ifdef ZEPHYR_BLE_MESH
  gpio_pin_set(dev_led1, PIN_LED1, (int)state);
  led1_is_on = state;
#elif defined NRF_SDK_Zigbee
  if (state) {
    bsp_board_led_on(BSP_BOARD_LED_1);
  } else {
    bsp_board_led_off(BSP_BOARD_LED_1);
  }
#endif
}

/* Get LED1 */
bool bm_led1_get() {
#ifdef ZEPHYR_BLE_MESH
  return led1_is_on;
#elif defined NRF_SDK_Zigbee
  return bsp_board_led_state_get(BSP_BOARD_LED_1);
#endif
}

/* Set LED2 */
void bm_led2_set(bool state) {
#ifdef ZEPHYR_BLE_MESH
  gpio_pin_set(dev_led2, PIN_LED2, (int)state);
  led2_is_on = state;
#elif defined NRF_SDK_Zigbee
  if (state) {
    bsp_board_led_on(BSP_BOARD_LED_2);
  } else {
    bsp_board_led_off(BSP_BOARD_LED_2);
  }
#endif
}

/* Get LED2 */
bool bm_led2_get() {
#ifdef ZEPHYR_BLE_MESH
  return led2_is_on;
#elif defined NRF_SDK_Zigbee
  return bsp_board_led_state_get(BSP_BOARD_LED_2);
#endif
}

/* Set LED3 */
void bm_led3_set(bool state) {
#ifdef ZEPHYR_BLE_MESH
  gpio_pin_set(dev_led3, PIN_LED3, (int)state);
  led3_is_on = state;
#elif defined NRF_SDK_Zigbee
  if (state) {
    bsp_board_led_on(BSP_BOARD_LED_3);
  } else {
    bsp_board_led_off(BSP_BOARD_LED_3);
  }
#endif
}

/* Get LED3 */
bool bm_led3_get() {
#ifdef ZEPHYR_BLE_MESH
  return led3_is_on;
#elif defined NRF_SDK_Zigbee
  return bsp_board_led_state_get(BSP_BOARD_LED_3);
#endif
}

#ifdef ZEPHYR_BLE_MESH

/* Get BUTTON0 Toggled State*/
bool bm_button0_toggle_state_get() {
  //gpio_pin_get(dev_led0, PIN_LED0, (int)state);
  return button0_toggel_state;
}

/* Set BUTTON0 Toggled State*/
void bm_button0_toggle_state_set(bool newstate) {
  //gpio_pin_get(dev_led0, PIN_LED0, (int)state);
  button0_toggel_state = newstate;
}

#endif