#ifndef __ERRORHANDLING_VARIO_H
#define __ERRORHANDLING_VARIO_H

#include <esp_err.h>

typedef enum error_location
{
    ERR_AIRSPACEPARCER,
    ERR_BLE,
    ERR_IGC,
    ERR_SCREEN,
    ERR_BMI088,
    ERR_DPS310,
    ERR_QMC588L,
    ERR_AGL,
    ERR_BATTERY,
    ERR_BUTTONS,
    ERR_FLASH,
    ERR_MAIN,
    ERR_MAINTASKS,
    ERR_NMEA,
    ERR_SDCARD,
    ERR_SOUNDMANAGER
} error_location;

void error_handle(esp_err_t err, error_location location, char *message);

#endif