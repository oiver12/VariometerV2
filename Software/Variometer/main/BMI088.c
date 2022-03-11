#include "BMI088.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include <esp_log.h>

#define ACC_CHIP_ID             0x1E
#define GYR_CHIP_ID             0x0F 
/* Register defines */
#define BMI_ACC_CHIP_ID 		0x00
#define BMI_ACC_DATA 			0x12
#define BMI_TEMP_DATA 			0x22
#define BMI_ACC_CONF 			0x40
#define BMI_ACC_RANGE 			0x41
#define BMI_INT1_IO_CONF 	   	0x53
#define BMI_INT1_INT2_MAP_DATA 	0x58
#define BMI_ACC_PWR_CONF 		0x7C
#define BMI_ACC_PWR_CTRL 		0x7D
#define BMI_ACC_SOFTRESET 		0x7E

#define BMI_GYR_CHIP_ID			0x00
#define BMI_GYR_DATA			0x02
#define	BMI_GYR_RANGE			0x0F
#define	BMI_GYR_BANDWIDTH		0x10
#define	BMI_GYR_SOFTRESET		0x14
#define	BMI_GYR_INT_CTRL		0x15
#define	BMI_INT3_INT4_IO_CONF	0x16
#define BMI_INT3_INT4_IO_MAP	0x18

#define accConversion 0.00089813232421875 /* (9.81f / 32768.0f * 2.0f * 1.5f) */
#define gyroConversion 0.00053263221771240234375 /* (0.01745329251f * 1000.0f / 32768.0f) */


static const char *TAG = "BMI088";

static inline esp_err_t writeBMIBytes(spi_device_handle_t spiHandle, uint8_t regAddr, size_t length, const uint8_t *data)
{
    spi_transaction_t transaction;
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = regAddr;
    transaction.length = length * 8;
    transaction.rxlength = 0;
    transaction.user = NULL;
    transaction.tx_buffer = data;
    transaction.rx_buffer = NULL;
    return spi_device_polling_transmit(spiHandle, &transaction);
}

static inline esp_err_t readAccByte(bmi088_t *dev, uint8_t regAddr, uint8_t *data)
{
    uint8_t rxBuffer[2]; // dummyByte + dataByte
    spi_transaction_t transaction;
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = regAddr | 0x80;
    transaction.length = 2 * 8;
    transaction.rxlength = 2 * 8;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = rxBuffer;
    esp_err_t err = spi_device_polling_transmit(dev->acc_spi_handle, &transaction);
    if(err == ESP_OK)
    {
        *data = rxBuffer[1];
    }
    return err;
}

static inline esp_err_t readGyroByte(bmi088_t *dev, uint8_t regAddr, uint8_t *data)
{
    spi_transaction_t transaction;
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = regAddr | 0x80;
    transaction.length = 8;
    transaction.rxlength = 8;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = data;
    esp_err_t err = spi_device_polling_transmit(dev->acc_spi_handle, &transaction);
    return err;
}

esp_err_t bmi088_init(bmi088_t *dev, spi_device_handle_t acc_spi_handle, gpio_num_t acc_cs_pin,spi_device_handle_t gyro_spi_handle, gpio_num_t gyro_cs_pin)
{
    dev->acc_spi_handle = acc_spi_handle;
    dev->gyro_spi_handle = gyro_spi_handle;
    dev->acc_cs_pin = acc_cs_pin;
    dev->gyro_cs_pin = gyro_cs_pin;
    /* SPI starten auf Acc, mit einem Rising CS */
    ESP_ERROR_CHECK(gpio_set_level(acc_cs_pin, 0));
    vTaskDelay(1 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(gpio_set_level(acc_cs_pin,1));
    vTaskDelay(50 / portTICK_PERIOD_MS);
    /* Soft Reset Acc */
    uint8_t dataSend = 0xB6;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_ACC_SOFTRESET, 1, &dataSend));
    vTaskDelay(50 / portTICK_PERIOD_MS);
    /* Check Device ID */
    uint8_t chipID;
    ESP_ERROR_CHECK(readAccByte(dev, BMI_ACC_CHIP_ID, &chipID));
    if(chipID != ACC_CHIP_ID)
    {
        ESP_LOGI(TAG, "Chip ID should be: %u, but is %u", (unsigned int)ACC_CHIP_ID, (unsigned int)chipID);
        return ESP_ERR_INVALID_RESPONSE;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);

    /* Cofigure Acc */
    dataSend = 0xA8;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_ACC_CONF, 1, &dataSend)); /* (no oversampling, ODR = 100 Hz, BW = 40 Hz) */
    vTaskDelay(10 / portTICK_PERIOD_MS);
    dev->acc_odr = ACC_ODR_100HZ_BW_40HZ;

    dataSend = 0x00;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_ACC_RANGE, 1, &dataSend)); /* +- 3g range */
    vTaskDelay(10 / portTICK_PERIOD_MS);
    dev->accelRange = ACCEL_RANGE_3G;

    /* Enable Acc Data Ready Interrupt */
    dataSend = 0x0A;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_INT1_IO_CONF, 1, &dataSend)); /* INT1 = push-pull output, active high */
    vTaskDelay(10 / portTICK_PERIOD_MS);

    dataSend = 0x04;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_INT1_INT2_MAP_DATA, 1, &dataSend)); /* Map DataReady Interrupt to Pin 1 */
    vTaskDelay(10 / portTICK_PERIOD_MS);

    /* Put Acc in active Mode */
    dataSend = 0x00;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_ACC_PWR_CONF, 1, &dataSend));
    vTaskDelay(10 / portTICK_PERIOD_MS);

    /* Turn on Acc */
    dataSend = 0x04;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_ACC_PWR_CTRL, 1, &dataSend));
    vTaskDelay(10 / portTICK_PERIOD_MS);

    /* Gyro soft reset */
    dataSend = 0xB6;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_GYR_SOFTRESET, 1, &dataSend));
    vTaskDelay(250 / portTICK_PERIOD_MS);

    /* Check chip ID */
    ESP_ERROR_CHECK(readGyroByte(dev, BMI_GYR_CHIP_ID, &chipID));
    if(chipID != GYR_CHIP_ID)
    {
        ESP_LOGI(TAG, "Chip ID should be: %u, but is %u", (unsigned int)GYR_CHIP_ID, (unsigned int)chipID);
        return ESP_ERR_INVALID_RESPONSE;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);

    /* Configue Gyro */
    dataSend = 0x01;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_GYR_RANGE, 1, &dataSend)); /* +- 1000 deg/s */
    vTaskDelay(10 / portTICK_PERIOD_MS);
    dev->gyroRange = GYRO_RANGE_1000DPS;

    dataSend = 0x07;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_GYR_BANDWIDTH, 1, &dataSend)); /* ODR = 100 Hz, Filter bandwidth = 32 Hz */
    vTaskDelay(10 / portTICK_PERIOD_MS);
    dev->gyr_odr = GYR_ODR_100HZ_BW_32HZ;

    dataSend = 0x80;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_GYR_INT_CTRL, 1, &dataSend)); /* New data interrupt enabled */
    vTaskDelay(10 / portTICK_PERIOD_MS);

    dataSend = 0x01;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_INT3_INT4_IO_CONF, 1, &dataSend)); /* INT3 = push-pull, active high */
    vTaskDelay(10 / portTICK_PERIOD_MS);

    dataSend = 0x01;
    ESP_ERROR_CHECK(writeBMIBytes(acc_spi_handle, BMI_INT3_INT4_IO_MAP, 1, &dataSend));  /* Data ready interrupt mapped to INT3 pin */
    vTaskDelay(10 / portTICK_PERIOD_MS);
    return ESP_OK;
}

esp_err_t bmi088_ReadAccelerometer(bmi088_t *dev)
{
    uint8_t rxBuf[7]; // dummyByte + 6 Bytes Data
    spi_transaction_t transaction;
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = BMI_ACC_DATA  | 0x80;
    transaction.length = 7 * 8;
    transaction.rxlength = 7 * 8;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = rxBuf;
    esp_err_t err = spi_device_polling_transmit(dev->acc_spi_handle, &transaction);

    /* Form signed 16-bit integers */
	int16_t accX = (int16_t) ((rxBuf[3] << 8) | rxBuf[2]);
	int16_t accY = (int16_t) ((rxBuf[5] << 8) | rxBuf[4]);
	int16_t accZ = (int16_t) ((rxBuf[7] << 8) | rxBuf[6]);

	/* Convert to m/s^2 */
	dev->acc_mps2[0] = accConversion * accX;
	dev->acc_mps2[1] = accConversion * accY;
	dev->acc_mps2[2] = accConversion * accZ;
    return err;
}

esp_err_t bmi088_ReadGyroskop(bmi088_t *dev)
{
    uint8_t rxBuf[6]; /* 6 Bytes Data */
    spi_transaction_t transaction;
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = BMI_GYR_DATA | 0x80;
    transaction.length = 6 * 8;
    transaction.rxlength = 6 * 8;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = rxBuf;
    esp_err_t err = spi_device_polling_transmit(dev->acc_spi_handle, &transaction);

    /* Form signed 16-bit integers */
	int16_t gyrX = (int16_t) ((rxBuf[2] << 8) | rxBuf[1]);
	int16_t gyrY = (int16_t) ((rxBuf[4] << 8) | rxBuf[3]);
	int16_t gyrZ = (int16_t) ((rxBuf[6] << 8) | rxBuf[5]);

	/* Convert to rad/s */
	dev->gyr_rps[0] = gyroConversion * gyrX;
	dev->gyr_rps[1] = gyroConversion * gyrY;
	dev->gyr_rps[2] = gyroConversion * gyrZ;
    return err;
}

