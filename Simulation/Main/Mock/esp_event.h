#ifndef _ESP_EVENT_H_
#define _ESP_EVENT_H_

#include <stdint.h>

#define ESP_EVENT_DECLARE_BASE(id) extern esp_event_base_t const id

typedef const char *esp_event_base_t; /**< unique pointer to a subsystem that exposes events */
typedef void (*esp_event_handler_t)(void *event_handler_arg,
                                    esp_event_base_t event_base,
                                    int32_t event_id,
                                    void *event_data); /**< function called when an event is posted to the queue */

#endif // _ESP_EVENT_H_