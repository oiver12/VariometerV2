#include "esp_timer.h"

#include <time.h>

uint64_t esp_timer_get_time()
{
    return (uint64_t)clock();
}