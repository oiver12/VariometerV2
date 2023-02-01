#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include <esp_log.h>
#include <math.h>
#include "Sensor/BMI088.h"
#include "Sensor/DPS310.h"
#include "nmea_parser.h"
#include "QuaternionFilter.h"
#include "Kalman/kalman.h"
#include <sys/unistd.h>
#include <string.h>
#include "SDCard.h"
#include "main.h"
#include "BatteryManager.h"
#include "Sensor/QMC588L.h"
#include "SoundManager.h"
#include "IGCLogger.h"
#include <time.h>
#include "TestTools/FlightLogger.h"
#include "DeviceContext.h"
#include "Screen/Display_Vario.h"
#include "ButtonInput.h"

#define POWSW_ENABLE 13

#define I2C_MASTER_FREQ_HZ 100000     
#define I2C_SDA 37
#define I2C_SCL 36
#define PresSens_INT 38
#define i2c_master_port 0

#define USART2_TX 17
#define USART2_RX 18

#define R_SPECIFIC 287.053
#define GRAVITY_G 9.80665
#define TEMPGRAD_L 0.0065

/* Kalman Filter */
float dps310_stnd_height = 0.1298; //Standardabweichung Höhenmessung
float bmi088_stnd_acc = 0.9; //Standardabweichung Beschleunigungssensor
TaskHandle_t TaskKalmanFilter_Handle; //zum entsperen des Kalman Filter Task, wird von TaskMeasureHeight gemacht

/* GPS */
#define TIME_ZONE (+1)   //Schweiz
#define YEAR_BASE (2000) //date in GPS starts from 2000

static const char *TAG = "main";

float accSum;
float accCount;
float rotationVario[4];
SemaphoreHandle_t acc_Mutex = NULL; //Sync von TaskMeasureAcc und KalmanFilter

float startPressure;
float startTemperature;
float pressure;
float temperature;
SemaphoreHandle_t pressure_temperature_Mutex = NULL; //Gemessen von TaskMeasureHeight und gebraucht in Kalman Filter

volatile bool hasMeasuredStartPressure = false;

IRAM_ATTR void dps310ISR(TaskHandle_t taskHandle)
{
    BaseType_t HPTaskWoken = pdFALSE;
    //unblock TaskMeasureAcc
    vTaskNotifyGiveFromISR(taskHandle, &HPTaskWoken);
    if (HPTaskWoken == pdTRUE) portYIELD_FROM_ISR();
}

static void TaskMeasureHeight(void *parameters)
{

    startPressure = 0;
    startTemperature = 0;
    dps310_t DPS310_Data;
    dps310_init(&DPS310_Data, i2c_master_port, 0x76);
    for (size_t i = 0; i < 10; i++)
    {
        dsp310_get_Pressure(&DPS310_Data);
        dps310_get_Temperature(&DPS310_Data);
        startPressure += DPS310_Data.Pressure;
        startTemperature += DPS310_Data.Temperature;
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
    startPressure /= 10;
    startTemperature /= 10;
    hasMeasuredStartPressure = true;
    // Setup Interrupt
    gpio_config_t gpio_config_INT_DPS310 = {
        .pin_bit_mask = 1ULL << PresSens_INT,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_POSEDGE 
    };
    //set PresSens Pin as Interrupt 
    gpio_config(&gpio_config_INT_DPS310);
    //Funktion bmiISR als Interrupt Funktion handle von dieser Funktion als Parameter, damit diese Funktion von der Interrupt aufgeweckt werden kann
    gpio_isr_handler_add((gpio_num_t)PresSens_INT, dps310ISR, xTaskGetCurrentTaskHandle());
    vTaskDelay(50 / portTICK_PERIOD_MS);
    int count = 0;
    while(1)
    {
        //wait till unblocked from Interrupt
        uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, 500 / portTICK_PERIOD_MS);
        if (notificationValue == 0) {
            ESP_LOGW(TAG, "Interrupt didn't fire");
        }
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
        float pressure = DPS310_Data.Pressure;
        vario_state_set_member(&pressure, vario_state_pressure);
        //printf("%.2f \n", DPS310_Data.Pressure);
        //notify Kalman Filter of new Pressure Reading
        xTaskNotifyGive(TaskKalmanFilter_Handle);
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static void TaskMeasureAcc(void *parameters)
{
    //wait till StartPressure has read
    while(!hasMeasuredStartPressure){
        vTaskDelay(2);
    }
    float rotation[4];
    /* Init BMI088 */
    bmi088_t BMI088_Data;
    bmi088_init(&BMI088_Data, i2c_master_port, 0x18, 0x68);

    /* Init QMC588L */
    qmc588l_t QMC588L_Data;
    qmc588l_init(&QMC588L_Data, i2c_master_port, 0xD, QMC588L_MR_200HZ, QMC588L_OSR_512, QMC588L_SCALE_8G);

    QuaternionFilter_init();
    //reset Rotation
    rotation[0] = 1.0f;
    rotation[1] = 0.0f;
    rotation[2] = 0.0f;
    rotation[3] = 0.0f;

    float lastTime = esp_timer_get_time();
    // volatile float roll = 0;
    // volatile float pitch = 0;
    // volatile float yaw = 0;
    int count = 0;
    while(1)
    {
        qmc588l_readMag(&QMC588L_Data);
        int32_t magN =  -QMC588L_Data.magnValue[0];
        int32_t magE = QMC588L_Data.magnValue[1];
        int32_t magD = QMC588L_Data.magnValue[2];

        bmi088_ReadAccelerometer(&BMI088_Data);
        bmi088_ReadGyroskop(&BMI088_Data);
        float deltaT = (esp_timer_get_time() - lastTime) / 1000000;
        lastTime = esp_timer_get_time();
        /* Update Orientation */
        //TODO: wahrscheinlich ein Vorzeichen oder sonstiges falsch mit Mag.
        QuaternionFilter_Madgwick(-BMI088_Data.acc_mps2[0] / 9.81, BMI088_Data.acc_mps2[1] / 9.81, BMI088_Data.acc_mps2[2] / 9.81, BMI088_Data.gyr_rps[0], -BMI088_Data.gyr_rps[1], -BMI088_Data.gyr_rps[2], magN, magE, magD, deltaT, rotation);
        //QuaternionFilter_Mahony(-BMI088_Data.acc_mps2[0] / 9.81, BMI088_Data.acc_mps2[1] / 9.81, BMI088_Data.acc_mps2[2] / 9.81, BMI088_Data.gyr_rps[0], -BMI088_Data.gyr_rps[1], -BMI088_Data.gyr_rps[2], deltaT, rotation);
        //Quaternion_get_RPY(rotation[0], rotation[1], rotation[2], rotation[3], &roll, &pitch, &yaw);
        // if(count % 10 == 0)
        //     printf("%.2f \n", yaw);
        count++;
        /* Calculate global Acc in X Direction */
        float accX = (2*(rotation[1]*rotation[3] - rotation[0]*rotation[2])) * -BMI088_Data.acc_mps2[0] + (2*(rotation[2]*rotation[3]+rotation[0]*rotation[1])) * BMI088_Data.acc_mps2[1] + (2*(rotation[0]*rotation[0]+rotation[3]*rotation[3]) -1)*BMI088_Data.acc_mps2[2];
        //printf("%.2f \n", accX-9.81);
        xSemaphoreTake(acc_Mutex, portMAX_DELAY);
        {
            accSum += accX;
            accCount++;
            memcpy(rotationVario, rotation, sizeof(rotationVario));
        }
        xSemaphoreGive(acc_Mutex);
    }
    vTaskDelete(NULL);
}

static void TaskKalmanFilter(void *parameters)
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
    /* R = dps310_stnd_height^2 */
    matrix_t *R = &kf.R;
    matrix_set(R, 0, 0, dps310_stnd_height*dps310_stnd_height);
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
    int countHeading =  0;
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
        float rotation[4];
        xSemaphoreTake(acc_Mutex, portMAX_DELAY);
        {
            if(accCount == 0)
            {
                ESP_LOGI(TAG, "Had no Acc Readings");
                acc = 9.81f;
            }
            else
            {
                acc = accSum / accCount;
            }
            accSum = 0;
            accCount = 0;
            memcpy(rotation, rotationVario, sizeof(rotationVario));
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
        // xSemaphoreTake(vario_state_Mutex, portMAX_DELAY);
        // {
        //     vario_state.accX = acc;
        //     vario_state.height = heightDifferenz; //TODO: add startHeight
        //     vario_state.velocity = matrix_get(x, 1, 0);
        // }
        // xSemaphoreGive(vario_state_Mutex);
        FlightLogger_t flight_state = {
            .height = heightDifferenz,
            .accX = acc,
            .velocity = matrix_get(x, 1, 0),
            .rotation = {rotation[0], rotation[1], rotation[2], rotation[3]},
        };

        float vertVel = matrix_get(x, 1, 0);
        vario_state_set_member(&vertVel, vario_state_vertVel);
        float accXCorr = acc - 9.81;
        vario_state_set_member(&accXCorr, vario_state_vertAcc);
        vario_state_set_member(&heightDifferenz, vario_state_altitudeBaro);
        if(countHeading % 10 == 0)
        {
            float roll, pitch, yaw;
            Quaternion_get_RPY(rotation[0], rotation[1], rotation[2], rotation[3], &roll, &pitch, &yaw);
            int heading = (int)yaw;
            vario_state_set_member(&heading, vario_state_heading);
        }
        logVarioData(flight_state);
        SoundManager_calculateNewBiep(matrix_get(x, 1, 0));
        //printf("%.2f\n", matrix_get(x, 1, 0));
        lastTime = esp_timer_get_time();
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
        .pull_up_en = 0
    };

    gpio_config(&enableGPIOOutput);
    gpio_set_level(POWSW_ENABLE, 1);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    
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

    // // /* Configure GPS Reader Library */
    nmea_parser_config_t gps_config = NMEA_PARSER_CONFIG_DEFAULT();
    gps_config.uart.rx_pin = USART2_RX;
    nmea_parser_handle_t gps_nmea_hdl = nmea_parser_init(&gps_config, 4*1024);
    ESP_LOGI(TAG, "GPS started");
    // sdcard_t sdcard;
    pressure_temperature_Mutex = xSemaphoreCreateMutex();
    acc_Mutex = xSemaphoreCreateMutex();
    init_DeviceContext(gps_nmea_hdl);

    sdcard_t *sdcard = calloc(sizeof(sdcard_t), 1);
    startLogging(sdcard, gps_nmea_hdl);
    start_IGCLogger(sdcard, gps_nmea_hdl);
    
    init_SoundManager();

    init_Battery_Manager(4);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
    xTaskCreate(TaskKalmanFilter, "Kalman Filter", 4 * 1024, NULL, TASK_KALMAN_PRIORITY, &TaskKalmanFilter_Handle);
    xTaskCreate(TaskMeasureHeight, "Measure Height", 4 * 1024, NULL, TASK_HEIGHT_PRIORITY, NULL);
    xTaskCreate(TaskMeasureAcc, "Measure Acceleration", 4 * 1024, NULL, TASK_ACC_PRIORITY, NULL);

    init_Display();
    ButtonInput_Init();
    return;
}
