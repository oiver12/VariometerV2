#include "BatteryManager.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "DeviceContext.h"

#define BAT_MS 3

#define maxVoltage 4 //nicht 4.2, weil der ESP nur bis 3.1V und nicht bis 3.3v lesen kann
#define minVoltage 3.3
static const char *TAG = "Battery Manager";

Battery_Manager_t battery_Manager;

static bool adc_calibration_init()
{
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP_FIT);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (ret == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (ret == ESP_OK) {
        cali_enable = true;
        esp_adc_cal_value_t temp = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, battery_Manager.adc_chars);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return cali_enable;
}

esp_err_t deinit_Battery_Manager()
{
    free(battery_Manager.adc_chars);
    battery_Manager.adc_chars = NULL;
    return ESP_OK;
}

esp_err_t readPercentage_Battery()
{
    gpio_set_level(BAT_MS, 1);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    uint sumValue = 0;
    for (size_t i = 0; i < battery_Manager.oversampling; i++)
    {
        int tempValue = 0;
        ESP_ERROR_CHECK(adc2_get_raw(ADC1_CHANNEL_7, ADC_WIDTH_BIT_DEFAULT, &tempValue));
        sumValue += tempValue;
    }
    sumValue /= battery_Manager.oversampling;
    if(battery_Manager.cali_enable)
    {
        sumValue = esp_adc_cal_raw_to_voltage(sumValue,battery_Manager.adc_chars);
        //ESP_LOGI(TAG, "SumValue: %i", sumValue);
        float voltageBattery = ((float)sumValue / 1000.0f) * (1.0f + (10000.0f / 33000.0f));
        if(voltageBattery > 3.0f && voltageBattery < 4.2f)
        {
            battery_Manager.voltage = voltageBattery;
        }
        //ESP_LOGI(TAG, "Voltage: %.2f", battery_Manager.voltage);
    }
    else{
        return ESP_ERR_INVALID_RESPONSE;
    }
    gpio_set_level(BAT_MS, 0);
    return ESP_OK;
}

static void BatteryManagerTask(void *parameters)
{
    while (1)
    {
        readPercentage_Battery();
        //float percentage = (battery_Manager.voltage - 3.2f) * 100.0f;
        float percentage = (battery_Manager.voltage - minVoltage) * 100.0f / (maxVoltage - minVoltage);
        if(percentage > 100.0f)
            percentage = 100.0f;
        else if(percentage < 0.0f)
            percentage = 0.0f;
        device_state_set_member(&percentage, device_state_batteryPercentage);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

esp_err_t init_Battery_Manager(int oversampling)
{
    battery_Manager.adc_chars = malloc(sizeof(esp_adc_cal_characteristics_t));
    battery_Manager.oversampling = oversampling;
    battery_Manager.cali_enable = adc_calibration_init(&battery_Manager);
    //ADC1 config
    ESP_ERROR_CHECK(adc2_config_channel_atten(ADC2_CHANNEL_3, ADC_ATTEN_DB_11));
    gpio_config_t gpio_config_t = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << BAT_MS,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&gpio_config_t);
    gpio_set_level(BAT_MS, 0);
    xTaskCreatePinnedToCore(BatteryManagerTask, "Battery", 4 * 1024, NULL, 5, NULL, 0);
    return ESP_OK;
}