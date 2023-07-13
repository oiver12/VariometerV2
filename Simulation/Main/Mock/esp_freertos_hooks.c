#include "esp_freertos_hooks.h"

esp_err_t esp_register_freertos_idle_hook_for_cpu(esp_freertos_idle_cb_t new_idle_cb, UBaseType_t cpuid)
{
    return ESP_OK;
}