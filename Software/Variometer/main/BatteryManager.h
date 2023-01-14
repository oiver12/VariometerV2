#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include "driver/adc.h"
#include "esp_adc_cal.h"

typedef struct{
    int oversampling;
    float voltage;
    float percentage;
    esp_adc_cal_characteristics_t* adc_chars;
    bool cali_enable;
} Battery_Manager_t;

esp_err_t init_Battery_Manager(int oversampling);
esp_err_t deinit_Battery_Manager();

#endif