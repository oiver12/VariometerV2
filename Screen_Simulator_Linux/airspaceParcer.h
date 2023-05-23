#ifndef __AIRSPACEPARCER__H
#define __AIRSPACEPARCER__H

#include "Screen/Util.h"
#include "Screen/DeviceContext.h"

#define AIRSPACE_NAMEBUFFERSIZE 100

typedef enum
{
    HeightType_mQNH = 1,
    HeightType_mSTD = 2,
    HeightType_mAGL = 3
} heightType;

typedef struct
{
    char name[100];
    uint16_t upperLimit;
    heightType heightTypeUpper;
    uint16_t lowerLimit;
    heightType heightTypeLower;
    polygonSide *polygons;
    size_t polygonSize;
    void *next;
    bool isValid;
} airspace;

typedef struct Airspace_Info_t
{
    char name[AIRSPACE_NAMEBUFFERSIZE];
    uint16_t upperLimt;
    heightType heightTypeUpper;
    uint16_t lowerLimit;
    heightType heightTypeLower;
    float distanceToAirspaceVert;
    float distanceToAirspaceHor;
    Position_t nearestLineStart;
    Position_t nearestLineEnd;
} Airspace_Info_t;

typedef struct AirspaceParcer_State_t
{
    airspace *nearestAirspaceList;
    airspace *currentNearestAirspace;
    Airspace_Info_t nearestAirspaceInfo;
    uint64_t lastLineUpdate;
    Position_t lastInNearestAirspaceUpdatePoint; // Point where nearestLine was found in the nearest Airspace
    Position_t lastInNearAirspacesUpdatePoint;   // Point where all near Airspaces where searched and the nearest was picked
    Position_t lastAllAirspaceUpdatePoint;       // Point where whole JSON File was last parsed
    float diffFirstToSecondAirspace;
} AirspaceParcer_State_t;

void startAirspaceParse();

#endif