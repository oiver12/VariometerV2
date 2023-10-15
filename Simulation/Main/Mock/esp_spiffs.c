#include "esp_spiffs.h"

esp_err_t esp_vfs_spiffs_register(const esp_vfs_spiffs_conf_t *conf)
{
    return ESP_OK;
}
esp_err_t esp_spiffs_info(const char *partition_label, size_t *total_bytes, size_t *used_bytes)
{
    return ESP_OK;
}
esp_err_t esp_spiffs_format(const char *partition_label)
{
    return ESP_OK;
}
esp_err_t esp_spiffs_check(const char *partition_label)
{
    return ESP_OK;
}