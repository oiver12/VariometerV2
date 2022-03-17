#ifndef MS5611_SPI_DRIVER_H
#define MS5611_SPI_DRIVER_H

#include <driver/spi_master.h>
#include <esp_err.h>
#include "freertos/semphr.h"

#define USEI2C

//https://github.com/UncleRus/esp-idf-lib/blob/master/components/ms5611/ms5611.c

typedef enum
{
    MS5611_OSR_256  = 0x00, //!< 256 samples per measurement
    MS5611_OSR_512  = 0x02, //!< 512 samples per measurement
    MS5611_OSR_1024 = 0x04, //!< 1024 samples per measurement
    MS5611_OSR_2048 = 0x06, //!< 2048 samples per measurement
    MS5611_OSR_4096 = 0x08  //!< 4096 samples per measurement
} ms5611_osr_t;

/**
 * Configuration data
 */
typedef struct
{
    uint16_t sens;       //!< C1 Pressure sensitivity                             | SENS_t1
    uint16_t off;        //!< C2 Pressure offset                                  | OFF_t1
    uint16_t tcs;        //!< C3 Temperature coefficient of pressure sensitivity  | TCS
    uint16_t tco;        //!< C4 Temperature coefficient of pressure offset       | TCO
    uint16_t t_ref;      //!< C5 Reference temperature                            | T_ref
    uint16_t tempsens;   //!< C6 Temperature coefficient of the temperature       | TEMPSENSE
} ms5611_config_data_t;

typedef struct 
{
    #ifndef USEI2C
    spi_device_handle_t spi_handle;
    #else
    SemaphoreHandle_t *i2c_Mutex;
    int I2C_Master_Num;
    uint8_t ms5611_i2c_addr;
    #endif
    ms5611_osr_t osrPressure;
    ms5611_osr_t osrTemperature;
    ms5611_config_data_t config_data;
    float Pressure;
    float Temperature;
} ms5611_t;

#ifndef USEI2C
esp_err_t ms5611_init(ms5611_t *dev, spi_device_handle_t spi_handle, ms5611_osr_t osrPressure, ms5611_osr_t osrTemperature);
#else
esp_err_t ms5611_init(ms5611_t *dev, SemaphoreHandle_t *i2c_Mutex, int I2C_Master_Num, uint8_t ms5611_i2c_addr, ms5611_osr_t osrPressure, ms5611_osr_t osrTemperature);
#endif
esp_err_t ms5611_get_sensor_data(ms5611_t *dev);


#endif