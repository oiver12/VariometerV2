#include "FlightLogger.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "DPS310.h"
#include "BMI088.h"
#include "QMC588L.h"
#include "main.h"
#include "Kalman/kalman.h"
#include "driver/i2c.h"
#include <math.h>
#include "QuaternionFilter.h"
#include <esp_log.h>
#include "nmea_parser.h"

#define BMI088_I2C

#define DEG_TO_RAD  0.017453292519943295769236907684886

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

#define USART2_TX 17
#define USART2_RX 18
#define GPS_ON 8

#define R_SPECIFIC 287.053
#define GRAVITY_G 9.80665
#define TEMPGRAD_L 0.0065

/* Kalman Filter */
float ms5611_stnd_height = 0.1298; //Standardabweichung Höhenmessung
float bmi088_stnd_acc = 0.2; //Standardabweichung Beschleunigungssensor
TaskHandle_t TaskKalmanFilter_Handle; //zum entsperen des Kalman Filter Task, wird von TaskMeasureHeight gemacht

static const char *TAG = "Logger";

vario_state_t vario_state;
SemaphoreHandle_t vario_state_Mutex = NULL; //für das Schrieben und Auslesen aus verschiedenen Tasks

float accSum;
float accCount;
SemaphoreHandle_t acc_Mutex = NULL; //Sync von TaskMeasureAcc und KalmanFilter

float startPressure;
float startTemperature;
float pressure;
float temperature;
SemaphoreHandle_t pressure_temperature_Mutex = NULL; //Gemessen von TaskMeasureHeight und gebraucht in Kalman Filter

int32_t magnOrientation[3];
SemaphoreHandle_t magnOrientation_Mutex = NULL; //Gemessen von TaskMeasureMagn und gebraucht von TaskMeasureAcc

volatile bool hasMeasuredStartPressure = false;

static void TaskMeasureHeight(void *parameters)
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
    vTaskDelay(50 / portTICK_PERIOD_MS);
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
        //notify Kalman Filter of new Pressure Reading
        xTaskNotifyGive(TaskKalmanFilter_Handle);
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static void TaskMeasureMagn(void *parameters)
{   
    qmc588l_t QMC588L_Data;
    qmc588l_init(&QMC588L_Data, i2c_master_port, 0x0D, QMC588L_MR_200HZ, QMC588L_OSR_512, QMC588L_SCALE_8G);
    //qmc588l_calculateCalibration(&QMC588L_Data);
    qmc588l_readMag(&QMC588L_Data);
    ESP_LOGI(TAG, "Magn: x= %i, y=%i, z=%i", QMC588L_Data.magnValue[0], QMC588L_Data.magnValue[1], QMC588L_Data.magnValue[2]);
    xSemaphoreTake(magnOrientation_Mutex, portMAX_DELAY);
    {
        magnOrientation[0] = QMC588L_Data.magnValue[0];
        magnOrientation[1] = QMC588L_Data.magnValue[1];
        magnOrientation[2] = QMC588L_Data.magnValue[2];
    }
    xSemaphoreGive(magnOrientation_Mutex);
    int azimuth = (int)(atan2(QMC588L_Data.magnValue[1], QMC588L_Data.magnValue[0]) * 180.0 / M_PI);
    azimuth = azimuth > 0 ? 360 + azimuth : azimuth;
    //printf("%i \n", azimuth);
}

static void TaskMeasureAcc(void *parameters)
{
    //wait till StartPressure has read
    while(!hasMeasuredStartPressure){
        vTaskDelay(2);
    }
    float rotation[4]; //Nur benutzen in TaskMeasureAcc
    bmi088_t BMI088_Data;
    bmi088_init(&BMI088_Data, i2c_master_port, 0x19, 0x69);
    QuaternionFilter_init();
    //reset Rotation
    rotation[0] = 1.0f;
    rotation[1] = 0.0f;
    rotation[2] = 0.0f;
    rotation[3] = 0.0f;
    float lastTime = esp_timer_get_time();
    while(1)
    {
        xSemaphoreTake(magnOrientation_Mutex, portMAX_DELAY);
        int32_t magN = magnOrientation[1];
        int32_t magE = -magnOrientation[0];
        int32_t magD = magnOrientation[2];
        xSemaphoreGive(magnOrientation_Mutex);
        bmi088_ReadAccelerometer(&BMI088_Data);
        bmi088_ReadGyroskop(&BMI088_Data);
        float deltaT = (esp_timer_get_time() - lastTime) / 1000000;
        lastTime = esp_timer_get_time();
        /* Update Orientation */
        //QuaternionFilter_Madgwick(-BMI088_Data.acc_mps2[0] / 9.81, BMI088_Data.acc_mps2[1] / 9.81, BMI088_Data.acc_mps2[2] / 9.81, BMI088_Data.gyr_rps[0], -BMI088_Data.gyr_rps[1], -BMI088_Data.gyr_rps[2], magN, magE, magD, deltaT, rotation);
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
        lastTime = esp_timer_get_time();
    }
    vTaskDelete(NULL);
} 

static void NMEASentenceLogger(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id) {
    case GPS_NMEA_SENTECE:
        ESP_LOGI(TAG, "NMEA Sentence:%s", (char *)event_data);
        break;
    default:
        break;
    }

    ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
}

static void LoggingTask(void *arg)
{

}

void startLogging()
{
}
void endLogging()
{

}