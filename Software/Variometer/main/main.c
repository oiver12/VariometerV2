#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include <esp_log.h>
#include "MS5611.h"
#include "BMI088.h"
#include "nmea_parser.h"
#include "QuaternionFilter.h"
#include "Kalman/kalman.h"

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


float ms5611_stnd_height = 0.1298; //Standardabweichung Höhenmessung
float bmi088_stnd_acc = 0.0069; //Standardabweichung Beschleunigungssensor
/* GPS */
#define TIME_ZONE (+1)   //Schweiz
#define YEAR_BASE (2000) //date in GPS starts from 2000

static const char *TAG = "main";

typedef struct {
    float velocity;
    float accX;
    float q[4];
} vario_state_t;

vario_state_t vario_state;

/* Sensoren */
ms5611_t MS5611_Data; //nur von TaskMeasureHeight benutzen!
spi_device_handle_t ms5611_spi_handle; //nur von TaskMeasureHeight benutzen!

spi_device_handle_t acc_spi_handle; //nur von TaskMeasureAcc benutzen!
spi_device_handle_t gyro_spi_handle; //nur von TaskMeasureAcc benutzen!
SemaphoreHandle_t vario_state_Mutex = NULL;
QueueHandle_t measuredAccMessage = NULL; //speichert Acc Data in einer Queue, damit dann ausgelesen werden kann, wenn Luftdruckdaten zur verfügung stehen

void TaskMeasureHeight(void *parameters)
{
    ms5611_init(&MS5611_Data, ms5611_spi_handle, MS5611_OSR_256, MS5611_OSR_256);
    while(1)
    {
        ms5611_get_sensor_data(&MS5611_Data);
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
    bmi088_t BMI088_Data;
    bmi088_init(&BMI088_Data, acc_spi_handle, IMU_ACC_CS, gyro_spi_handle, IMU_GYR_CS);
    QuaternionFilter_init();
    //reset Rotation
    xSemaphoreTake(vario_state_Mutex, portMAX_DELAY);
    {
        vario_state.q[0] = 1.0f;
        vario_state.q[1] = 0.0f;
        vario_state.q[2] = 0.0f;
        vario_state.q[3] = 0.0f;
    }
    xSemaphoreGive(vario_state_Mutex);
    // Setup Interrupt
    gpio_config_t gpio_config_int_bmi = {
        .pin_bit_mask = (uint64_t) 0x1 << ACC_INT,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type    = GPIO_INTR_POSEDGE  //
    };
    gpio_config(&gpio_config_int_bmi);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add((gpio_num_t) ACC_INT, bmiISR, xTaskGetCurrentTaskHandle());
    float lastTime = esp_timer_get_time();
    while(1)
    {
        //wait till unblocked from Interrupt
        uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (notificationValue > 1) {
            ESP_LOGW(TAG, "Task Notification higher than 1, value: %d", notificationValue);
            continue;
        }
        bmi088_ReadAccelerometer(&BMI088_Data);
        bmi088_ReadGyroskop(&BMI088_Data);
        float deltaT = esp_timer_get_time() - lastTime;
        lastTime = esp_timer_get_time();
        float accX = 0;
        xSemaphoreTake(vario_state_Mutex, portMAX_DELAY);
        {
            /* Update Orientation */
            QuaternionFilter_Mahony(-BMI088_Data.acc_mps2[0], BMI088_Data.acc_mps2[1], BMI088_Data.acc_mps2[2], BMI088_Data.gyr_rps[0], -BMI088_Data.gyr_rps[1], -BMI088_Data.gyr_rps[2], deltaT, vario_state.q);
            /* Calculate global Acc in X Direction */
            accX = (2*(vario_state.q[1]*vario_state.q[3] - vario_state.q[0]*vario_state.q[2])) * -BMI088_Data.acc_mps2[0] + (2*(vario_state.q[2]*vario_state.q[3]+vario_state.q[0]*vario_state.q[1])) * BMI088_Data.acc_mps2[1] + (2*(vario_state.q[0]*vario_state.q[0]+vario_state.q[3]*vario_state.q[3]) -1)*BMI088_Data.acc_mps2[2];
            vario_state.accX = accX;
        };
        xSemaphoreGive(vario_state_Mutex);
        if(xQueueSend(measuredAccMessage, &accX, portMAX_DELAY) != pdTRUE)
        {
            ESP_LOGI(TAG, "Queue full");
        }
    }
    vTaskDelete(NULL);
}

void TaskKalmanFilter(void *parameters)
{
    kalman_t kf;
    kalman_filter_init(&kf);
    float deltaT = 30 / 1000; //alle 30ms
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
    kalman_update(&kf);
    kalman_predict(&kf);
} 

static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    gps_t *gps = NULL;
    switch (event_id) {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        /* print information parsed from GPS statements */
        ESP_LOGI(TAG, "%d/%d/%d %d:%d:%d => \r\n"
                 "\t\t\t\t\t\tlatitude   = %.05f°N\r\n"
                 "\t\t\t\t\t\tlongitude = %.05f°E\r\n"
                 "\t\t\t\t\t\taltitude   = %.02fm\r\n"
                 "\t\t\t\t\t\tspeed      = %fm/s",
                 gps->date.year + YEAR_BASE, gps->date.month, gps->date.day,
                 gps->tim.hour + TIME_ZONE, gps->tim.minute, gps->tim.second,
                 gps->latitude, gps->longitude, gps->altitude, gps->speed);
        break;
    case GPS_UNKNOWN:
        /* print unknown statements */
        ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
        break;
    default:
        break;
    }
}

void app_main(void)
{
    vario_state_Mutex = xSemaphoreCreateMutex();
    measuredAccMessage = xQueueCreate(10, sizeof(float));
    /* Initialize SPI for MS5611 and BMI088 */
    spi_bus_config_t spi1cfg={
        .miso_io_num=SPI1_MISO,
        .mosi_io_num=SPI1_MOSI,
        .sclk_io_num=SPI1_SCK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4092
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi1cfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "Initialized SPI1");
    /* Add MS5611 to SPI */
    spi_device_interface_config_t ms5611_spi_cfg = {
        .clock_speed_hz = 10*1000*1000, //10 MHz
        .mode = 0,
        .queue_size = 7, 
        .spics_io_num = PresSens_CS
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &ms5611_spi_cfg, &ms5611_spi_handle));
    xTaskCreate(TaskMeasureHeight, "Measure Height", 4 * 1024, NULL, 1, NULL);

    /* Add BMI088 to SPI */
    spi_device_interface_config_t bmi088_acc_spi_cfg = {
        .clock_speed_hz = 10*1000*1000,
        .mode = 0,
        .queue_size = 7,
        .spics_io_num = IMU_ACC_CS
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &bmi088_acc_spi_cfg, &acc_spi_handle));
    spi_device_interface_config_t bmi088_gyr_spi_cfg = {
        .clock_speed_hz = 10*1000*1000,
        .mode = 0,
        .queue_size = 7,
        .spics_io_num = IMU_GYR_CS
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &bmi088_gyr_spi_cfg, &gyro_spi_handle));
    xTaskCreate(TaskMeasureAcc, "Measure Acceleration", 4 * 1024, NULL, 1, NULL);

    /* Configure GPS Reader Library */
    nmea_parser_config_t gps_config = NMEA_PARSER_CONFIG_DEFAULT();
    gps_config.uart.rx_pin = USART2_RX;
    nmea_parser_handle_t gps_nmea_hdl = nmea_parser_init(&gps_config);
    nmea_parser_add_handler(gps_nmea_hdl, gps_event_handler, NULL);
}
