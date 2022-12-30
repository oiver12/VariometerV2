#include "DeviceContext.h"
#include <math.h>
#include "../../SemaphorTemp.h"
#define portMAX_DELAY 10

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

void vario_state_set_member(void* value, vario_state_member_t member)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    switch (member)
    {
    case vario_state_vertVel:
        vario_state.vertVelocity = *((float*)value);
        float temp;
        if (fifo_is_full(vario_state.vertVel_Fifo))
        {
            fifo_get(vario_state.vertVel_Fifo, &temp);
            vario_state.vertVel_Sum -= temp;
        }
        vario_state.vertVel_Sum += vario_state.vertVelocity;
        fifo_add(vario_state.vertVel_Fifo, &vario_state.vertVelocity);
        vario_state.stateChange |= vario_state_vertVelLazy;
        break;
    case vario_state_groundSpeed:
        vario_state.groundSpeed = *((float*)value);
        break;
    case vario_state_vertVelLazy:
        printf("Cant set vertVelLazy set vertVel, and then get vertVelLazy\n");
        break;
    case vario_state_vertAcc:
        vario_state.vertAcc = *((float*)value);
        break;
    case vario_state_pressure:
        vario_state.pressure = *((float*)value);
        break;
    case vario_state_temperature:
        vario_state.temperature = *((float*)value);
        break;
    case vario_state_heading:
        if (*((int*)value) > 360)
            *((int*)value) = (int)*((int*)value) % 360;
        else if (*((int*)value) < 0)
            *((int*)value) = (int)-*((int*)value) % 360;
        vario_state.heading = *((int*)value);
        break;
    case vario_state_altitudeBaro:
        vario_state.altitudeBaro = *((float*)value);
        break;
    default:
        printf(" Member von Vario State konnte nicht gefunden werden!\n");
    }
    vario_state.stateChange |= member;
    xSemaphoreGive(semaphore_vario_state);
}

bool vario_state_get_member(void* value, vario_state_member_t member, bool stateChange)
{
    xSemaphoreTake(semaphore_vario_state, portMAX_DELAY);
    switch (member)
    {
    case vario_state_vertVel:
        *(float*)value = vario_state.vertVelocity;
        break;
    case vario_state_groundSpeed:
        *(float*)value = vario_state.groundSpeed;
        break;
    case vario_state_vertVelLazy:
        *(float*)value = vario_state.vertVel_Sum / FIFO_VERTLAZY_SIZE;
        break;
    case vario_state_vertAcc:
        *(float*)value = vario_state.vertAcc;
        break;
    case vario_state_pressure:
        *(float*)value = vario_state.pressure;
        break;
    case vario_state_temperature:
        *(float*)value = vario_state.temperature;
        break;
    case vario_state_heading:
        *(int*)value = vario_state.heading;
        break;
    case vario_state_altitudeBaro:
        *(float*)value = vario_state.altitudeBaro;
        break;
    default:
        printf("Member von Vario State konnte nicht gefunden werden!\n");
    }
    bool returnValue = vario_state.stateChange & member;
    if (stateChange)
        vario_state.stateChange &= 0xFFFF - member;
    xSemaphoreGive(semaphore_vario_state);
    return returnValue;
}

void device_state_set_member(void* value, device_state_member_t member)
{
    xSemaphoreTake(semaphore_device_state, portMAX_DELAY);
    switch (member)
    {
    case device_state_batteryPercentage:
        device_state.batteryPercentage = *((float*)value);
        break;
    case device_state_statusGPS:
        device_state.statusGPS = *((uint8_t*)value);
        break;
    case device_state_statusSDCard:
        device_state.status_sd_card = *((sd_card_state_t*)value);
        break;
    default:
        printf("Member von Device State konnte nicht gefunden werden!\n");
        break;
    }
    device_state.stateChange |= member;
    xSemaphoreGive(semaphore_device_state);
}

bool device_state_get_member(void* value, device_state_member_t member, bool stateChange)
{
    xSemaphoreTake(semaphore_device_state, portMAX_DELAY);
    switch (member)
    {
    case device_state_batteryPercentage:
        *(float*)value = device_state.batteryPercentage;
        break;
    case device_state_statusGPS:
        *(uint8_t*)value = device_state.statusGPS;
        break;
    case device_state_statusSDCard:
        *(sd_card_state_t*)value = device_state.status_sd_card;
        break;
    default:
        printf("Member von Device State konnte nicht gefunden werden!\n");
        break;
    }
    bool returnValue = device_state.stateChange & member;
    if (stateChange)
        device_state.stateChange &= 0xFFFF - member;
    xSemaphoreGive(semaphore_device_state);
    return returnValue;
}
