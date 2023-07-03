#include "i2c.h"

// TODO: also play tone in Simulation, when switched to new i2s driver

esp_err_t i2s_write(i2s_port_t i2s_num, const void *src, size_t size, size_t *bytes_written, TickType_t ticks_to_wait)
{
    return ESP_OK;
}
esp_err_t i2s_driver_install(i2s_port_t i2s_num, const i2s_config_t *i2s_config, int queue_size, void *i2s_queue)
{
    return ESP_OK;
}
esp_err_t i2s_set_pin(i2s_port_t i2s_num, const i2s_pin_config_t *pin)
{
    return ESP_OK;
}