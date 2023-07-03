#ifndef _ESP_FREERTOS_HOOKS_H_
#define _ESP_FREERTOS_HOOKS_H_

#include "esp_err.h"

typedef bool (*esp_freertos_idle_cb_t)(void);
typedef unsigned int UBaseType_t;

esp_err_t esp_register_freertos_idle_hook_for_cpu(esp_freertos_idle_cb_t new_idle_cb, UBaseType_t cpuid);

#endif // _ESP_FREERTOS_HOOKS_H_