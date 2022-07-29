#include "MS5611.h"
#include "driver/i2c.h"
#include <driver/spi_master.h>
#include <string.h>
#include "freertos/task.h"
#include <esp_log.h>
#include "freertos/semphr.h"
#include <math.h> 

#define I2C_MASTER_TIMEOUT_MS       1000

static const char *TAG = "MS5611";

#define MS5611_CMD_READ_ADC       0x00
#define MS5611_CMD_RESET          0x1E
#define MS5611_CMD_CONVERT_D1     0x40
#define MS5611_CMD_CONVERT_D2     0x50

#define MS5611_PROM_ADDR          0xa0
// #define MS5611_PROM_ADDR_OFF      0xa4
// #define MS5611_PROM_ADDR_TCS      0xa6
// #define MS5611_PROM_ADDR_TCO      0xa8
// #define MS5611_PROM_ADDR_T_REF    0xaa
// #define MS5611_PROM_ADDR_TEMPSENS 0xac

static inline esp_err_t send_command(ms5611_t *dev, uint8_t command)
{
    #ifndef MS5611_I2C
    spi_transaction_t transaction;
    transaction.cmd = cmd;
    transaction.addr = 0;
    transaction.length = 0;
    transaction.rxlength = 0;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = NULL;
    return spi_device_polling_transmit(dev->spi_handle, &transaction);   
    #else
    esp_err_t err = ESP_ERR_TIMEOUT;
    err = i2c_master_write_to_device(dev->I2C_Master_Num, dev->ms5611_i2c_addr, &command, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return err;
    // i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, dev->ms5611_i2c_addr << 1, true);
    // i2c_master_write(cmd, &command, 1, true);
    // i2c_master_stop(cmd);
    // esp_err_t res = i2c_master_cmd_begin(dev->I2C_Master_Num, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    // if (res != ESP_OK)
    //     ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", dev->ms5611_i2c_addr, dev->I2C_Master_Num, res, esp_err_to_name(res));
    // i2c_cmd_link_delete(cmd);
    // return res;
    #endif
}

static inline esp_err_t read_Bytes(ms5611_t *dev, const uint8_t addr, size_t size, uint8_t *data)
{
    #ifndef MS5611_I2C
    spi_transaction_t transaction;
    transaction.cmd = 0;
    transaction.addr = addr;
    transaction.length = size * 8;
    transaction.rxlength = size * 8;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = data;
    return spi_device_polling_transmit(dev->spi_handle, &transaction);
    #else
    esp_err_t err = ESP_ERR_TIMEOUT;
    err = i2c_master_write_read_device(dev->I2C_Master_Num, dev->ms5611_i2c_addr, &addr, 1, data, size, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return err;
    #endif
}

static void wait_conversion(ms5611_osr_t osr)
{
    //TODO: vTaskDelay nur mind. 10 ms möglich zum warten. Messung jedoch nur 0.5 ms --> auch busy waiting mäöglich mit ets_delay_us(us). Ist es sinnvoll?
    // volatile uint32_t ms = 9;
    // switch (osr)
    // {
    //     case MS5611_OSR_256: ms = 1; break;   // 0.5ms
    //     case MS5611_OSR_512: ms = 2; break;  // 1.1ms
    //     case MS5611_OSR_1024: ms = 3; break; // 2.1ms
    //     case MS5611_OSR_2048: ms = 5; break; // 4.1ms
    //     case MS5611_OSR_4096: ms = 9; break; // 8.22ms
    // }
    vTaskDelay(1);
    //ets_delay_us(1100);
}

static esp_err_t read_adc(ms5611_t *dev, uint32_t *result)
{
    uint8_t tmp[3];
    ESP_ERROR_CHECK(read_Bytes(dev, MS5611_CMD_READ_ADC, 3, tmp));
    *result = ((uint32_t)tmp[0] << 16) | ((uint32_t)tmp[1] << 8) | (tmp[2]);
    return *result == 0 ? ESP_ERR_INVALID_RESPONSE : ESP_OK;
}

static esp_err_t read_Prom(ms5611_t *dev, uint8_t addr, uint16_t *result)
{
    uint8_t tmp[2];
    ESP_ERROR_CHECK(read_Bytes(dev, addr, 2, tmp));
    *result = (tmp[0] << 8) | tmp[1];
    return ESP_OK;
    //return *result == 0 ? ESP_ERR_INVALID_RESPONSE : ESP_OK;
}
unsigned char crc4(unsigned int n_prom[])
{
    int cnt; // simple counter
    unsigned int n_rem; // crc reminder
    unsigned int crc_read; // original value of the crc
    unsigned char n_bit;
    n_rem = 0x00;
    crc_read=n_prom[7]; //save read CRC
    n_prom[7]=(0xFF00 & (n_prom[7])); //CRC byte is replaced by 0
    for (cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
    {// choose LSB or MSB
        if (cnt%2==1) n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
        else n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);
        for (n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & (0x8000))
            {
                n_rem = (n_rem << 1) ^ 0x3000;
            }
            else
            {
                n_rem = (n_rem << 1);
            }
        }
    }
    n_rem= (0x000F & (n_rem >> 12)); // final 4-bit reminder is CRC code
    n_prom[7]=crc_read; // restore the crc_read to its original place
    return (n_rem ^ 0x0);
} 
#ifndef MS5611_I2C
esp_err_t ms5611_init(ms5611_t *dev, spi_device_handle_t spi_handle, ms5611_osr_t osrPressure, ms5611_osr_t osrTemperature)
{
    dev->spi_handle = spi_handle;
#else
esp_err_t ms5611_init(ms5611_t *dev, int I2C_Master_Num, uint8_t ms5611_i2c_addr, ms5611_osr_t osrPressure, ms5611_osr_t osrTemperature)
{
    dev->I2C_Master_Num = I2C_Master_Num;
    dev->ms5611_i2c_addr = ms5611_i2c_addr;
    #endif
    dev->osrPressure = osrPressure;
    dev->osrTemperature = osrTemperature;
    //reset MS5611
    ESP_ERROR_CHECK(send_command(dev, MS5611_CMD_RESET));
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //read Prom
    ESP_LOGI(TAG, "Read Prom...");
    uint16_t tmp;
    unsigned int n_prom[8];
    for (size_t i = 0; i < 8; i++)
    {
        ESP_ERROR_CHECK(read_Prom(dev, MS5611_PROM_ADDR + i*2, &tmp));
        n_prom[i] = (unsigned int)tmp;
    }
    dev->config_data.sens = (uint64_t)(((uint64_t)n_prom[1]) * 32768);
    dev->config_data.off = (uint64_t)((uint64_t)n_prom[2] * 65536);
    dev->config_data.tcs = (uint64_t)n_prom[3];
    dev->config_data.tco = (uint64_t)n_prom[4];
    dev->config_data.t_ref = ((uint32_t)n_prom[5] << 8);
    dev->config_data.tempsens = (uint64_t)n_prom[6];
    if(crc4(n_prom) != n_prom[7])
    {
        ESP_LOGI(TAG, "CRC was worong!");
        return ESP_ERR_INVALID_CRC;
    }
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
    read_adc(dev, &raw_pressure);
    //get raw temperature
    ESP_ERROR_CHECK(send_command(dev, MS5611_CMD_CONVERT_D2 + dev->osrTemperature));
    wait_conversion(dev->osrTemperature);
    read_adc(dev, &raw_temperature);
    dev->lastRawTemp = raw_temperature;
    //compensate Pressure
    uint32_t dt = raw_temperature - dev->config_data.t_ref;
    uint64_t temp = (2000 + (uint64_t)dt * dev->config_data.tempsens / 8388608);
    uint64_t off = dev->config_data.off + ((dev->config_data.tco * dt) / 128);
    uint64_t sens = dev->config_data.sens+ ((dev->config_data.tcs * dt) / 256);
    // Set defaults for temp >= 2000
    uint64_t t_2 = 0;
    uint64_t off_2 = 0;
    uint64_t sens_2 = 0;
    uint64_t help = 0;
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
    dev->Pressure = (double)(((uint64_t)raw_pressure * (sens / 0x200000) - off) / 32768);
    dev->Temperature = (double)temp / 100.0;
    return ESP_OK;
}

esp_err_t ms5611_get_pressure(ms5611_t *dev)
{
    uint32_t raw_pressure = 0;
    int64_t temp = dev->Temperature * 100.0;
    ESP_ERROR_CHECK(send_command(dev, (uint8_t)0x40 + dev->osrPressure));
    wait_conversion(dev->osrPressure);
    ESP_ERROR_CHECK(read_adc(dev, &raw_pressure));

    int32_t dt = dev->lastRawTemp - dev->config_data.t_ref;
    int64_t off = dev->config_data.off + (((int64_t)dev->config_data.tco * dt) / 128);
    int64_t sens = dev->config_data.sens+ (((int64_t)dev->config_data.tcs * dt) / 256);
    // Set defaults for temp >= 2000
    int64_t off_2 = 0;
    int64_t sens_2 = 0;
    int64_t help = 0;
    if (temp < 2000)
    {
        // Low temperature
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
    off = off - off_2;
    sens = sens - sens_2;
    dev->Pressure = (int32_t)(((int64_t)raw_pressure * (sens / 0x200000) - off) / 32768);
    return ESP_OK;
}

