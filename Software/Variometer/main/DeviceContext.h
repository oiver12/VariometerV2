#include <stdint.h>
#include <stdbool.h>
#include "Screen/FIFO.h"

#define MAX_STRING_SIZE 16
#define FIFO_VERTLAZY_SIZE 5

typedef struct {
    float vertVelocity; //m/s  0. bit (0x1)
    float groundSpeed; //km/h  1. bit (0x2)
    fifo_t vertVel_Fifo;
    float vertVel_Sum; //   2. bit (0x4)
    float vertAcc; //m/s^2  3. bit (0x8)
    float pressure; //hPa   4. bit (0x10)
    float temperature; //C° 5. bit (0x20)
    float heading; //0-360° 6. bit (0x40)
    float altitudeBaro; //m 7. bit (0x80)
    uint16_t stateChange;
} vario_state_t;

typedef struct {
    float soundVolume;
    float takeOffSpeed;
    float landingSpeed;
    float landingTimout;

    uint16_t stateChange;
} vario_settings_t;

typedef struct {
    char manufacture[MAX_STRING_SIZE];
    char model[MAX_STRING_SIZE];

    uint16_t stateChange;
} paraglider_info_t;

typedef struct {
    float batteryPower;     //0. bit (0x1)
    uint8_t statusGPS;      //1. bit (0x2)
    uint8_t statusSDCard;   //2. bit (0x4)

    uint16_t stateChange;
} device_state_t;

typedef struct {
    float altitudeMax;
    float altitudeMin;

    float maxVelX;
    float minVelX;

    uint16_t stateChange;
} flight_stats_t;

void init_DeviceContext();

bool vario_state_get_vertVelocity(float* value);
void vario_state_set_vertVelocity(float value);
bool vario_state_get_groundSpeed(float* value);
void vario_state_set_groundSpeed(float value);
bool vario_state_get_vertLazy(float* value);
bool vario_state_get_vertAcc(float* value);
void vario_state_set_vertAcc(float value);
bool vario_state_get_pressure(float* value);
void vario_state_set_pressure(float value);
bool vario_state_get_temperature(float* value);
void vario_state_set_temperature(float value);
bool vario_state_get_heading(float* value);
void vario_state_set_heading(float value);
bool vario_state_get_altitudeBaro(float* value, bool stateChange);
void vario_state_set_altitudeBaro(float value);

void device_state_set_sdcard(uint8_t value);
bool device_state_get_sdcard(uint8_t* value);
