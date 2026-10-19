#include "esp_timer.h"

#include <time.h>
#include <stdbool.h>

uint64_t esp_timer_get_time()
{
    static struct timespec origin;
    static bool initialized = false;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (!initialized) {
        origin = now;
        initialized = true;
    }
    return (uint64_t)((now.tv_sec - origin.tv_sec) * 1000000LL +
                      (now.tv_nsec - origin.tv_nsec) / 1000);
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
