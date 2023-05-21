#ifndef __AIRSPACEPARCER__H
#define __AIRSPACEPARCER__H

#include "Screen/Util.h"
#include "Screen/DeviceContext.h"

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
    char name[100];
    uint16_t upperLimt;
    heightType heightTypeUpper;
    uint16_t lowerLimit;
    heightType heightTypeLower;
    float distanceToAirspace;
} Airspace_Info_t;

typedef struct AirspaceParcer_State_t
{
    airspace *nearestAirspace;
    uint64_t lastLineUpdate;
    Position_t nearestLineStart;
    Position_t nearestLineEnd;
    Position_t lastInNearestAirspaceUpdatePoint;
    Position_t lastInNearAirspaceUpdatePoint;
    Position_t lastAllAirspaceUpdatePoint;
    float diffFirstToSecondAirspace;
    Airspace_Info_t nearestAirspaceInfo;
} AirspaceParcer_State_t;

void startAirspaceParse();
void airspaceParse();

#endif