#include "DeviceContext.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

vario_state_t vario_state;
vario_settings_t vario_settings;
paraglider_info_t paraglider_info;
device_state_t device_state;
flight_stats_t flight_stats;
SemaphoreHandle_t semaphore_vario_state;
SemaphoreHandle_t semaphore_vario_settings;
SemaphoreHandle_t semaphore_paraglider_info;
SemaphoreHandle_t semaphore_device_state;
SemaphoreHandle_t semaphore_flight_stats;

void init_DeviceContext()
{
    semaphore_vario_state = xSemaphoreCreateMutex();
    semaphore_vario_settings = xSemaphoreCreateMutex();
    semaphore_paraglider_info = xSemaphoreCreateMutex();
    semaphore_device_state = xSemaphoreCreateMutex();
    semaphore_flight_stats = xSemaphoreCreateMutex();
    vario_state.vertVel_Fifo = fifo_create(FIFO_VERTLAZY_SIZE, sizeof(float));
}

bool vario_state_get_vertVelocity(float* value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.vertVelocity;
    bool returnValue = vario_state.stateChange & 0x1; //read State
    vario_state.stateChange &= 0xFFFF - 0x1; //reset State
    xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_vertVelocity(float value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.vertVelocity = value;
    vario_state.stateChange |= 0x1; //set State to changed
    float temp;
    if (fifo_is_full(vario_state.vertVel_Fifo))
    {
        fifo_get(vario_state.vertVel_Fifo, &temp);
        vario_state.vertVel_Sum -= temp;
    }
    vario_state.vertVel_Sum += value;
    fifo_add(vario_state.vertVel_Fifo, &value);
    vario_state.stateChange |= 0x4;
    xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_vertLazy(float* value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.vertVel_Sum / FIFO_VERTLAZY_SIZE;
    bool returnValue = vario_state.stateChange & 0x4;
    vario_state.stateChange &= 0xFFFF - 0x4;
    xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
bool vario_state_get_groundSpeed(float* value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.groundSpeed;
    bool returnValue = vario_state.stateChange & 0x2;
    vario_state.stateChange &= 0xFFFF - 0x2;
    xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_groundSpeed(float value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.groundSpeed = value;
    vario_state.stateChange |= 0x2;
    xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_vertAcc(float* value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.vertAcc;
    bool returnValue = vario_state.stateChange & 0x8;
    vario_state.stateChange &= 0xFFFF - 0x8;
    xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_vertAcc(float value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.vertAcc = value;
    vario_state.stateChange |= 0x8;
    xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_pressure(float* value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.pressure;
    bool returnValue = vario_state.stateChange & 0x10;
    vario_state.stateChange &= 0xFFFF - 0x10;
    xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_pressure(float value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.pressure = value;
    vario_state.stateChange |= 0x10;
    xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_temperature(float* value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.temperature;
    bool returnValue = vario_state.stateChange & 0x20;
    vario_state.stateChange &= 0xFFFF - 0x20;
    xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_temperature(float value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.temperature = value;
    vario_state.stateChange |= 0x20;
    xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_heading(float* value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.heading;
    bool returnValue = vario_state.stateChange & 0x40;
    vario_state.stateChange &= 0xFFFF - 0x40;
    xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_heading(float value)
{
    if(value < 0)
        value += 360;
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.heading = value;
    vario_state.stateChange |= 0x40;
    xSemaphoreGive(semaphore_vario_state);
}

bool vario_state_get_altitudeBaro(float* value, bool stateChange)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.altitudeBaro;
    bool returnValue = vario_state.stateChange & 0x80;
    if(stateChange)
        vario_state.stateChange &= 0xFFFF - 0x80;
    xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}

void vario_state_set_altitudeBaro(float value)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.altitudeBaro = value;
    vario_state.stateChange |= 0x80;
    xSemaphoreGive(semaphore_vario_state);
}

void device_state_set_sdcard(uint8_t value)
{
    xSemaphoreTake(semaphore_device_state, portMAX_DELAY);
    device_state.statusSDCard = value;
    device_state.stateChange |= 0x4;
    xSemaphoreGive(semaphore_device_state);
}

bool device_state_get_sdcard(uint8_t* value)
{
    xSemaphoreTake(semaphore_device_state, portMAX_DELAY);
    *value = device_state.statusSDCard;
    bool returnValue = device_state.stateChange & 0x4;
    device_state.stateChange &= 0xFFFF - 0x4;
    xSemaphoreGive(semaphore_device_state);
    return returnValue;
}
