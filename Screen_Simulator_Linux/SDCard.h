#ifndef SDCARD_VARIO_H
#define SDCARD_VARIO_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdint.h>

#define MAX_FILENAME 100

typedef struct FlightFile_t {
    char fileName[MAX_FILENAME];
    long timeStamp;
} FlightFile_t;

//foundAfterStart stops after 6
void GetFlights(int startIndex, FlightFile_t flightFile[6], uint16_t* foundAfterStart);

#endif