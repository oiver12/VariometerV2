#include <stdio.h>
#include "driver/i2c.h"
#include <esp_log.h>
#include "nmea_parser.h"
#include <sys/unistd.h>
#include <string.h>
#include "SDCard.h"
#include "main.h"
#include "BatteryManager.h"
#include "SoundManager.h"
#include "IGCLogger.h"
#include "TestTools/FlightLogger.h"
#include "DeviceContext.h"
#include "Screen/Display_Vario.h"
#include "ButtonInput.h"
#include "AGL.h"
#include <compass.h>
#include "flashMemory.h"
#include "TestTools/performance.h"
#include "esp_timer.h"
#include "Sensor/MAXM10S.h"
#include "AirspaceParcer/airspaceParcer.h"
#include "MainTasks.h"

#define POWSW_ENABLE 13

#define I2C_MASTER_FREQ_HZ 100000
#define I2C_SDA 37
#define I2C_SCL 36
#define i2c_master_port 0

#define USART2_TX 17
#define USART2_RX 18

#define USART2_EX_RX 44
#define OUT_EX_GPIO 35

/* GPS */
#define TIME_ZONE (+1)   // Schweiz
#define YEAR_BASE (2000) // date in GPS starts from 2000

static const char *TAG = "main";

void sendAttitude(void *params)
{
    float rotation[4];
    // float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;
    while (1)
    {
        // Quaternion_get_RPY(rotation[0], rotation[1], rotation[2], rotation[3], &roll, &pitch, &tmep);
        // fwrite(&roll, sizeof(float), 1, stdout);
        // fwrite(&pitch, sizeof(float), 1, stdout);
        // fwrite(&yaw, sizeof(float), 1, stdout);
        fwrite(&rotation, sizeof(float), 4, stdout);
        char ending = 0xff;
        fwrite(&ending, sizeof(char), 1, stdout);
        printf("\n");
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    gpio_config_t enableGPIOOutput = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << POWSW_ENABLE,
        .pull_down_en = 0,
        .pull_up_en = 0};

    gpio_config(&enableGPIOOutput);
    gpio_set_level(POWSW_ENABLE, 1);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);

    i2c_config_t conf_I2C = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(i2c_master_port, &conf_I2C);
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf_I2C.mode, 0, 0, 0));

    gpio_config_t gpio_gps_ex = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << OUT_EX_GPIO,
        .pull_down_en = 0,
        .pull_up_en = 0};

    gpio_config(&gpio_gps_ex);
    gpio_set_level(OUT_EX_GPIO, 1);
    // // /* Configure GPS Reader Library */
    nmea_parser_config_t gps_config = NMEA_PARSER_CONFIG_DEFAULT();
    gps_config.uart.rx_pin = USART2_EX_RX;
    nmea_parser_handle_t gps_nmea_hdl = nmea_parser_init(&gps_config, 5 * 1024);
    ESP_LOGI(TAG, "GPS started");

    // sdcard_t *sdcard = calloc(sizeof(sdcard_t), 1);
    init_DeviceContext();

    init_sdcard("/sdcard");
    init_IGCLogger(gps_nmea_hdl);
    initLogging(gps_nmea_hdl);

    // gps handle wird beim DeviceContext erst nach SD Karte gestartet, da wenn direkt abfulg ist, soll
    // das loggen auf SDKarte direkt starten. Wenn die Sd Karte gestartet wird, muss jedoch device kontext
    // schon gestartet sein
    startGps_DeviceContext(gps_nmea_hdl);

    init_SoundManager();
    init_Display();

    flashMemoryInit();

    init_Battery_Manager(4);

    ButtonInput_Init();

    startLogging();
    // AGL_startAGL(sdcard, gps_nmea_hdl);

    // StartAirspaceParse();

    vTaskDelay(100 / portTICK_PERIOD_MS);

    startMainTasks();
    // xTaskCreate(sendAttitude, "Send Attitude", 4 * 1024, NULL, 6, NULL);
    // perfmon_start();
    return;
}
