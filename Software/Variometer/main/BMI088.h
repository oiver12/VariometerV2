#ifndef BMI088_SPI_DRIVER_H
#define BMI088_SPI_DRIVER_H

#include <driver/spi_master.h>
#include <esp_err.h>
#include "driver/gpio.h"

typedef enum {
    ACCEL_RANGE_3G,
    ACCEL_RANGE_6G,
    ACCEL_RANGE_12G,
    ACCEL_RANGE_24G
} bmi088_accelRange_t;

typedef enum{
    GYRO_RANGE_2000DPS,
    GYRO_RANGE_1000DPS,
    GYRO_RANGE_500DPS,
    GYRO_RANGE_250DPS,
    GYRO_RANGE_125DPS
} bmi088_gyroRange_t;

typedef enum{
    ODR_2000HZ,
    ODR_1000HZ,
    ODR_400HZ
} bmi088_odr_t;

typedef struct {
    spi_device_handle_t acc_spi_handle;
    spi_device_handle_t gyro_spi_handle;
    gpio_num_t acc_cs_pin;
    gpio_num_t gyro_cs_pin;
    bmi088_accelRange_t accelRange;
    bmi088_gyroRange_t gyroRange;
    bmi088_odr_t odr;
} bmi088_t;

esp_err_t bmi088_init(bmi088_t *dev, spi_device_handle_t acc_spi_handle, gpio_num_t acc_cs_pin,spi_device_handle_t gyro_spi_handle, gpio_num_t gyro_cs_pin);
esp_err_t bmi088_setOdr(bmi088_t *dev, bmi088_odr_t odr);
esp_err_t bmi088_setRange(bmi088_t *dev, bmi088_accelRange_t accelRange, bmi088_gyroRange_t gyroRange);
esp_err_t bmi088_enableDataReady(bmi088_t *dev);


#endif