#ifndef ESP_NVS_H
#define ESP_NVS_H

#include <stdint.h>
#include "esp_err.h"

typedef uint32_t nvs_handle_t;
typedef enum
{
    NVS_READONLY, /*!< Read only */
    NVS_READWRITE /*!< Read and write */
} nvs_open_mode_t;

esp_err_t nvs_flash_init(void);
esp_err_t nvs_flash_erase(void);
esp_err_t nvs_open(const char *namespace_name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle);
esp_err_t nvs_set_i32(nvs_handle_t handle, const char *key, int32_t value);
esp_err_t nvs_commit(nvs_handle_t c_handle);
esp_err_t nvs_get_i32(nvs_handle_t c_handle, const char *key, int32_t *out_value);

#endif