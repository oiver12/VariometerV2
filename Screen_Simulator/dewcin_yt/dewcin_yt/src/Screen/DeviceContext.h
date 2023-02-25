#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "FIFO.h"
#include "nmea_parser.h"

/*
* Speicher für alle Device Information. Auf diese können alle zugreifen (z.B. wird vertVel gespeichert, welche vom Bildschirm gelesen wird und
* auch von der SD-Karte gelesen wird. Mit einem SemaphoreHandle_t wird dann vor Race Conditions geschützt, damit von verschiedenen Tasks ausgelesen
* werden kann. stateChange gibt immer an, ob seit dem letzten auslesen die Variable verändert wurde (muss dann z.B. nicht nochmal gerendert werden).
*/

#define MAX_STRING_SIZE 16
#define FIFO_VERTLAZY_SIZE 66 //ca. 2 Sekunde integriert

#define Volume_max_Size 20000 //eig. 2^16 / 2 = 32'768, aber wäre zu viel Leistung für den Lautsprecher (denke ich ausprobieren!)

typedef struct {
    float vertVelocity; //m/s  0. bit (0x1)
    fifo_t vertVel_Fifo;
    float vertVel_Sum; // 1. bit (0x4)
    float vertAcc; //m/s^2  2. bit (0x8)
    float pressure; //hPa   3. bit (0x10)
    float temperature; //C° 4. bit (0x20)
    int heading; //0-360° 5. bit (0x40)
    float altitudeBaro; //m 6. bit(0x80)
    gps_t gps_data;
    float groundLevel;
    float agl;

    uint16_t stateChange;
} vario_state_t;

typedef enum {
    vario_state_vertVel = 1 << 0,
    vario_state_vertVelLazy = 1 << 1,
    vario_state_vertAcc = 1 << 2,
    vario_state_pressure = 1 << 3,
    vario_state_temperature = 1 << 4,
    vario_state_heading = 1 << 5,
    vario_state_altitudeBaro = 1 << 6,
    vario_state_gps_data = 1 << 7,
    vario_state_groundLevel = 1 << 8,
    vario_state_agl = 1 << 9,
} vario_state_member_t;

typedef struct {
    uint16_t soundVolume;
    float takeOffSpeed;
    float landingSpeed;
    float landingTimout;

    uint16_t stateChange;
} vario_settings_t;

typedef enum {
    vario_settings_soundVolume = 1 << 0,
    vario_settings_takeOffSpeed = 1 << 1,
    vario_settings_landingSpeed = 1 << 2,
    vario_settings_landingTimout = 1 << 3,
} vario_settings_member_t;

typedef struct {
    char manufacture[MAX_STRING_SIZE];
    char model[MAX_STRING_SIZE];

    uint16_t stateChange;
} paraglider_info_t;

typedef enum {
    SD_CARD_STATE_INIT = 1 << 0,
    SD_CARD_STATE_ERROR_INIT = 1 << 1,
    SD_CARD_STATE_IGC_LOGGING = 1 << 2,
    SD_CARD_STATE_FLIGHTLOGGING = 1 << 3, //for Testdata
} SD_CARD_STATE;

typedef struct {
    uint64_t freeSpace;
    uint64_t maxSpace;
    SD_CARD_STATE status;
} sd_card_state_t;

typedef struct {
    float batteryPercentage; //from 0 to 100
    gps_fix_mode_t statusGPS;
    sd_card_state_t status_sd_card;

    uint16_t stateChange;
} device_state_t;

typedef enum {
    device_state_batteryPercentage = 1 << 0, //from 0 to 100
    device_state_statusGPS = 1 << 1,
    device_state_statusSDCard = 1 << 2,
} device_state_member_t;

typedef struct {
    float altitudeMax;
    float altitudeMin;

    float maxVelX;
    float minVelX;

    uint16_t stateChange;
} flight_stats_t;

void init_DeviceContext(nmea_parser_handle_t nmea_parser_handle);

/*
* Vario State. value has to be the right type for the returnValue!!!
*/
void vario_state_set_member(void* value, vario_state_member_t member);
bool vario_state_get_member(void* value, vario_state_member_t member, bool stateChange);
/*
* Device Settings
*/
void vario_settings_set_member(void* value, vario_settings_member_t member);
bool vario_settings_get_member(void* value, vario_settings_member_t member, bool stateChange);
/*
* Device State
*/
void device_state_set_member(void* value, device_state_member_t member);
bool device_state_get_member(void* value, device_state_member_t member, bool stateChange);