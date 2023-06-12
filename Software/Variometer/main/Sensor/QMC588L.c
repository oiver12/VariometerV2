#include "QMC588L.h"
#include "driver/i2c.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Screen/ScreenManager.h"
#include "Flash/flashMemory.h"
#include "esp_timer.h"

#define I2C_MASTER_TIMEOUT_MS 1000

// float offsetVector[3] = { 0, 0, 0};
// float magneticMapping[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1};
float offsetVector[3] = {-15.06f, -6.4f, 3.94f};
float magneticMapping[9] = {1.003f, -0.011f, 0.0f, -0.011f, 0.997f, 0.009f, 0.00f, 0.009f, 1.00f};

qmc588l_t qmc588l_data;

static const char *TAG = "QMC588L";

static inline esp_err_t writeByte(uint8_t regAddr, const uint8_t data)
{
    esp_err_t err = ESP_ERR_TIMEOUT;
    uint8_t write_buf[2] = {regAddr, data};
    err = i2c_master_write_to_device(qmc588l_data.I2C_Master_Num, qmc588l_data.qmc588l_i2c_addr, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return err;
}

static inline esp_err_t read_Bytes(const uint8_t addr, size_t size, uint8_t *data)
{
    esp_err_t err = ESP_ERR_TIMEOUT;
    err = i2c_master_write_read_device(qmc588l_data.I2C_Master_Num, qmc588l_data.qmc588l_i2c_addr, &addr, 1, data, size, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return err;
}

qmc588l_t *qmc588l_init(int I2C_Master_Num, uint8_t i2c_addr, qmc588l_mr_t mr, qmc588l_osr_t osr, qmc588l_scale_t scale)
{
    qmc588l_data.I2C_Master_Num = I2C_Master_Num;
    qmc588l_data.qmc588l_i2c_addr = i2c_addr;
    ESP_ERROR_CHECK(writeByte(0x0B, 0x01));           /* Set Reset Period*/
    ESP_ERROR_CHECK(qmc588l_setMode(mr, osr, scale)); /* Set Mode to continous */
    ESP_ERROR_CHECK(writeByte(0x0A, 0x41));           /* Enable Pointer Roll Over and disable Interrupt */
    qmc588l_data.useCalibration = false;
    qmc588l_data.isCalibrating = false;
    return &qmc588l_data;
}

esp_err_t qmc588l_setMode(qmc588l_mr_t mr, qmc588l_osr_t osr, qmc588l_scale_t scale)
{
    qmc588l_data.measurement_rate = mr;
    qmc588l_data.oversampling = osr;
    qmc588l_data.scale = scale;
    return writeByte(0x09, 0x01 | mr | osr | scale);
}

esp_err_t qmc588l_setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max)
{
    qmc588l_data.useCalibration = true;
    qmc588l_data._vCalibration[0][0] = x_min;
    qmc588l_data._vCalibration[0][1] = x_max;
    qmc588l_data._vCalibration[1][0] = y_min;
    qmc588l_data._vCalibration[1][1] = y_max;
    qmc588l_data._vCalibration[2][0] = z_min;
    qmc588l_data._vCalibration[2][1] = z_max;
    return ESP_OK;
}

static void calibrationTask(void *params)
{
    ESP_LOGI(TAG, "Start Calibration");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    qmc588l_data.isCalibrating = true;
    bool changed = false;
    int64_t changedTime = esp_timer_get_time();
    int64_t lastTime = esp_timer_get_time();
    while ((lastTime - changedTime) < 10000000)
    {
        qmc588l_readMag();
        changed = false;
        if (qmc588l_data.rawMagnValue[0] < qmc588l_data._vCalibration[0][0])
        {
            qmc588l_data._vCalibration[0][0] = qmc588l_data.rawMagnValue[0];
            changed = true;
        }
        if (qmc588l_data.rawMagnValue[0] > qmc588l_data._vCalibration[0][1])
        {
            qmc588l_data._vCalibration[0][1] = qmc588l_data.rawMagnValue[0];
            changed = true;
        }
        if (qmc588l_data.rawMagnValue[1] < qmc588l_data._vCalibration[1][0])
        {
            qmc588l_data._vCalibration[1][0] = qmc588l_data.rawMagnValue[1];
            changed = true;
        }
        if (qmc588l_data.rawMagnValue[1] > qmc588l_data._vCalibration[1][1])
        {
            qmc588l_data._vCalibration[1][1] = qmc588l_data.rawMagnValue[1];
            changed = true;
        }
        if (qmc588l_data.rawMagnValue[2] < qmc588l_data._vCalibration[2][0])
        {
            qmc588l_data._vCalibration[2][0] = qmc588l_data.rawMagnValue[2];
            changed = true;
        }
        if (qmc588l_data.rawMagnValue[2] > qmc588l_data._vCalibration[2][1])
        {
            qmc588l_data._vCalibration[2][1] = qmc588l_data.rawMagnValue[2];
            changed = true;
        }
        if (changed)
        {
            // ESP_LOGI(TAG, "Calibrating... Keep moving sensor around");
            changedTime = esp_timer_get_time();
        }
        lastTime = esp_timer_get_time();
        portYIELD();
    }
    ESP_LOGI(TAG, "DONE. The Values: \r\n"
                  "%i, %i, %i, %i, %i, %i",
             qmc588l_data._vCalibration[0][0], qmc588l_data._vCalibration[0][1], qmc588l_data._vCalibration[1][0], qmc588l_data._vCalibration[1][1], qmc588l_data._vCalibration[2][0], qmc588l_data._vCalibration[2][1]);
    qmc588l_data.useCalibration = true;
    qmc588l_data.isCalibrating = false;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    flashMemoryWriteInt32("MagCaliMinX", qmc588l_data._vCalibration[0][0]);
    flashMemoryWriteInt32("MagCaliMaxX", qmc588l_data._vCalibration[0][1]);
    flashMemoryWriteInt32("MagCaliMinY", qmc588l_data._vCalibration[1][0]);
    flashMemoryWriteInt32("MagCaliMaxY", qmc588l_data._vCalibration[1][1]);
    flashMemoryWriteInt32("MagCaliMinZ", qmc588l_data._vCalibration[2][0]);
    flashMemoryWriteInt32("MagCaliMaxZ", qmc588l_data._vCalibration[2][1]);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ScreenManager_deselctSettingsWidget();
    vTaskDelete(NULL);
}

void qmc588l_calculateCalibration()
{
    xTaskCreate(calibrationTask, "calibrationTask", 1024 * 3, NULL, 5, NULL);
}

static float convertTomG(int16_t value)
{
    if (qmc588l_data.scale == QMC588L_SCALE_2G)
    {
        return ((float)value * 2.0f * 1000.0f) / 32768.0f;
    }
    return ((float)value * 8.0f * 1000.0f) / 32768.0f;
}

esp_err_t qmc588l_readMag()
{
    if (qmc588l_data.isCalibrating)
    {
        return ESP_OK;
    }
    uint8_t buffer[6] = {0};
    ESP_ERROR_CHECK(read_Bytes(0x00, 6, buffer));
    qmc588l_data.rawMagnValue[0] = (int16_t)(buffer[1] << 8 | buffer[0]);
    qmc588l_data.rawMagnValue[1] = (int16_t)(buffer[3] << 8 | buffer[2]);
    qmc588l_data.rawMagnValue[2] = (int16_t)(buffer[5] << 8 | buffer[4]);
    float x = convertTomG(qmc588l_data.rawMagnValue[0]) - offsetVector[0];
    float y = convertTomG(qmc588l_data.rawMagnValue[1]) - offsetVector[1];
    float z = convertTomG(qmc588l_data.rawMagnValue[2]) - offsetVector[2];
    qmc588l_data.CorrectedMagnValuemG[0] = x * magneticMapping[0] + y * magneticMapping[1] + z * magneticMapping[2];
    qmc588l_data.CorrectedMagnValuemG[1] = x * magneticMapping[3] + y * magneticMapping[4] + z * magneticMapping[5];
    qmc588l_data.CorrectedMagnValuemG[2] = x * magneticMapping[6] + y * magneticMapping[7] + z * magneticMapping[8];
    // Was used with old calibration
    //  if(qmc588l_data.useCalibration)
    //  {
    //      int x_offset = (qmc588l_data._vCalibration[0][0] + qmc588l_data._vCalibration[0][1])/2;
    //      int y_offset = (qmc588l_data._vCalibration[1][0] + qmc588l_data._vCalibration[1][1])/2;
    //      int z_offset = (qmc588l_data._vCalibration[2][0] + qmc588l_data._vCalibration[2][1])/2;

    //     int x_avg_delta = (qmc588l_data._vCalibration[0][1] - qmc588l_data._vCalibration[0][0])/2;
    //     int y_avg_delta = (qmc588l_data._vCalibration[1][1] - qmc588l_data._vCalibration[1][0])/2;
    //     int z_avg_delta = (qmc588l_data._vCalibration[2][1] - qmc588l_data._vCalibration[2][0])/2;

    //     float avg_delta = (x_avg_delta + y_avg_delta + z_avg_delta) / 3.0f;

    //     float x_scale = avg_delta / (float)x_avg_delta;
    //     float y_scale = avg_delta / (float)y_avg_delta;
    //     float z_scale = avg_delta / (float)z_avg_delta;

    //     qmc588l_data.rawMagnValue[0] = (qmc588l_data.rawMagnValue[0] - x_offset) * x_scale;
    //     qmc588l_data.rawMagnValue[1] = (qmc588l_data.rawMagnValue[1] - y_offset) * y_scale;
    //     qmc588l_data.rawMagnValue[2] = (qmc588l_data.rawMagnValue[2] - z_offset) * z_scale;
    //     float x = convertTomG(qmc588l_data.rawMagnValue[0]) - offsetVector[0];
    //     float y = convertTomG(qmc588l_data.rawMagnValue[1]) - offsetVector[1];
    //     float z = convertTomG(qmc588l_data.rawMagnValue[2]) - offsetVector[2];
    //     // qmc588l_data.CorrectedMagnValuemG[0] = x * magneticMapping[0] + y * magneticMapping[1] + z * magneticMapping[2];
    //     // qmc588l_data.CorrectedMagnValuemG[1] = x * magneticMapping[3] + y * magneticMapping[4] + z * magneticMapping[5];
    //     // qmc588l_data.CorrectedMagnValuemG[2] = x * magneticMapping[6] + y * magneticMapping[7] + z * magneticMapping[8];
    // }
    return ESP_OK;
}