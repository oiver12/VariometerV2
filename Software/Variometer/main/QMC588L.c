#include "QMC588L.h"
#include "driver/i2c.h"
#include <esp_log.h>

#define I2C_MASTER_TIMEOUT_MS       1000

static const char *TAG = "QMC588L";

static inline esp_err_t writeByte(qmc588l_t *dev, uint8_t regAddr, const uint8_t data)
{
    esp_err_t err = ESP_ERR_TIMEOUT;
    uint8_t write_buf[2] = {regAddr, data};
    err = i2c_master_write_to_device(dev->I2C_Master_Num, dev->qmc588l_i2c_addr, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return err;
}

static inline esp_err_t read_Bytes(qmc588l_t *dev, const uint8_t addr, size_t size, uint8_t *data)
{
    esp_err_t err = ESP_ERR_TIMEOUT;
    err = i2c_master_write_read_device(dev->I2C_Master_Num, dev->qmc588l_i2c_addr, &addr, 1, data, size, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return err;
}

esp_err_t qmc588l_init(qmc588l_t *dev, int I2C_Master_Num, uint8_t i2c_addr, qmc588l_mr_t mr, qmc588l_osr_t osr, qmc588l_scale_t scale)
{
    dev->I2C_Master_Num = I2C_Master_Num;
    dev->qmc588l_i2c_addr = i2c_addr;
    ESP_ERROR_CHECK(writeByte(dev, 0x0B, 0x01)); /* Set Reset Period*/
    ESP_ERROR_CHECK(qmc588l_setMode(dev, mr, osr, scale)); /* Set Mode to continous */
    ESP_ERROR_CHECK(writeByte(dev, 0x0A, 0x41)); /* Enable Pointer Roll Over and disable Interrupt */
    dev->useCalibration = false;
    return ESP_OK;
}

esp_err_t qmc588l_setMode(qmc588l_t *dev, qmc588l_mr_t mr, qmc588l_osr_t osr, qmc588l_scale_t scale)
{
    dev->measurement_rate = mr;
    dev->oversampling = osr;
    dev->scale = scale;
    return writeByte(dev, 0x09, 0x01 | mr | osr | scale);
}

esp_err_t qmc588l_setCalibration(qmc588l_t *dev, int x_min, int x_max, int y_min, int y_max, int z_min, int z_max)
{
    dev->useCalibration = true;
    dev->_vCalibration[0][0] = x_min;
	dev->_vCalibration[0][1] = x_max;
	dev->_vCalibration[1][0] = y_min;
	dev->_vCalibration[1][1] = y_max;
	dev->_vCalibration[2][0] = z_min;
	dev->_vCalibration[2][1] = z_max;
    return ESP_OK;
}

esp_err_t qmc588l_calculateCalibration(qmc588l_t *dev)
{
    ESP_LOGI(TAG, "Calibration of QMC588L will start in 2 Seconds. \n Please move your Board in all Directions");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    bool changed = false;
    int64_t changedTime = 0;
    int64_t lastTime = 0;
    while((lastTime - changedTime) > 5000)
    {
        qmc588l_readMag(dev);
        changed = false;
        if(dev->magnValue[0] < dev->_vCalibration[0][0])
        {
            dev->_vCalibration[0][0] = dev->magnValue[0];
            changed = true;
        }
        if(dev->magnValue[0] > dev->_vCalibration[0][1])
        {
            dev->_vCalibration[0][1] = dev->magnValue[0];
            changed = true;
        }
        if(dev->magnValue[1] < dev->_vCalibration[1][0])
        {
            dev->_vCalibration[1][0] = dev->magnValue[1];
            changed = true;
        }
        if(dev->magnValue[1] > dev->_vCalibration[1][1])
        {
            dev->_vCalibration[1][1] = dev->magnValue[1];
            changed = true;
        }
        if(dev->magnValue[2] < dev->_vCalibration[2][0])
        {
            dev->_vCalibration[2][0] = dev->magnValue[2];
            changed = true;
        }
        if(dev->magnValue[2] > dev->_vCalibration[2][1])
        {
            dev->_vCalibration[2][1] = dev->magnValue[2];
            changed = true;
        }
        if(changed)
        {
            ESP_LOGI(TAG, "Calibrating... Keep moving sensor around");
            changedTime = esp_timer_get_time();
        }
        lastTime = esp_timer_get_time();
    }
    ESP_LOGI(TAG, "DONE. The Values: \r\n"
            "%i, %i, %i, %i, %i, %i",
            dev->_vCalibration[0][0], dev->_vCalibration[0][1], dev->_vCalibration[1][0], dev->_vCalibration[1][1], dev->_vCalibration[2][0], dev->_vCalibration[2][1]);
    dev->useCalibration = true; 
    return ESP_OK;
}

esp_err_t qmc588l_readMag(qmc588l_t *dev)
{
    uint8_t buffer[6] = {0};
    ESP_ERROR_CHECK(read_Bytes(dev, 0x00, 6, buffer));
    dev->magnValue[0] = (int16_t)(buffer[1] << 8 | buffer[0]);
    dev->magnValue[1] = (int16_t)(buffer[3] << 8 | buffer[2]);
    dev->magnValue[2] = (int16_t)(buffer[5] << 8 | buffer[4]);
    if(dev->useCalibration)
    {
        int x_offset = (dev->_vCalibration[0][0] + dev->_vCalibration[0][1])/2;
        int y_offset = (dev->_vCalibration[1][0] + dev->_vCalibration[1][1])/2;
        int z_offset = (dev->_vCalibration[2][0] + dev->_vCalibration[2][1])/2;
        int x_avg_delta = (dev->_vCalibration[0][1] - dev->_vCalibration[0][0])/2;
        int y_avg_delta = (dev->_vCalibration[1][1] - dev->_vCalibration[1][0])/2;
        int z_avg_delta = (dev->_vCalibration[2][1] - dev->_vCalibration[2][0])/2;

        int avg_delta = (x_avg_delta + y_avg_delta + z_avg_delta) / 3;

        float x_scale = (float)avg_delta / x_avg_delta;
        float y_scale = (float)avg_delta / y_avg_delta;
        float z_scale = (float)avg_delta / z_avg_delta;

        dev->magnValue[0] = (dev->magnValue[0] - x_offset) * x_scale;
        dev->magnValue[1] = (dev->magnValue[1] - y_offset) * y_scale;
        dev->magnValue[2] = (dev->magnValue[2] - z_offset) * z_scale;
    }
    return ESP_OK;
}