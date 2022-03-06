#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

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

void app_main(void)
{
    esp_err_t ret;
    spi_device_handle_t spi;
    spi_bus_config_t spi1cfg={
        .miso_io_num=SPI1_MISO,
        .mosi_io_num=SPI1_MOSI,
        .sclk_io_num=SPI1_SCK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4092
    };
}
