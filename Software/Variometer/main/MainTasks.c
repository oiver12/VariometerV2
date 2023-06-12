#include "MainTasks.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>
#include "Sensor/BMI088.h"
#include "Sensor/DPS310.h"
#include "QuaternionFilter.h"
#include "Kalman/kalman.h"
#include "Sensor/QMC588L.h"
#include <math.h>
#include "DeviceContext.h"
#include "SoundManager.h"
#include "TestTools/FlightLogger.h"
#include <esp_log.h>
#include <esp_timer.h>
#include "Watchdog/Watchdog.h"

#define i2c_master_port 0
#define PresSens_INT 38

#define R_SPECIFIC 287.053
#define GRAVITY_G 9.80665
#define TEMPGRAD_L 0.0065

/* Kalman Filter */
float dps310_stnd_height = 0.1298;    // Standardabweichung Höhenmessung
float bmi088_stnd_acc = 0.9;          // Standardabweichung Beschleunigungssensor
TaskHandle_t TaskKalmanFilter_Handle; // zum entsperen des Kalman Filter Task, wird von TaskMeasureHeight gemacht

QueueHandle_t accQueue;
// float rotationVario[4];

float startPressure;
float startTemperature;
float pressure;
float temperature;
SemaphoreHandle_t pressure_temperature_Mutex = NULL; // Gemessen von TaskMeasureHeight und gebraucht in Kalman Filter

volatile bool hasMeasuredStartPressure = false;

static const char *TAG = "MainTasks";

static void IRAM_ATTR dps310ISR(void *arg)
{
    BaseType_t HPTaskWoken = pdFALSE;
    // unblock TaskMeasureAcc
    vTaskNotifyGiveFromISR((TaskHandle_t)arg, &HPTaskWoken);
    if (HPTaskWoken == pdTRUE)
        portYIELD_FROM_ISR();
}

static void TaskMeasureHeight(void *parameters)
{
    // watchdog_add(NULL);
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
        ESP_LOGI(TAG, "StartPressure: %.2f, StartTemp: %.2f", startPressure, startTemperature);
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
    startPressure /= 10;
    startTemperature /= 10;
    hasMeasuredStartPressure = true;
    // Setup Interrupt
    gpio_config_t gpio_config_INT_DPS310 = {
        .pin_bit_mask = 1ULL << PresSens_INT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE};
    // set PresSens Pin as Interrupt
    gpio_config(&gpio_config_INT_DPS310);
    // Funktion bmiISR als Interrupt Funktion handle von dieser Funktion als Parameter, damit diese Funktion von der Interrupt aufgeweckt werden kann
    gpio_isr_handler_add((gpio_num_t)PresSens_INT, dps310ISR, xTaskGetCurrentTaskHandle());
    vTaskDelay(50 / portTICK_PERIOD_MS);
    int count = 0;
    float lastPressure = startPressure;
    while (1)
    {
        // wait till unblocked from Interrupt
        uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, 500 / portTICK_PERIOD_MS);
        if (notificationValue == 0)
        {
            ESP_LOGW(TAG, "Interrupt didn't fire");
        }
        else if (notificationValue > 1)
        {
            ESP_LOGW(TAG, "Interrupt did fire multiple times");
        }
        // watchdog_reset();
        dsp310_get_Pressure(&DPS310_Data);
        if (count % 34 == 0)
        {
            dps310_get_Temperature(&DPS310_Data);
        }
        count++;
        // Filter invalid pressure Results (200Pa = 19m -> change of 600 m in one second)
        if (fabsf(lastPressure - (float)DPS310_Data.Pressure) > 200)
        {
            ESP_LOGW(TAG, "Invalid Pressure Result!");
        }
        else
        {
            lastPressure = DPS310_Data.Pressure;
            xSemaphoreTake(pressure_temperature_Mutex, portMAX_DELAY);
            {
                pressure = DPS310_Data.Pressure;
                temperature = DPS310_Data.Temperature;
            }
            xSemaphoreGive(pressure_temperature_Mutex);
            float pressure = DPS310_Data.Pressure;
            vario_state_set_member(&pressure, vario_state_pressure);
        }
        // notify Kalman Filter of new Pressure Reading
        xTaskNotifyGiveIndexed(TaskKalmanFilter_Handle, 0);
        // vTaskDelay(30 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static bool checkBMI088MsValidity(float accMs[], float last_Acc[], uint8_t counterAcc[], float gyrMs[], float last_Gyr[], uint8_t counterGyr[])
{
    const uint8_t maxValueCounter = 2;
    // This means approx. change of 1000 m/s^2 per second
    const float maxAccD = 10;
    if (fabsf(accMs[0] - last_Acc[0]) > maxAccD && counterAcc[0] < maxValueCounter)
    {
        counterAcc[0]++;
        ESP_LOGE(TAG, "Not AcX: %.2f, %.2f", accMs[0], last_Acc[0]);
        return false;
    }
    if (fabsf(accMs[1] - last_Acc[1]) > maxAccD && counterAcc[1] < maxValueCounter)
    {
        counterAcc[1]++;
        ESP_LOGE(TAG, "Not AcY: %.2f, %.2f", accMs[1], last_Acc[1]);
        return false;
    }
    if (fabsf(accMs[2] - last_Acc[2]) > maxAccD && counterAcc[2] < maxValueCounter)
    {
        counterAcc[2]++;
        ESP_LOGE(TAG, "Not AcZ: %.2f, %.2f", accMs[2], last_Acc[2]);
        return false;
    }
    const float maxGyrD = 100;
    if (fabsf(gyrMs[0] - last_Gyr[0]) > maxGyrD && counterGyr[0] < maxValueCounter)
    {
        counterGyr[0]++;
        ESP_LOGE(TAG, "Not GyX: %.2f, %.2f", gyrMs[0], last_Gyr[0]);
        return false;
    }
    if (fabsf(gyrMs[1] - last_Gyr[1]) > maxGyrD && counterGyr[1] < maxValueCounter)
    {
        counterGyr[1]++;
        ESP_LOGE(TAG, "Not GyY: %.2f, %.2f", gyrMs[1], last_Gyr[1]);
        return false;
    }
    if (fabsf(gyrMs[2] - last_Gyr[2]) > maxGyrD && counterGyr[2] < maxValueCounter)
    {
        counterGyr[2]++;
        ESP_LOGE(TAG, "Not GyZ: %.2f, %.2f", gyrMs[2], last_Gyr[2]);
        return false;
    }
    memset(counterAcc, 0, sizeof(uint8_t) * 3);
    memset(counterGyr, 0, sizeof(uint8_t) * 3);
    last_Acc[0] = accMs[0];
    last_Acc[1] = accMs[1];
    last_Acc[2] = accMs[2];
    last_Gyr[0] = gyrMs[0];
    last_Gyr[1] = gyrMs[1];
    last_Gyr[2] = gyrMs[2];
    return true;
}

static void TaskMeasureAcc(void *parameters)
{
    // wait till StartPressure has read
    while (!hasMeasuredStartPressure)
    {
        vTaskDelay(2);
    }
    watchdog_add(NULL);
    float rotation[4];
    /* Init BMI088 */
    bmi088_t BMI088_Data;
    bmi088_init(&BMI088_Data, i2c_master_port, 0x18, 0x68);

    /* Init QMC588L */
    qmc588l_t *QMC588L_Data;
    QMC588L_Data = qmc588l_init(i2c_master_port, 0xD, QMC588L_MR_200HZ, QMC588L_OSR_512, QMC588L_SCALE_2G);

    QuaternionFilter_init();
    // reset Rotation
    rotation[0] = 1.0f;
    rotation[1] = 0.0f;
    rotation[2] = 0.0f;
    rotation[3] = 0.0f;

    float lastTime = esp_timer_get_time();
    int count = 0;
    float last_Acc[3];
    uint8_t counterAcc[3] = {0, 0, 0};
    float last_Gyr[3];
    uint8_t counterGyr[3] = {0, 0, 0};
    bmi088_ReadAccelerometer(&BMI088_Data);
    bmi088_ReadGyroskop(&BMI088_Data);
    last_Acc[0] = BMI088_Data.acc_mps2[0];
    last_Acc[1] = BMI088_Data.acc_mps2[1];
    last_Acc[2] = BMI088_Data.acc_mps2[2];
    last_Gyr[0] = BMI088_Data.gyr_rps[0];
    last_Gyr[1] = BMI088_Data.gyr_rps[1];
    last_Gyr[2] = BMI088_Data.gyr_rps[2];
    // TODO: check why sometimes acc after filter is wrong?
    float lastAccX = 9.81f;
    int counterAccX = 0;
    while (1)
    {
        watchdog_reset();
        // Clear Notification from Kalman Filter to 1 at index 1, so klaman Filter waits, till we are finished
        ulTaskNotifyValueClearIndexed(TaskKalmanFilter_Handle, 1, 0xffffffff);
        qmc588l_readMag();
        bmi088_ReadAccelerometer(&BMI088_Data);
        bmi088_ReadGyroskop(&BMI088_Data);
        if (!checkBMI088MsValidity(BMI088_Data.acc_mps2, last_Acc, counterAcc, BMI088_Data.gyr_rps, last_Gyr, counterGyr))
        {
            xTaskNotifyGiveIndexed(TaskKalmanFilter_Handle, 1);
            continue;
        }
        float deltaT = (esp_timer_get_time() - lastTime) / 1000000;
        lastTime = esp_timer_get_time();
        /* Update Orientation */
        float an = BMI088_Data.acc_mps2[1] / 9.81;
        float ae = BMI088_Data.acc_mps2[0] / 9.81f;
        float ad = BMI088_Data.acc_mps2[2] / 9.81f;
        float gn = -BMI088_Data.gyr_rps[1];
        float ge = -BMI088_Data.gyr_rps[0];
        float gd = -BMI088_Data.gyr_rps[2];
        float mn = -QMC588L_Data->CorrectedMagnValuemG[0];
        float me = QMC588L_Data->CorrectedMagnValuemG[1];
        float md = -QMC588L_Data->CorrectedMagnValuemG[2];
        QuaternionFilter_Madgwick(an, ae, ad, gn, ge, gd, mn, me, md, deltaT, rotation);
        count++;
        // if(count % 10 == 0)
        // printf("Raw:%i,%i,%i,%i,%i,%i,%i,%i,%i\n", (int)(BMI088_Data.acc_mps2[0] * 1000), (int)(BMI088_Data.acc_mps2[1] * 1000), (int)(BMI088_Data.acc_mps2[2] * 1000), (int)BMI088_Data.gyr_rps[0], (int)BMI088_Data.gyr_rps[1], (int)BMI088_Data.gyr_rps[2], (int)QMC588L_Data->CorrectedMagnValuemG[0], (int)QMC588L_Data->CorrectedMagnValuemG[1], (int)QMC588L_Data->CorrectedMagnValuemG[2]);
        /* Calculate global Acc in X Direction */
        float accX = (2 * (rotation[1] * rotation[3] - rotation[0] * rotation[2])) * an + (2 * (rotation[2] * rotation[3] + rotation[0] * rotation[1])) * ae + (2 * (rotation[0] * rotation[0] + rotation[3] * rotation[3]) - 1) * ad;
        accX *= 9.81f;
        if (fabsf(accX - lastAccX) > 10 && counterAccX < 3)
        {
            counterAccX++;
            ESP_LOGE(TAG, "Can't accept this Acc");
        }
        else
        {
            counterAccX = 0;
            lastAccX = accX;
            xQueueSend(accQueue, &accX, (TickType_t)0);
        }
        xTaskNotifyGiveIndexed(TaskKalmanFilter_Handle, 1);
        // wait for 1 tick
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}
static void ResetKalman(kalman_t *kf)
{
    // kalman_filter_reset(kf);
    float deltaT = 0.025;
    /* Set State Transition Matrix */
    /* F = 1 t
           0 1 */
    matrix_t *F = &kf->F;
    matrix_set(F, 0, 0, 1);
    matrix_set(F, 0, 1, deltaT);
    matrix_set(F, 1, 0, 0);
    matrix_set(F, 1, 1, 1);
    /* Set Control Matrix */
    /* G = 0.5*t^2
           t        */
    matrix_t *G = &kf->G;
    matrix_set(G, 0, 0, 0.5 * deltaT * deltaT);
    matrix_set(G, 1, 0, deltaT);
    /* Set  Process Noise Matrix*/
    /* Q = (t^4)/4  (t^3)/2
           (t^3)/2   t^2       * kalman_process_noise^2*/
    matrix_t *Q = &kf->Q;
    matrix_set(Q, 0, 0, ((deltaT * deltaT * deltaT * deltaT) / 4) * bmi088_stnd_acc * bmi088_stnd_acc);
    matrix_set(Q, 0, 1, ((deltaT * deltaT * deltaT) / 2) * bmi088_stnd_acc * bmi088_stnd_acc);
    matrix_set(Q, 1, 0, ((deltaT * deltaT * deltaT) / 2) * bmi088_stnd_acc * bmi088_stnd_acc);
    matrix_set(Q, 1, 1, (deltaT * deltaT) * bmi088_stnd_acc * bmi088_stnd_acc);
    /* Set Observation Matrix */
    /* H = [1   0] */
    matrix_t *H = &kf->H;
    matrix_set(H, 0, 0, 1);
    matrix_set(H, 0, 1, 0);
    /* Set Measurement covariance matrix */
    matrix_t *R = &kf->R;
    matrix_set(R, 0, 0, dps310_stnd_height * dps310_stnd_height);
    /* Set Estimate Uncertainty Matrix*/
    /* P = 500   0
            0   500 */
    matrix_t *P = &kf->P;
    matrix_set(P, 0, 0, 500);
    matrix_set(P, 0, 1, 0);
    matrix_set(P, 1, 0, 0);
    matrix_set(P, 1, 1, 500);
    /* Set  Input Vector*/
    /* u = (a + g) */
    matrix_t *u = &kf->u;
    matrix_set(u, 0, 0, 0);
    /* Initialize State Vector */
    matrix_t *x = &kf->x;
    matrix_set(x, 0, 0, 0);
    matrix_set(x, 1, 0, 0);
}

static void TaskKalmanFilter(void *parameters)
{
    // wait till StartPressure has read
    while (!hasMeasuredStartPressure)
    {
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    kalman_t kf;
    kalman_filter_init(&kf);
    ResetKalman(&kf);
    kalman_predict(&kf);
    float deltaT = 0.025;
    matrix_t *F = &kf.F;
    matrix_t *G = &kf.G;
    matrix_t *Q = &kf.Q;
    matrix_t *u = &kf.u;
    matrix_t *x = &kf.x;
    matrix_t *z = &kf.z;
    float lastTime = esp_timer_get_time();
    int countHeading = 0;
    bool wasGreater = false;
    while (1)
    {
        // wait for new Result from Pressure Sensor
        uint32_t notificationValue = ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
        if (notificationValue > 1)
        {
            ESP_LOGW(TAG, "Task Notification higher than 1, value: %lu", (unsigned long)notificationValue);
            continue;
        }
        // wait, till acc Reading has finished
        ulTaskNotifyTakeIndexed(1, pdTRUE, portMAX_DELAY);
        float heightDifferenz;
        xSemaphoreTake(pressure_temperature_Mutex, portMAX_DELAY);
        {
            heightDifferenz = ((startTemperature + 273.15) / TEMPGRAD_L) * (pow(pressure / startPressure, -TEMPGRAD_L * R_SPECIFIC / GRAVITY_G) - 1);
        }
        xSemaphoreGive(pressure_temperature_Mutex);
        float acc = 0;
        float rotation[4] = {1, 0, 0, 0};

        float tempAcc;
        float accSum = 0;
        int accSize = 0;
        while (xQueueReceive(accQueue, &tempAcc, (TickType_t)0) == pdTRUE)
        {
            accSum += tempAcc;
            accSize++;
        }
        if (accSize == 0)
        {
            ESP_LOGW(TAG, "Cant recieve Acc");
            acc = 9.81f;
        }
        else
            acc = accSum / accSize;
        deltaT = (esp_timer_get_time() - lastTime) / 1000000;
        /* Set Q with new deltaT */
        matrix_set(Q, 0, 0, ((deltaT * deltaT * deltaT * deltaT) / 4) * bmi088_stnd_acc * bmi088_stnd_acc);
        matrix_set(Q, 0, 1, ((deltaT * deltaT * deltaT) / 2) * bmi088_stnd_acc * bmi088_stnd_acc);
        matrix_set(Q, 1, 0, ((deltaT * deltaT * deltaT) / 2) * bmi088_stnd_acc * bmi088_stnd_acc);
        matrix_set(Q, 1, 1, (deltaT * deltaT) * bmi088_stnd_acc * bmi088_stnd_acc);
        /* Set G with new deltaT */
        matrix_set(G, 0, 0, 0.5 * deltaT * deltaT);
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

        FlightLogger_t flight_state = {
            .height = heightDifferenz,
            .accX = acc,
            .velocity = matrix_get(x, 1, 0),
            .rotation = {rotation[0], rotation[1], rotation[2], rotation[3]},
        };
        float vertVel = matrix_get(x, 1, 0);
        if (isnanf(vertVel))
        {
            ESP_LOGI(TAG, "A: %.2f, P: %.2f", acc, pressure);
            ESP_LOGI(TAG, "Had to reset Kalman Filter\n");
            ResetKalman(&kf);
            continue;
        }
        vario_state_set_member(&vertVel, vario_state_vertVel);
        float accXCorr = acc - 9.81;
        vario_state_set_member(&accXCorr, vario_state_vertAcc);
        vario_state_set_member(&heightDifferenz, vario_state_altitudeBaro);
        if (countHeading % 10 == 0)
        {
            float roll, pitch, yaw;
            Quaternion_get_RPY(rotation[0], rotation[1], rotation[2], rotation[3], &roll, &pitch, &yaw);
            int heading = (int)yaw;
            while (heading > 360.0)
                heading -= 360.0;
            while (heading < 0.0)
                heading += 360.0;
            // printf("Yaw: %.2f\n", yaw);
            vario_state_set_member(&heading, vario_state_heading);
        }
        logVarioData(flight_state);
        SoundManager_calculateNewBiep();
        // SoundManager_calculateNewBiep(2.5f);
        if (vertVel > 10)
            wasGreater = true;
        if (wasGreater)
            ESP_LOGE(TAG, "Was greater");
        printf("%.2f\n", vertVel);
        // printf("T: %i, pressure: %.1f, ACC: %.2f, S: %i, V: %.2f\n", (int)(deltaT * 1000), pressure, acc, accSize, vertVel);
        lastTime = esp_timer_get_time();
    }
    vTaskDelete(NULL);
}

void startMainTasks()
{
    pressure_temperature_Mutex = xSemaphoreCreateMutex();
    accQueue = xQueueCreate(10, sizeof(float));
    // startKalmanFilter();
    xTaskCreate(TaskKalmanFilter, "Kalman Filter", 4 * 1024, NULL, TASK_KALMAN_PRIORITY, &TaskKalmanFilter_Handle);
    xTaskCreate(TaskMeasureHeight, "Measure Height", 4 * 1024, NULL, TASK_HEIGHT_PRIORITY, NULL);
    xTaskCreate(TaskMeasureAcc, "Measure Acceleration", 4 * 1024, NULL, TASK_ACC_PRIORITY, NULL);
}
