#include "esp_timer.h"

#include <time.h>

uint64_t esp_timer_get_time()
{
    return (uint64_t)clock();
}

esp_err_t esp_timer_create(const esp_timer_create_args_t *args,
                           esp_timer_handle_t *out_handle)
{
    return ESP_OK;
}
esp_err_t esp_timer_start_periodic(esp_timer_handle_t timer, uint64_t period_us)
{
    return ESP_OK;
}