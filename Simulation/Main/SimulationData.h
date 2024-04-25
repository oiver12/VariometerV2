#ifndef __SIMULATIONDATA_H__
#define __SIMULATIONDATA_H__

#include "DPS310.h"
#include "BMI088.h"
#include "QMC588L.h"

#include <stdio.h>
#include <esp_event.h>
#include <esp_err.h>

void simulation_start(char *varioname, char *gpsName);
esp_err_t simulation_nextPressure(dps310_t *dps); // should be called every 30ms
esp_err_t simulation_nextAcc(bmi088_t *bmi);      // returns the same acc for one pressure
esp_err_t simulation_nextMagn(qmc588l_t *qmc);    // returns the same magn for one pressure
esp_err_t simulation_addGPSHandle(esp_event_handler_t eventHandler);

#endif