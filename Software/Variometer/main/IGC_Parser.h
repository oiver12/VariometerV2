#ifndef __IGC_PARSER_H
#define __IGC_PARSER_H

#include "Util.h"
#include "nmea_parser.h"
#include <stdio.h>
#include "DeviceContext.h"

#define MAX_STRING_SIZE 16

#include <stdbool.h> 

typedef enum {
    IGC_FXA,
    IGC_VXA,
    IGC_GSP,
    IGC_CCO,
    IGC_HDT,
    IGC_UNKNOWN
} IFieldTypeIGC;

//translated from go in c from: https://github.com/ezgliding/goigc
typedef struct HeaderIGC_t
{
    //TODO: add all Headers
    char manufacture[MAX_STRING_SIZE];
    char model[MAX_STRING_SIZE];
} HeaderIGC_t;

typedef struct TimeIGC_t {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} TimeIGC_t;

typedef int64_t IFieldPointIGC_t;

typedef struct TrackPointIGC_t {
    IFieldPointIGC_t* IFields;
    TimeIGC_t time;
    bool FixValidity;
    int pressureAlt;
    int gnssAlt;
    float lat;
    float lon; 
} TrackPointIGC_t;

typedef struct IFieldIGC_t {
    uint8_t posStart;
    uint8_t posEnd;
    IFieldTypeIGC type;
} IFieldIGC_t;

typedef struct Track_t
{
    HeaderIGC_t header;
    flight_stats_t flightStats;
    TrackPointIGC_t* trackPoints;
    size_t sizeTrackPoints;
    IFieldIGC_t* IFields;
    size_t sizeIFields;
} Track_t;

int ParseIGCFile(Track_t* track, size_t maxSizeTrackPoints, FILE* igcFile);
void Get_duration_diff(TimeIGC_t start_time, TimeIGC_t end_time, uint8_t *hours, uint8_t *minutes);


#endif