#include "BMI088.h"
#include "driver/gpio.h"
#include "freertos/task.h"

static const char *TAG = "BMI088";

esp_err_t bmi088_init(bmi088_t *dev, spi_device_handle_t acc_spi_handle, gpio_num_t acc_cs_pin,spi_device_handle_t gyro_spi_handle, gpio_num_t gyro_cs_pin)
{
    dev->acc_spi_handle = acc_spi_handle;
    dev->gyro_spi_handle = gyro_spi_handle;
    dev->acc_cs_pin = acc_cs_pin;
    dev->gyro_cs_pin = gyro_cs_pin;
    ESP_ERROR_CHECK(gpio_set_level(acc_cs_pin, 0));
    vTaskDelay(1 / portTICK_PERIOD_MS);
    return ESP_OK;
}