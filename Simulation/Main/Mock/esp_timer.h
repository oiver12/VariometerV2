#ifndef _ESP_TIMER_H__
#define _ESP_TIMER_H__

#include <stdint.h>
#include "esp_err.h"

typedef void (*esp_timer_cb_t)(void *arg);

struct esp_timer
{
    uint64_t alarm;
};

typedef struct esp_timer *esp_timer_handle_t;

typedef enum
{
    ESP_TIMER_TASK, //!< Callback is called from timer task
#ifdef CONFIG_ESP_TIMER_SUPPORTS_ISR_DISPATCH_METHOD
    ESP_TIMER_ISR, //!< Callback is called from timer ISR
#endif
    ESP_TIMER_MAX, //!< Count of the methods for dispatching timer callback
} esp_timer_dispatch_t;

typedef struct
{
    esp_timer_cb_t callback;              //!< Function to call when timer expires
    void *arg;                            //!< Argument to pass to the callback
    esp_timer_dispatch_t dispatch_method; //!< Call the callback from task or from ISR
    const char *name;                     //!< Timer name, used in esp_timer_dump function
} esp_timer_create_args_t;

uint64_t esp_timer_get_time();
esp_err_t esp_timer_create(const esp_timer_create_args_t *args,
                           esp_timer_handle_t *out_handle);
esp_err_t esp_timer_start_periodic(esp_timer_handle_t timer, uint64_t period_us);

#endif