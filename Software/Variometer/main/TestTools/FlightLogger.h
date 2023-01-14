#ifndef LOGGER_VARIO_H
#define LOGGER_VARIO_H

#include "SDCard.h"
#include "../main.h"
#include "nmea_parser.h"

void startLogging(sdcard_t *card, nmea_parser_handle_t nmea_parser_handle);
void endLogging();

void logVarioData(FlightLogger_t state);

#endif