#ifndef __SIMULATIONDATA_H__
#define __SIMULATIONDATA_H__

#include <stdio.h>
#include <esp_event.h>
#include <esp_err.h>

typedef enum
{
    SIMULATION_SDCARD,
    SIMULATION_SERIAL
} simulation_type_t;

void simulation_start(char *name, simulation_type_t simulationType);
esp_err_t simulation_nextPressure(float *pressure); // should be called every 30ms
esp_err_t simulation_nextAcc(float *acc);           // returns the same acc for one pressure
esp_err_t simulation_addGPSHandle(esp_event_handler_t eventHandler);

#endif