#include "BatteryManager.h"
#include "driver/gpio.h"
#include <esp_log.h>

#define BAT_MS 9
static const char *TAG = "Battery Manager";

static bool adc_calibration_init(Battery_Manager_t* manager)
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
        esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, manager->adc_chars);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return cali_enable;
}

esp_err_t init_Battery_Manager(Battery_Manager_t* manager, int oversampling)
{
    manager->adc_chars = malloc(sizeof(esp_adc_cal_characteristics_t));
    manager->oversampling = oversampling;
    manager->cali_enable = adc_calibration_init(manager);
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
    return ESP_OK;
}

esp_err_t deinit_Battery_Manager(Battery_Manager_t* manager)
{
    free(manager->adc_chars);
    manager->adc_chars = NULL;
    return ESP_OK;
}

esp_err_t readPercentage_Battery(Battery_Manager_t* manager)
{
    gpio_set_level(BAT_MS, 1);
    int sumValue = 0;
    for (size_t i = 0; i < manager->oversampling; i++)
    {
        int tempValue = 0;
        ESP_ERROR_CHECK(adc2_get_raw(ADC2_CHANNEL_3, ADC_WIDTH_BIT_DEFAULT, &tempValue));
        sumValue += tempValue;
    }
    sumValue /= manager->oversampling;
    if(manager->cali_enable)
    {
        sumValue = esp_adc_cal_raw_to_voltage(sumValue, manager->adc_chars);
        manager->voltage = ((float)sumValue / 1000.0f) * (1.0f + (10000.0f / 33000.0f));
        ESP_LOGI(TAG, "Voltage: %.2f", manager->voltage);
    }
    else{
        return ESP_ERR_INVALID_RESPONSE;
    }
    gpio_set_level(BAT_MS, 0);
    return ESP_OK;
}