#include "IGC_Parser.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LINELENGTH 256

#define ERROR_CHECK(val) ({if(val != 0) return 1;})

int str2int(int *out, const char *s) {
    char *end;
    if (s[0] == '\0' || isspace((unsigned char)s[0]))
        return 1;
    errno = 0;
    long l = strtol(s, &end, 10);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    //Owerflow
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return 1;
    //Underflow
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return 1;
    if (*end != '\0')
        return 1;
    *out = l;
    return 0;
}

static void sliceString(const char* oldString, char* newString, int start, int size)
{
    strncpy(newString, &oldString[start], size);
    newString[size] = '\0';
}

static int floatFROMDMD(const char* dmd, float* valueRad)
{
    char tempBuffer[9];
    size_t sizeString = strlen(dmd);
    if (sizeString != 8 && sizeString != 9)
    {
        printf("Can't parse dmd\n");
        return 1;
    }
    int degress, minutes, dminutes;
    if (dmd[0] == 'S' || dmd[0] == 'N') 
    {
        sliceString(dmd, tempBuffer, 1, 2);
        ERROR_CHECK(str2int(&degress, tempBuffer));
        sliceString(dmd, tempBuffer, 3, 2);
        ERROR_CHECK(str2int(&minutes, tempBuffer));
        sliceString(dmd, tempBuffer, 5, 2);
        ERROR_CHECK(str2int(&dminutes, tempBuffer));
    }
    else if (dmd[sizeString-1] == 'S' || dmd[sizeString-1] == 'N')
    {
        sliceString(dmd, tempBuffer, 0, 2);
        ERROR_CHECK(str2int(&degress, tempBuffer));
        sliceString(dmd, tempBuffer, 2, 2);
        ERROR_CHECK(str2int(&minutes, tempBuffer));
        sliceString(dmd, tempBuffer, 4, 2);
        ERROR_CHECK(str2int(&dminutes, tempBuffer));
    }
    else if (dmd[0] == 'W' || dmd[0] == 'E')
    {
        sliceString(dmd, tempBuffer, 1, 3);
        ERROR_CHECK(str2int(&degress, tempBuffer));
        sliceString(dmd, tempBuffer, 4, 2);
        ERROR_CHECK(str2int(&minutes, tempBuffer));
        sliceString(dmd, tempBuffer, 6, 3);
        ERROR_CHECK(str2int(&dminutes, tempBuffer));
    }
    else if (dmd[sizeString-1] == 'W' || dmd[sizeString-1] == 'E') 
    {
        sliceString(dmd, tempBuffer, 0, 3);
        ERROR_CHECK(str2int(&degress, tempBuffer));
        sliceString(dmd, tempBuffer, 3, 2);
        ERROR_CHECK(str2int(&minutes, tempBuffer));
        sliceString(dmd, tempBuffer, 5, 3);
        ERROR_CHECK(str2int(&dminutes, tempBuffer));
    }
    float r = (float)degress + (((float)minutes + ((float)dminutes / 1000.0f)) / 60.0f);
    if (dmd[0] == 'S' || dmd[0] == 'W' || dmd[sizeString-1] == 'S' || dmd[sizeString-1] == 'W')
        r *= -1;
    *valueRad = r;
    return 0;
} 

static int parseTime(const char* timeString, TimeIGC_t* date)
{
    if(strlen(timeString) != 6)
    {
        printf("Can't parse Time\n");
    }
    int hour, minute, second;
    char tempBuffer[4];
    sliceString(timeString, tempBuffer, 0, 2);
    ERROR_CHECK(str2int(&hour, tempBuffer));
    sliceString(timeString, tempBuffer, 2, 2);
    ERROR_CHECK(str2int(&minute, tempBuffer));
    sliceString(timeString, tempBuffer, 4, 2);
    ERROR_CHECK(str2int(&second, tempBuffer));
    date->hour = hour;
    date->minute = minute;
    date->second = second;
    printf("H: %i, M: %i, S: %i \n", hour, minute, second);
    return 0;
}

static int parseB(const char* line, TrackPointIGC_t* point)
{
    if(point == NULL)
    {
        printf("Not enough Heap Space\n");
        return 1;
    }
    if (strlen(line) < 35) 
    {
        printf("Line not enough long\n");
        return 1;
    }
    char tempBuffer[MAX_LINELENGTH];
    //Parse Latitude
    sliceString(line, tempBuffer, 7, 8);
    ERROR_CHECK(floatFROMDMD(tempBuffer, &point->lat));
    //Parse Longitude
    sliceString(line, tempBuffer, 15, 9);
    ERROR_CHECK(floatFROMDMD(tempBuffer, &point->lon));
    //Parse Time
    sliceString(line, tempBuffer, 1, 6);
    ERROR_CHECK(parseTime(tempBuffer, &point->time));
    if(line[24] == 'A')
        point->FixValidity = true;
    else if(line[24] == 'V')
        point->FixValidity = false;
    else
    {
        printf("Invalid fix validity: %s", line);
    }
    //Parse PressureAlt
    sliceString(line, tempBuffer, 25, 5);
    ERROR_CHECK(str2int(&point->pressureAlt, tempBuffer));
    //Parse GNSSAlt
    sliceString(line, tempBuffer, 30, 5);
    ERROR_CHECK(str2int(&point->gnssAlt, tempBuffer));
    //TODO: Parse I fields
    return 0;
}

static int parseI(const char* line, Track_t *track)
{
    if(strlen(line) < 3) 
    {
        return 1;
    }
    char tempBuffer[MAX_LINELENGTH];
    sliceString(line, tempBuffer, 1, 2);
    int n;
    ERROR_CHECK(str2int(&n, tempBuffer));
    
}

void updateFlightStats(TrackPointIGC_t trackPoint, Track_t *track)
{
    if(trackPoint.FixValidity)
    {
        if(track->flightStats.altitudeMax < trackPoint.gnssAlt)
            track->flightStats.altitudeMax = trackPoint.gnssAlt;
        if(track->flightStats.altitudeMin > trackPoint.gnssAlt)
            track->flightStats.altitudeMin = trackPoint.gnssAlt;
    }
}

int ParseIGCFile(Track_t* track, size_t maxSizeTrackPoints, FILE* igcFile)
{   
    char line[MAX_LINELENGTH];
    if (igcFile == NULL)
        return 1;
    memset(&track->flightStats, 0, sizeof(flight_stats_t));
    track->trackPoints = NULL;
    track->sizeTrackPoints = 0;
    TrackPointIGC_t tempPoint;
    size_t totalPoints = 0;
    while (fgets(line, sizeof(line), igcFile)) {
        switch(line[0]) {
            case 'B':
                ERROR_CHECK(parseB(line, &tempPoint));
                updateFlightStats(tempPoint, track);
                totalPoints++;
                break;
            case 'I':

            default:
                break;
        }
    }
    //TODO: Go trough it again. Set trackPoints in track with maxSizeTrackPoints
    //size_t size = sizeof(TrackPointIGC_t) * track->sizeTrackPoints + sizeof(Track_t);
    return 0;
}