#ifndef LOGGER_VARIO_H
#define LOGGER_VARIO_H

#include "SDCard.h"
#include "../main.h"

void startLogging(sdcard_t *card);
void endLogging();

void logVarioData(FlightLogger_t state);

#endif