#include "MS5611.h"
#include <driver/spi_master.h>
#include <string.h>
#include "freertos/task.h"
#include <esp_log.h>

static const char *TAG = "MS5611";

#define MS5611_CMD_READ_ADC       0x00
#define MS5611_CMD_RESET          0x1E
#define MS5611_CMD_CONVERT_D1     0x40
#define MS5611_CMD_CONVERT_D2     0x50

#define MS5611_PROM_ADDR_SENS     0xa2
#define MS5611_PROM_ADDR_OFF      0xa4
#define MS5611_PROM_ADDR_TCS      0xa6
#define MS5611_PROM_ADDR_TCO      0xa8
#define MS5611_PROM_ADDR_T_REF    0xaa
#define MS5611_PROM_ADDR_TEMPSENS 0xac

static inline esp_err_t send_command(ms5611_t *dev, const uint8_t cmd)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8;    
    t.tx_buffer=&cmd;            
    t.user=(void*)0;  
    return spi_device_polling_transmit(dev->spi_handle, &t);   
}

static inline esp_err_t read_Bytes(ms5611_t *dev, const uint8_t addr, size_t size, uint8_t *data)
{
    spi_transaction_t transaction;
    memset(&transaction, 0, sizeof(transaction));
    transaction.addr = addr;
    transaction.length = size * 8;
    transaction.rxlength = size * 8;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = data;
    return spi_device_polling_transmit(dev->spi_handle, &transaction);
}

static inline uint16_t shuffle(uint16_t val)
{
    return ((val & 0xff00) >> 8) | ((val & 0xff) << 8);
}

static void wait_conversion(ms5611_osr_t osr)
{
    uint16_t ms = 9;
    switch (osr)
    {
        case MS5611_OSR_256: ms = 1; break;   // 0.5ms
        case MS5611_OSR_512: ms = 2; break;  // 1.1ms
        case MS5611_OSR_1024: ms = 3; break; // 2.1ms
        case MS5611_OSR_2048: ms = 5; break; // 4.1ms
        case MS5611_OSR_4096: ms = 9; break; // 8.22ms
    }
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

static esp_err_t read_adc(ms5611_t *dev, uint32_t *result)
{
    uint8_t tmp[3];
    ESP_ERROR_CHECK(read_Bytes(dev, MS5611_CMD_READ_ADC, 3, tmp));
    *result = (tmp[0] << 16) | (tmp[1] << 8) | (tmp[2]);
    return *result == 0 ? ESP_ERR_INVALID_RESPONSE : ESP_OK;
}

static esp_err_t read_Prom(ms5611_t *dev, uint8_t addr, uint16_t *result)
{
    uint8_t tmp[2];
    ESP_ERROR_CHECK(read_Bytes(dev, addr, 2, tmp));
    *result = (tmp[0] << 8) | tmp[1];
    return *result == 0 ? ESP_ERR_INVALID_RESPONSE : ESP_OK;
}

esp_err_t ms5611_init(ms5611_t *dev, spi_device_handle_t spi_handle, ms5611_osr_t osrPressure, ms5611_osr_t osrTemperature)
{
    dev->spi_handle = spi_handle;
    dev->osrPressure = osrPressure;
    dev->osrTemperature = osrTemperature;
    //reset MS5611
    ESP_ERROR_CHECK(send_command(dev, MS5611_CMD_RESET));
    vTaskDelay(10 / portTICK_PERIOD_MS);
    //read Prom
    ESP_LOGI(TAG, "Read Prom...");
    uint16_t tmp;
    ESP_ERROR_CHECK(read_Prom(dev, MS5611_PROM_ADDR_SENS, &tmp));
    dev->config_data.sens = shuffle(tmp);
    ESP_ERROR_CHECK(read_Prom(dev, MS5611_PROM_ADDR_OFF, &tmp));
    dev->config_data.off = shuffle(tmp);
    ESP_ERROR_CHECK(read_Prom(dev, MS5611_PROM_ADDR_TCS, &tmp));
    dev->config_data.tcs = shuffle(tmp);
    ESP_ERROR_CHECK(read_Prom(dev, MS5611_PROM_ADDR_TCO, &tmp));
    dev->config_data.tco = shuffle(tmp);
    ESP_ERROR_CHECK(read_Prom(dev, MS5611_PROM_ADDR_T_REF, &tmp));
    dev->config_data.t_ref = shuffle(tmp);
    ESP_ERROR_CHECK(read_Prom(dev, MS5611_PROM_ADDR_TEMPSENS, &tmp));
    dev->config_data.tempsens = shuffle(tmp);
    ESP_LOGI(TAG, "Sens: %u, Off: %u, Tcs: %u, Tco: %u, t_ref: %u, tempsens: %u", (unsigned int)dev->config_data.sens, (unsigned int)dev->config_data.off, (unsigned int)dev->config_data.tcs, (unsigned int)dev->config_data.tco, (unsigned int)dev->config_data.t_ref, (unsigned int)dev->config_data.tempsens);
    return ESP_OK;
}

esp_err_t ms5611_get_sensor_data(ms5611_t *dev)
{
    uint32_t raw_pressure = 0;
    uint32_t raw_temperature = 0;
    //get raw Pressure
    ESP_ERROR_CHECK(send_command(dev, MS5611_CMD_CONVERT_D1 + dev->osrPressure));
    wait_conversion(dev->osrPressure);
    ESP_ERROR_CHECK(read_adc(dev, &raw_pressure));
    //get raw temperature
    ESP_ERROR_CHECK(send_command(dev, MS5611_CMD_CONVERT_D1 + dev->osrTemperature));
    wait_conversion(dev->osrTemperature);
    ESP_ERROR_CHECK(read_adc(dev, &raw_temperature));
    //compensate Pressure
    int32_t dt = raw_temperature - ((int32_t)dev->config_data.t_ref << 8);
    int64_t temp = (2000 + (int64_t)dt * dev->config_data.tempsens / 8388608);
    int64_t off = (int64_t)((int64_t)dev->config_data.off * 65536)+ (((int64_t)dev->config_data.tco * dt) / 128);
    int64_t sens = (int64_t)(((int64_t)dev->config_data.sens) * 32768)+ (((int64_t)dev->config_data.tcs * dt) / 256);
    // Set defaults for temp >= 2000
    int64_t t_2 = 0;
    int64_t off_2 = 0;
    int64_t sens_2 = 0;
    int64_t help = 0;
    if (temp < 2000)
    {
        // Low temperature
        t_2 = ((dt * dt) >> 31); // T2 = dT^2/2^31
        help = (temp - 2000);
        help = 5 * help * help;
        off_2 = help >> 1;       // OFF_2  = 5 * (TEMP - 2000)^2/2^1
        sens_2 = help >> 2;      // SENS_2 = 5 * (TEMP - 2000)^2/2^2
        if (temp < -1500)
        {
            // Very low temperature
            help = (temp + 1500);
            help = help * help;
            off_2 = off_2 + 7 * help;             // OFF_2  = OFF_2 + 7 * (TEMP + 1500)^2
            sens_2 = sens_2 + ((11 * help) >> 1); // SENS_2 = SENS_2 + 7 * (TEMP + 1500)^2/2^1
        }
    }
    temp = temp - t_2;
    off = off - off_2;
    sens = sens - sens_2;
    // Temperature compensated pressure (10...1200mbar with 0.01mbar resolution
    // P = digital pressure value  * SENS - OFF = (D1 * SENS/2^21 -OFF)/2^15
    dev->Pressure = (int32_t)(((int64_t)raw_pressure * (sens / 0x200000) - off) / 32768);
    dev->Temperature = (float)temp / 100.0;
    return ESP_OK;
}

