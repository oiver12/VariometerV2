#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include <esp_log.h>
#include <math.h>
#include "BMI088.h"
#include "DPS310.h"
#include "nmea_parser.h"
#include "QuaternionFilter.h"
#include "Kalman/kalman.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h>

#define BMI088_I2C

#define DEG_TO_RAD  0.017453292519943295769236907684886

#define Button_Input 4
#define SPI1_MOSI 11
#define SPI1_SCK 12
#define SPI1_MISO 13

#define IMU_ACC_CS 6
#define ACC_INT 16
#define IMU_GYR_CS 15
#define GYR_INT 7

#define PresSens_CS 10

#define I2C_MASTER_FREQ_HZ 400000     
#define I2C_SDA 39
#define I2C_SCL 42
#define I2C_ACC_INT 41
#define i2c_master_port 0

#define SPI2_MOSI 35
#define SPI2_SCK 36
#define SPI2_MISO 37

#define SDCARD_CS 38
#define SDCARD_DETECT 45
#define SDCARD_MOUNT_POINT "/sdcard"

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

#define R_SPECIFIC 287.053
#define GRAVITY_G 9.80665
#define TEMPGRAD_L 0.0065

/* Kalman Filter */
float ms5611_stnd_height = 0.1298; //Standardabweichung Höhenmessung
float bmi088_stnd_acc = 0.2; //Standardabweichung Beschleunigungssensor
TaskHandle_t TaskKalmanFilter_Handle; //zum entsperen des Kalman Filter Task, wird von TaskMeasureHeight gemacht

/* GPS */
#define TIME_ZONE (+1)   //Schweiz
#define YEAR_BASE (2000) //date in GPS starts from 2000

static const char *TAG = "main";

typedef struct {
    float height;
    float velocity;
    float accX;
} vario_state_t;

vario_state_t vario_state;
SemaphoreHandle_t vario_state_Mutex = NULL; //für das Schrieben und Auslesen aus verschiedenen Tasks

float accSum;
float accCount;
SemaphoreHandle_t acc_Mutex = NULL;

float startPressure;
float startTemperature;
float pressure; //Gemessen von TaskMeasureHeight und gebraucht in Kalman Filter
float temperature;
SemaphoreHandle_t pressure_temperature_Mutex = NULL;

volatile bool hasMeasuredStartPressure = false;

/* Sensoren */
spi_device_handle_t ms5611_spi_handle; //nur von TaskMeasureHeight benutzen!

spi_device_handle_t acc_spi_handle; //nur von TaskMeasureAcc benutzen!
spi_device_handle_t gyro_spi_handle; //nur von TaskMeasureAcc benutzen!

void TaskMeasureHeight(void *parameters)
{
    startPressure = 0;
    startTemperature = 0;
    dps310_t DPS310_Data;
    dps310_init(&DPS310_Data, I2C_NUM_1, 0x77);
    for (size_t i = 0; i < 10; i++)
    {
        dsp310_get_Pressure(&DPS310_Data);
        dps310_get_Temperature(&DPS310_Data);
        startPressure += DPS310_Data.Pressure;
        startTemperature += DPS310_Data.Temperature;
    }
    startPressure /= 10;
    startTemperature /= 10;
    hasMeasuredStartPressure = true;
    int count = 0;
    while(1)
    {
        dsp310_get_Pressure(&DPS310_Data);
        if(count % 34 == 0)
        {
            dps310_get_Temperature(&DPS310_Data);
        }
        count++;
        xSemaphoreTake(pressure_temperature_Mutex, portMAX_DELAY );
        {
            pressure = DPS310_Data.Pressure;
            temperature = DPS310_Data.Temperature;
        }
        xSemaphoreGive(pressure_temperature_Mutex);
        vTaskDelay(30 / portTICK_PERIOD_MS);
        //notify Kalman Filter of new Pressure Reading
        xTaskNotifyGive(TaskKalmanFilter_Handle);
    }
    vTaskDelete(NULL);
}

IRAM_ATTR void bmiISR(TaskHandle_t taskHandle)
{
    BaseType_t HPTaskWoken = pdFALSE;
    //unblock TaskMeasureAcc
    vTaskNotifyGiveFromISR(taskHandle, &HPTaskWoken);
    if (HPTaskWoken == pdTRUE) portYIELD_FROM_ISR();
}

void TaskMeasureAcc(void *parameters)
{
    //wait till StartPressure has read
    while(!hasMeasuredStartPressure){
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
    float rotation[4]; //Nur benutzen in TaskMeasureAcc
    bmi088_t BMI088_Data;
    #ifndef BMI088_I2C
    bmi088_init(&BMI088_Data, acc_spi_handle, IMU_ACC_CS, gyro_spi_handle, IMU_GYR_CS);
    #else
    bmi088_init(&BMI088_Data, i2c_master_port, 0x19, 0x69);
    #endif
    QuaternionFilter_init();
    //reset Rotation
    rotation[0] = 1.0f;
    rotation[1] = 0.0f;
    rotation[2] = 0.0f;
    rotation[3] = 0.0f;
    // Setup Interrupt
    // gpio_config_t gpio_config_int_bmi = {
    //     .pin_bit_mask = 1ULL << I2C_ACC_INT,
    //     .mode         = GPIO_MODE_INPUT,
    //     .pull_up_en   = GPIO_PULLUP_DISABLE,
    //     .pull_down_en = GPIO_PULLDOWN_ENABLE,
    //     .intr_type    = GPIO_INTR_NEGEDGE  //
    // };
    // //set Acc Pin as Interrupt 
    // //TODO: nur AccelPin wir als Interrupt benutzt. Vielleicht auch Gyro benutzen?
    // gpio_config(&gpio_config_int_bmi);
    // gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    // //Funktion bmiISR als Interrupt Funktion handle von dieser Funktion als Parameter, damit diese Funktion von der Interrupt aufgeweckt werden kann
    // gpio_isr_handler_add((gpio_num_t)I2C_ACC_INT, bmiISR, xTaskGetCurrentTaskHandle());
    float lastTime = esp_timer_get_time();
    // volatile float roll = 0;
    // volatile float pitch = 0;
    // volatile float yaw = 0;
    while(1)
    {
        //wait till unblocked from Interrupt
        //uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        /*if (notificationValue > 1) {
            ESP_LOGW(TAG, "Task Notification higher than 1, value: %d", notificationValue);
        }*/
        bmi088_ReadAccelerometer(&BMI088_Data);
        bmi088_ReadGyroskop(&BMI088_Data);
        float deltaT = (esp_timer_get_time() - lastTime) / 1000000;
        lastTime = esp_timer_get_time();
        /* Update Orientation */
        QuaternionFilter_Mahony(-BMI088_Data.acc_mps2[0] / 9.81, BMI088_Data.acc_mps2[1] / 9.81, BMI088_Data.acc_mps2[2] / 9.81, BMI088_Data.gyr_rps[0], -BMI088_Data.gyr_rps[1], -BMI088_Data.gyr_rps[2], deltaT, rotation);
        //Quaternion_get_RPY(rotation[0], rotation[1], rotation[2], rotation[3], &roll, &pitch, &yaw);
        /* Calculate global Acc in X Direction */
        float accX = (2*(rotation[1]*rotation[3] - rotation[0]*rotation[2])) * -BMI088_Data.acc_mps2[0] + (2*(rotation[2]*rotation[3]+rotation[0]*rotation[1])) * BMI088_Data.acc_mps2[1] + (2*(rotation[0]*rotation[0]+rotation[3]*rotation[3]) -1)*BMI088_Data.acc_mps2[2];
        //printf("%.2f \n\r", accX);
        xSemaphoreTake(acc_Mutex, portMAX_DELAY);
        {
            accSum += accX;
            accCount++;
        }
        xSemaphoreGive(acc_Mutex);
    }
    vTaskDelete(NULL);
}

void TaskKalmanFilter(void *parameters)
{
    //wait till StartPressure has read
    while(!hasMeasuredStartPressure){
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    kalman_t kf;
    kalman_filter_init(&kf);
    float deltaT = 0.025;
    /* Set State Transition Matrix */
    /* F = 1 t
           0 1 */
    matrix_t *F = &kf.F;
    matrix_set(F, 0, 0, 1);
    matrix_set(F, 0, 1, deltaT);
    matrix_set(F, 1, 0, 0);
    matrix_set(F, 1, 1, 1);
    /* Set Control Matrix */
    /* G = 0.5*t^2
           t        */
    matrix_t *G = &kf.G;
    matrix_set(G, 0, 0, 0.5*deltaT*deltaT);
    matrix_set(G, 1, 0, deltaT);
    /* Set  Process Noise Matrix*/
    /* Q = (t^4)/4  (t^3)/2
           (t^3)/2   t^2       * kalman_process_noise^2*/
    matrix_t *Q = &kf.Q;
    matrix_set(Q, 0, 0, ((deltaT*deltaT*deltaT*deltaT)/4)*bmi088_stnd_acc*bmi088_stnd_acc);
    matrix_set(Q, 0, 1, ((deltaT*deltaT*deltaT)/2)*bmi088_stnd_acc*bmi088_stnd_acc);
    matrix_set(Q, 1, 0, ((deltaT*deltaT*deltaT)/2)*bmi088_stnd_acc*bmi088_stnd_acc);
    matrix_set(Q, 1, 1, (deltaT*deltaT)*bmi088_stnd_acc*bmi088_stnd_acc);
    /* Set Observation Matrix */
    /* H = [1   0] */
    matrix_t *H = &kf.H;
    matrix_set(H, 0, 0, 1);
    matrix_set(H, 0, 1, 0);
    /* Set Measurement covariance matrix */
    /* R = ms5611_stnd_height^2 */
    matrix_t *R = &kf.R;
    matrix_set(R, 0, 0, ms5611_stnd_height*ms5611_stnd_height);
    /* Set Estimate Uncertainty Matrix*/
    /* P = 500   0
            0   500 */
    matrix_t *P = &kf.P;
    matrix_set(P, 0, 0, 500);
    matrix_set(P, 0, 1, 0);
    matrix_set(P, 1, 0, 0);
    matrix_set(P, 1, 1, 500);
    /* Set  Input Vector*/
    /* u = (a + g) */
    matrix_t *u = &kf.u;
    matrix_set(u, 0, 0, 0);
    /* Initialize State Vector */
    matrix_t *x = &kf.x;
    matrix_set(x, 0, 0, 0);
    matrix_set(x, 1, 0, 0);
    kalman_predict(&kf);
    matrix_t *z = &kf.z;
    float lastTime = esp_timer_get_time();
    while(1)
    {
        //wait for new Result from Pressure Sensor
        uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (notificationValue > 1) {
            ESP_LOGW(TAG, "Task Notification higher than 1, value: %d", notificationValue);
            continue;
        }
        float heightDifferenz;
        xSemaphoreTake(pressure_temperature_Mutex, portMAX_DELAY);
        {
            heightDifferenz = ((startTemperature + 273.15) / TEMPGRAD_L)*(pow(pressure / startPressure, -TEMPGRAD_L * R_SPECIFIC / GRAVITY_G) - 1);
        }
        xSemaphoreGive(pressure_temperature_Mutex);
        float acc = 0;
        //TODO: ist Mean am besten?
        //read Acc Queue till its empty and compute mean from Acc
        xSemaphoreTake(acc_Mutex, portMAX_DELAY);
        {
            acc = accSum / accCount;
            if(accCount == 0)
            {
                ESP_LOGI(TAG, "Had no Acc Readings");
                acc = 9.81f;
            }
            accSum = 0;
            accCount = 0;
        }
        xSemaphoreGive(acc_Mutex);
        deltaT = (esp_timer_get_time() - lastTime) / 1000000;
        /* Set Q with new deltaT */
        matrix_set(Q, 0, 0, ((deltaT*deltaT*deltaT*deltaT)/4)*bmi088_stnd_acc*bmi088_stnd_acc);
        matrix_set(Q, 0, 1, ((deltaT*deltaT*deltaT)/2)*bmi088_stnd_acc*bmi088_stnd_acc);
        matrix_set(Q, 1, 0, ((deltaT*deltaT*deltaT)/2)*bmi088_stnd_acc*bmi088_stnd_acc);
        matrix_set(Q, 1, 1, (deltaT*deltaT)*bmi088_stnd_acc*bmi088_stnd_acc);
        /* Set G with new deltaT */
        matrix_set(G, 0, 0, 0.5*deltaT*deltaT);
        matrix_set(G, 1, 0, deltaT);
        /* Set F with new deltaT */
        matrix_set(F, 0, 0, 1);
        matrix_set(F, 0, 1, deltaT);
        matrix_set(F, 1, 0, 0);
        matrix_set(F, 1, 1, 1);
        /* Set z measurement */
        matrix_set(z, 0, 0, heightDifferenz);
        /* Set Input */
        matrix_set(u, 0, 0, acc - 9.81);
        kalman_update(&kf);
        kalman_predict(&kf);
        xSemaphoreTake(vario_state_Mutex, portMAX_DELAY);
        {
            vario_state.accX = acc;
            vario_state.height = heightDifferenz; //TODO: add startHeight
            vario_state.velocity = matrix_get(x, 1, 0);
        }
        xSemaphoreGive(vario_state_Mutex);
        printf("%.2f\n", matrix_get(x, 1, 0));
        lastTime = esp_timer_get_time();
    }
    vTaskDelete(NULL);
} 

static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    //TODO: GNZDA kann ignoriert werden
    gps_t *gps = NULL;
    switch (event_id) {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        /* print information parsed from GPS statements */
        ESP_LOGI(TAG, "%d/%d/%d %d:%d:%d => \r\n"
                 "\t\t\t\t\t\tlatitude   = %.05f°N\r\n"
                 "\t\t\t\t\t\tlongitude = %.05f°E\r\n"
                 "\t\t\t\t\t\taltitude   = %.02fm\r\n"
                 "\t\t\t\t\t\tspeed      = %fm/s\r\n"
                 "\t\t\t\t\t\tsats in View      = %u\r\n"
                "\t\t\t\t\t\tmessage      = %s ",
                 gps->date.year + YEAR_BASE, gps->date.month, gps->date.day,
                 gps->tim.hour + TIME_ZONE, gps->tim.minute, gps->tim.second,
                 gps->latitude, gps->longitude, gps->altitude, gps->speed, gps->sats_in_view, gps->infoString);
        break;
    case GPS_UNKNOWN:
        /* print unknown statements */
        ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
        break;
    default:
        break;
    }
}

void TaskSDCard(void *pvParameters)
{
    sdmmc_card_t *card = (sdmmc_card_t *)pvParameters;
    sdmmc_card_print_info(stdout, card);
    vTaskDelete(NULL);
}

void PrintTask(void *params)
{
    while(1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "TestLog");
    }
    vTaskDelete(NULL);
}

void app_main(void)
 {
    vario_state_Mutex = xSemaphoreCreateMutex();
    pressure_temperature_Mutex = xSemaphoreCreateMutex();
    acc_Mutex = xSemaphoreCreateMutex();
    /* Initialize SPI for MS5611 and BMI088 */
    // spi_bus_config_t spi1cfg={
    //     .miso_io_num=SPI1_MISO,
    //     .mosi_io_num=SPI1_MOSI,
    //     .sclk_io_num=SPI1_SCK,
    //     .quadwp_io_num=-1,
    //     .quadhd_io_num=-1,
    //     .max_transfer_sz=4092
    // };
    // ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &spi1cfg, SPI_DMA_CH_AUTO));
    // ESP_LOGI(TAG, "Initialized SPI1");
    // /* Add MS5611 to SPI */
    // spi_device_interface_config_t ms5611_spi_cfg = {
    //     .clock_speed_hz = 1*1000, //1 kHz
    //     .mode = 0,
    //     .queue_size = 7, 
    //     .spics_io_num = PresSens_CS,
    //     .address_bits = 8,
    //     .command_bits = 8,
    // };
    // ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &ms5611_spi_cfg, &ms5611_spi_handle));
    // ms5611_t MS5611_Data;
    // ms5611_init(&MS5611_Data, ms5611_spi_handle, MS5611_OSR_256, MS5611_OSR_256);
    // while(1)
    // {
    //     ms5611_get_sensor_data(&MS5611_Data);
    //     xSemaphoreTake(pressure_temperature_Mutex, portMAX_DELAY );
    //     {
    //         pressure = MS5611_Data.Pressure;
    //         temperature = MS5611_Data.Temperature;
    //     }
    //     xSemaphoreGive(pressure_temperature_Mutex);
    // }
    //xTaskCreate(TaskMeasureHeight, "Measure Height", 4 * 1024, NULL, 1, NULL);

    // /* Add BMI088 to SPI */
    // spi_device_interface_config_t bmi088_acc_spi_cfg = {
    //     .clock_speed_hz = 10*1000*1000,
    //     .mode = 0,
    //     .queue_size = 7,
    //     .spics_io_num = IMU_ACC_CS
    // };
    // ESP_ERROR_CHECK(spi_bus_add_device(SPI1_HOST, &bmi088_acc_spi_cfg, &acc_spi_handle));
    // spi_device_interface_config_t bmi088_gyr_spi_cfg = {
    //     .clock_speed_hz = 10*1000*1000,
    //     .mode = 0,
    //     .queue_size = 7,
    //     .spics_io_num = IMU_GYR_CS
    // };
    // ESP_ERROR_CHECK(spi_bus_add_device(SPI1_HOST, &bmi088_gyr_spi_cfg, &gyro_spi_handle));
    // xTaskCreate(TaskMeasureAcc, "Measure Acceleration", 4 * 1024, NULL, 1, NULL);
    // #else

    i2c_config_t conf_MS5611 = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SPI1_MOSI,
        .scl_io_num = SPI1_SCK,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_NUM_1, &conf_MS5611);
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_1, conf_MS5611.mode, 0, 0, 0));

    i2c_config_t conf_BMI088 = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(i2c_master_port, &conf_BMI088);
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf_BMI088.mode, 0, 0, 0));

    xTaskCreate(TaskKalmanFilter, "Kalman Filter", 4 * 1024, NULL, 10, &TaskKalmanFilter_Handle);
    xTaskCreate(TaskMeasureHeight, "Measure Height", 4 * 1024, NULL, 5, NULL);
    xTaskCreate(TaskMeasureAcc, "Measure Acceleration", 4 * 1024, NULL, 1, NULL);
    // #endif
    while(1)
    {
        vTaskDelay(100000 / portTICK_PERIOD_MS);
    }
    /* Configure GPS Reader Library */
    nmea_parser_config_t gps_config = NMEA_PARSER_CONFIG_DEFAULT();
    gps_config.uart.rx_pin = USART2_RX;
    nmea_parser_handle_t gps_nmea_hdl = nmea_parser_init(&gps_config, 4*1024);
    nmea_parser_add_handler(gps_nmea_hdl, gps_event_handler, NULL);
    ESP_LOGI(TAG, "GPS started");
    // xTaskCreate(PrintTask, "Measure Height", 4 * 1024, NULL, 1, NULL);
    // /* Start Kalman Filter Task (High Priority)*/
    // xTaskCreate(TaskKalmanFilter, "Kalman Filter", 4 * 1024, NULL, 10, &TaskKalmanFilter_Handle);
    // /* Configure SDCard */
    // //TODO: Keine SPI Kommunikation, bevor die SD Karte init ist. https://github.com/espressif/esp-idf/issues/1597
    /*gpio_config_t sdcard_detect_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (uint64_t) 0x1 << SDCARD_DETECT,
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&sdcard_detect_conf);
    int sdCardLevel = gpio_get_level(SDCARD_DETECT);
    ESP_LOGI(TAG, "%i", sdCardLevel);
    if(sdCardLevel != 1)
    {
        //TODO: Was nun machen?
        ESP_LOGI(TAG, "No SD Card detected!");
    }
    else if(sdCardLevel == 1)
    {
        ESP_LOGI(TAG, "SD Card detected!");
    }
    esp_vfs_fat_sdmmc_mount_config_t sdcard_mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    ESP_LOGI(TAG, "Initialized SD Card");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    spi_bus_config_t spi2cfg={
        .miso_io_num=SPI2_MISO,
        .mosi_io_num=SPI2_MOSI,
        .sclk_io_num=SPI2_SCK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4000
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi2cfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "Initialized SPI2");
    sdspi_device_config_t slot_config ={ 
        .host_id   = SPI2_HOST, 
        .gpio_cs   = SDCARD_CS, 
        .gpio_cd   = SDSPI_SLOT_NO_CD, 
        .gpio_wp   = SDSPI_SLOT_NO_WP, 
        .gpio_int  = GPIO_NUM_NC};
    ESP_LOGI(TAG, "Mounting filesystem");
    //TODO: schlechte Funktion oder so da kein Error Handling
    esp_err_t ret = esp_vfs_fat_sdspi_mount(SDCARD_MOUNT_POINT, &host, &slot_config, &sdcard_mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    xTaskCreate(TaskSDCard, "TaskSDCard", 4 * 1024, card, 1, NULL);*/
}
