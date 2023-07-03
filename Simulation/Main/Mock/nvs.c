#include "nvs.h"

esp_err_t nvs_flash_init(void)
{
    return ESP_OK;
}
esp_err_t nvs_flash_erase(void)
{
    return ESP_OK;
}
esp_err_t nvs_open(const char *namespace_name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle)
{
    return ESP_OK;
}
esp_err_t nvs_set_i32(nvs_handle_t handle, const char *key, int32_t value)
{
    return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t c_handle)
{
    return ESP_OK;
}
esp_err_t nvs_get_i32(nvs_handle_t c_handle, const char *key, int32_t *out_value)
{
    return ESP_OK;
}