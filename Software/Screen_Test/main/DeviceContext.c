#include "DeviceContext.h"

vario_state_t vario_state;
vario_settings_t vario_settings;
paraglider_info_t paraglider_info;
device_state_t device_state;
flight_stats_t flight_stats;
//SemaphoreHandle_t semaphore_vario_state;
//SemaphoreHandle_t semaphore_vario_settings;
//SemaphoreHandle_t semaphore_paraglider_info;
//SemaphoreHandle_t semaphore_device_state;
//SemaphoreHandle_t semaphore_flight_stats;

void init_DeviceContext()
{
    //semaphore_vario_state = xSemaphoreCreateMutex();
    //semaphore_vario_settings = xSemaphoreCreateMutex();
    //semaphore_paraglider_info = xSemaphoreCreateMutex();
    //semaphore_device_state = xSemaphoreCreateMutex();
    //semaphore_flight_stats = xSemaphoreCreateMutex();
    vario_state.vertVel_Fifo = fifo_create(FIFO_VERTLAZY_SIZE, sizeof(float));
}

bool vario_state_get_vertVelocity(float* value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.vertVelocity;
    bool returnValue = vario_state.stateChange & 0x1; //read State
    vario_state.stateChange &= 0xFFFF - 0x1; //reset State
    //xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_vertVelocity(float value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
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
    //xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_vertLazy(float* value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.vertVel_Sum / FIFO_VERTLAZY_SIZE;
    bool returnValue = vario_state.stateChange & 0x4;
    vario_state.stateChange &= 0xFFFF - 0x4;
    //xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
bool vario_state_get_groundSpeed(float* value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.groundSpeed;
    bool returnValue = vario_state.stateChange & 0x2;
    vario_state.stateChange &= 0xFFFF - 0x2;
    //xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_groundSpeed(float value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.groundSpeed = value;
    vario_state.stateChange |= 0x2;
    //xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_vertAcc(float* value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.vertAcc;
    bool returnValue = vario_state.stateChange & 0x8;
    vario_state.stateChange &= 0xFFFF - 0x8;
    //xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_vertAcc(float value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.vertAcc = value;
    vario_state.stateChange |= 0x8;
    //xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_pressure(float* value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.pressure;
    bool returnValue = vario_state.stateChange & 0x10;
    vario_state.stateChange &= 0xFFFF - 0x10;
    //xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_pressure(float value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.pressure = value;
    vario_state.stateChange |= 0x10;
    //xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_temperature(float* value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.temperature;
    bool returnValue = vario_state.stateChange & 0x20;
    vario_state.stateChange &= 0xFFFF - 0x20;
    //xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_temperature(float value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.temperature = value;
    vario_state.stateChange |= 0x20;
    //xSemaphoreGive(semaphore_vario_state);
}
bool vario_state_get_heading(float* value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    *value = vario_state.heading;
    bool returnValue = vario_state.stateChange & 0x40;
    vario_state.stateChange &= 0xFFFF - 0x40;
    //xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}
void vario_state_set_heading(float value)
{
    //xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    vario_state.heading = value;
    vario_state.stateChange |= 0x40;
    //xSemaphoreGive(semaphore_vario_state);
}


vario_settings_t* take_vario_settings()
{
    //xSemaphoreTake(semaphore_vario_settings, portMAX_DELAY);
    return &vario_settings;
}
void giveBack_vario_settings()
{
    //xSemaphoreGive(semaphore_vario_settings);
}
paraglider_info_t* take_paraglider_info()
{
    //xSemaphoreTake(semaphore_paraglider_info, portMAX_DELAY);
    return &paraglider_info;
}
void giveBack_paraglider_info()
{
    //xSemaphoreGive(semaphore_paraglider_info);
}
device_state_t* take_device_state()
{
    //xSemaphoreTake(semaphore_device_state, portMAX_DELAY);
    return &device_state;
}
void giveBack_device_state()
{
    //xSemaphoreGive(semaphore_device_state);
}
flight_stats_t* take_flight_stats()
{
    //xSemaphoreTake(semaphore_flight_stats, portMAX_DELAY);
    return &flight_stats;
}
void giveBack_flight_stats()
{
    //xSemaphoreGive(semaphore_flight_stats);
}
