#ifndef AGL_H
#define AGL_H 

#include "SDCard.h"
#include "nmea_parser.h"

typedef struct {
    int mLong;
    int mLat;
    FILE *currFile;
    sdcard_t *sdcard;
    nmea_parser_handle_t nmea_parser_handle;
} AGLContext_t;

void AGL_startAGL(sdcard_t *sdcard, nmea_parser_handle_t nmea_parser_handle);

#endif