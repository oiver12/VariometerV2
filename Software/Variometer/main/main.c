#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "MS5611.h"
#include <esp_log.h>

#define Button_Input 4

#define SPI1_MOSI 11
#define SPI1_SCK 12
#define SPI1_MISO 13

#define IMU_ACC_CS 6
#define ACC_INT 16
#define IMU_GYR_CS 15
#define GYR_INT 7

#define PresSens_CS 10

#define SPI2_MOSI 35
#define SPI2_SCK 36
#define SPI2_MSIO 37

#define SDCARD_CS 38
#define SDCARD_DETECT 45

#define LCD_CS 48
#define LCD_DS 47
#define LCD_RST 21
#define LCD_TCH_CS 46

#define BAT_MS 9
#define BAT_ADC 14

#define USART2_TX 17
#define USART2_RX 18
#define GSP_ON 8

#define BUZZ_1 1
#define BUZZ_2 2

static const char *TAG = "main";

ms5611_t MS5611_Data;
spi_device_handle_t ms5611_spi_handle;

void TaskMeasureHeight(void *parameters)
{
    ms5611_init(&MS5611_Data, ms5611_spi_handle, MS5611_OSR_256, MS5611_OSR_256);
    ms5611_get_sensor_data(&MS5611_Data);
}

void app_main(void)
{
    //Initialize SPI for MS5611 and BMI088
    esp_err_t ret;
    spi_bus_config_t spi1cfg={
        .miso_io_num=SPI1_MISO,
        .mosi_io_num=SPI1_MOSI,
        .sclk_io_num=SPI1_SCK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4092
    };
    ret = spi_bus_initialize(SPI2_HOST, &spi1cfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "Initialized SPI1");
    //Initialize MS5611
    spi_device_interface_config_t ms5611_spi_cfg = {
        .clock_speed_hz = 10*1000*1000, //10 MHz
        .mode = 0,
        .queue_size = 7, 
        .spics_io_num = PresSens_CS
    };
    ret = spi_bus_add_device(SPI2_HOST, &ms5611_spi_cfg, &ms5611_spi_handle);
    ESP_ERROR_CHECK(ret);
    xTaskCreate(TaskMeasureHeight, "Measure Height", 1000, NULL, 1, NULL);
}
