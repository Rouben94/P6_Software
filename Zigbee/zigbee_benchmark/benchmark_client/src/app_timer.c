#include <zephyr.h>
#include <device.h>
#include <logging/log.h>

void my_work_handler(struct k_work *work)
{
    LOG_INF("Test Timer Handler");
    
}

K_WORK_DEFINE(my_work, my_work_handler);

void app_timer_handler(struct k_timer *dummy)
{
    k_work_submit(&my_work);
}
K_TIMER_DEFINE(app_timer, app_timer_handler, NULL);

