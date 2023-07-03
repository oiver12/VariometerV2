#ifndef _ESP_SLEEP_H_
#define _ESP_SLEEP_H_
#include "driver/gpio.h"

esp_err_t esp_sleep_enable_ext0_wakeup(gpio_num_t gpio_num, int level);
void esp_deep_sleep_start(void);

#endif