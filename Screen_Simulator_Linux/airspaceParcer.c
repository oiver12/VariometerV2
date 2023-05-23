#include <stdint.h>
#include <stdio.h>
#include "Screen/Util.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "airspaceParcer.h"
#include "freertos/FreeRTOS.h"

#define BOUNDINGBOX_OPTIMIZATION

#define MAXDISTANCETOSEARCH 10000

AirspaceParcer_State_t instanceAirspace;

void airspaceCalculateValuesToLine(float myLat, float myLon, float myAlt)
{
    float nearestLat, nearestLon = 0;
    NearestPointOnLine(instanceAirspace.nearestAirspaceInfo.nearestLineStart.lat, instanceAirspace.nearestAirspaceInfo.nearestLineStart.lon, instanceAirspace.nearestAirspaceInfo.nearestLineEnd.lat, instanceAirspace.nearestAirspaceInfo.nearestLineEnd.lon, myLat, myLon, &nearestLat, &nearestLon);
    float HorDist = GET_DISTANCEFAST(myLat, myLon, nearestLat, nearestLon);
    float VertDist = 0;
    if (myAlt < instanceAirspace.currentNearestAirspace->lowerLimit)
    {
        VertDist = instanceAirspace.currentNearestAirspace->lowerLimit - myAlt;
    }
    else if (myAlt > instanceAirspace.currentNearestAirspace->upperLimit)
    {
        VertDist = myAlt - instanceAirspace.currentNearestAirspace->upperLimit;
    }
    strcpy(instanceAirspace.nearestAirspaceInfo.name, instanceAirspace.currentNearestAirspace->name);
    instanceAirspace.nearestAirspaceInfo.upperLimt = instanceAirspace.currentNearestAirspace->upperLimit;
    instanceAirspace.nearestAirspaceInfo.heightTypeUpper = instanceAirspace.currentNearestAirspace->heightTypeUpper;
    instanceAirspace.nearestAirspaceInfo.lowerLimit = instanceAirspace.currentNearestAirspace->lowerLimit;
    instanceAirspace.nearestAirspaceInfo.heightTypeLower = instanceAirspace.currentNearestAirspace->heightTypeLower;
    instanceAirspace.nearestAirspaceInfo.distanceToAirspaceHor = HorDist;
    instanceAirspace.nearestAirspaceInfo.distanceToAirspaceVert = VertDist;
}

void nearestLineAirspace(double myLat, double myLon, float altitude)
{

    uint32_t tempStartIndex = 0;
    uint32_t tempEndIndex = 0;
    float minDistance = INFINITY;
    for (size_t i = 0; i < instanceAirspace.currentNearestAirspace->polygonSize; i++)
    {
        size_t nextI = i + 1;
        if (nextI >= instanceAirspace.currentNearestAirspace->polygonSize)
            nextI = 0;
        float dist = DistancePointToLineSqr(instanceAirspace.currentNearestAirspace->polygons[i].latitude, instanceAirspace.currentNearestAirspace->polygons[i].longitude, instanceAirspace.currentNearestAirspace->polygons[nextI].latitude, instanceAirspace.currentNearestAirspace->polygons[nextI].longitude, myLat, myLon);
        if (dist < minDistance)
        {
            tempStartIndex = i;
            tempEndIndex = nextI;
            minDistance = dist;
        }
    }
    instanceAirspace.nearestAirspaceInfo.nearestLineStart.lat = instanceAirspace.currentNearestAirspace->polygons[tempStartIndex].latitude;
    instanceAirspace.nearestAirspaceInfo.nearestLineStart.lon = instanceAirspace.currentNearestAirspace->polygons[tempStartIndex].longitude;
    instanceAirspace.nearestAirspaceInfo.nearestLineEnd.lat = instanceAirspace.currentNearestAirspace->polygons[tempEndIndex].latitude;
    instanceAirspace.nearestAirspaceInfo.nearestLineEnd.lon = instanceAirspace.currentNearestAirspace->polygons[tempEndIndex].longitude;
}

void searchInNearestAirspace(double myLat, double myLon, float altitude)
{
    airspace *nearestAirspace = NULL;
    float nearestDistanceToAirspace = MAXDISTANCETOSEARCH;
    uint32_t lineStartIndex = 0;
    airspace *airspaces = instanceAirspace.nearestAirspaceList;
    while (airspaces != NULL)
    {
        float minDistance = INFINITY;
        for (size_t i = 0; i < airspaces->polygonSize; i++)
        {
            size_t nextI = i + 1;
            if (nextI >= airspaces->polygonSize)
                nextI = 0;
            float dist = DistancePointToLineSqr(airspaces->polygons[i].latitude, airspaces->polygons[i].longitude, airspaces->polygons[nextI].latitude, airspaces->polygons[nextI].longitude, myLat, myLon);
            if (dist < minDistance)
            {
                minDistance = dist;
            }
        }
        minDistance = sqrtf(minDistance);
        if (minDistance < nearestDistanceToAirspace)
        {
            if (altitude < airspaces->lowerLimit)
            {
                minDistance += airspaces->lowerLimit - altitude;
            }
            else if (altitude > airspaces->upperLimit)
            {
                minDistance += (altitude - airspaces->upperLimit);
            }

            if (minDistance < nearestDistanceToAirspace)
            {
                nearestDistanceToAirspace = minDistance;
                nearestAirspace = airspaces;
            }
        }
        airspaces = airspaces->next;
    }
    instanceAirspace.currentNearestAirspace = nearestAirspace;
}

bool readTilCharacter(FILE *file, char character)
{
    char c;
    while ((c = fgetc(file)) != character && (int)c != EOF)
        ;
    {
    }
    return (int)c != EOF;
}

bool readTilCharacterTimes(FILE *file, char character, uint8_t times)
{
    char c;
    for (size_t i = 0; i < times; i++)
    {
        while ((c = fgetc(file)) != character && (int)c != EOF)
            ;
        {
        }
        if ((int)c == EOF)
            return false;
    }
    return true;
}

void readToBufferTillNextChar(FILE *file, char character, char *buffer, int bufferSize)
{
    char c;
    int i = 0;
    while ((c = fgetc(file)) != character)
    {
        if (i < bufferSize - 1)
        {
            buffer[i] = c;
            i++;
        }
        else
        {
            printf("Name Buffer to small!\n");
            break;
        }
    }
    buffer[i] = '\0';
}

static airspace *createAirspaceOnHeap(size_t polygonSize)
{
    airspace *newAirspace = calloc(sizeof(airspace), 1);
    newAirspace->polygons = calloc(sizeof(polygonSide), polygonSize);
    return newAirspace;
}

static bool existsInLinkedList(char *name)
{
    airspace *tempAirspace = instanceAirspace.nearestAirspaceList;
    while (tempAirspace)
    {
        if (strcmp(name, tempAirspace->name))
            return true;
    }
    return false;
}

airspace *readPolygonsFirst(FILE *fileptr, bool *hasNext, BoundingBox_LatLong_t *myBoundingBox, char *nameAirspace)
{
    char nextChar = ',';
    int polygonSize = 0;
    long posPolyStart = ftell(fileptr);
    BoundingBox_LatLong_t boundingBoxAirspace = {.maxLat = 0, .maxLong = 0, .minLat = INFINITY, .minLong = INFINITY};
    while (nextChar == ',')
    {
        polygonSize++;
        readTilCharacter(fileptr, '[');
        float lonitude;
        fscanf(fileptr, "%f", &lonitude);
        boundingBoxAirspace.maxLong = fmax(boundingBoxAirspace.maxLong, lonitude);
        boundingBoxAirspace.minLong = fmin(boundingBoxAirspace.minLong, lonitude);
        fgetc(fileptr);
        float latitude;
        fscanf(fileptr, "%f", &latitude);
        boundingBoxAirspace.maxLat = fmax(boundingBoxAirspace.maxLat, latitude);
        boundingBoxAirspace.minLat = fmin(boundingBoxAirspace.minLat, latitude);

        fgetc(fileptr);
        nextChar = fgetc(fileptr);
    }
    if (overlappBoundingBox(myBoundingBox, &boundingBoxAirspace))
    {
        *hasNext = readTilCharacter(fileptr, '{');
        return NULL;
    }
    airspace *tempAirspace = instanceAirspace.nearestAirspaceList;
    while (tempAirspace)
    {
        // Airspace does not yet exist append to end
        if (!tempAirspace->next)
        {
            airspace *newAirspace = createAirspaceOnHeap(polygonSize);
            tempAirspace->next = newAirspace;
            tempAirspace = tempAirspace->next;
            break;
        }
        tempAirspace = tempAirspace->next;
    }
    // No Airspace does yet exist at all
    if (!tempAirspace)
    {
        tempAirspace = createAirspaceOnHeap(polygonSize);
    }
    tempAirspace->isValid = true;
    tempAirspace->polygonSize = polygonSize;

    fseek(fileptr, posPolyStart, SEEK_SET);

    int i = 0;
    nextChar = ',';
    while (nextChar == ',')
    {
        polygonSide *tempPolygon = &tempAirspace->polygons[i];
        i++;
        readTilCharacter(fileptr, '[');
        fscanf(fileptr, "%f", &tempPolygon->longitude);
        fgetc(fileptr);
        fscanf(fileptr, "%f", &tempPolygon->latitude);

        fgetc(fileptr);
        nextChar = fgetc(fileptr);
    }
    *hasNext = true;
    return tempAirspace;
}

void deInitAirspaceList()
{
}

void airspaceParse()
{
    airspace *listNearest = instanceAirspace.nearestAirspaceList;
    while (listNearest)
    {
        listNearest->isValid = false;
        listNearest = listNearest->next;
    }
    double myLat = 47.429665;
    double myLon = 9.388993;

    FILE *fileptr;
    fileptr = fopen("airspace/airspaces.json", "rb");
    if (!fileptr)
    {
        printf("Error opening file");
        return;
    }
    readTilCharacter(fileptr, '{');
    char nameBuffer[50];
    char heightBuffer[10];
    readTilCharacter(fileptr, '{');
    bool hasNext = true;
    uint32_t airspacesCount = 0;

    airspace *prev = NULL;
    uint32_t airspacesCounted = 0;
    BoundingBox_LatLong_t myBoundingBox;
    boundingBoxFromPoint(&myBoundingBox, myLat, myLon, MAXDISTANCETOSEARCH);
    while (hasNext)
    {
        airspacesCounted++;
        readTilCharacter(fileptr, '{');
        readTilCharacter(fileptr, ':');
        fgetc(fileptr);
        readToBufferTillNextChar(fileptr, '"', nameBuffer, 50);
        readTilCharacterTimes(fileptr, ',', 6);
        fgetc(fileptr);
        char temp = fgetc(fileptr);
        // Check if Freqeuncy Field exists
        if (temp == 'F')
            readTilCharacter(fileptr, ',');
        fgetc(fileptr);
        char temp1 = fgetc(fileptr);
        // check if Callsign Field exists
        if (temp1 == 'C' || temp == 'C')
            readTilCharacter(fileptr, ',');
        readTilCharacter(fileptr, ',');
        fgetc(fileptr);
        temp = fgetc(fileptr);
        // Check if Validity Field exists
        // TODO: save the validity
        if (temp == 'V')
        {
            readTilCharacter(fileptr, '{');
        }
        readTilCharacterTimes(fileptr, '{', 3);
        readTilCharacter(fileptr, ':');
        fgetc(fileptr);
        heightType heightTypeUpper;
        readToBufferTillNextChar(fileptr, '"', heightBuffer, 10);
        if (strcmp(heightBuffer, "m QNH") == 0)
        {
            heightTypeUpper = HeightType_mQNH;
        }
        else if (strcmp(heightBuffer, "m AGL") == 0)
        {
            heightTypeUpper = HeightType_mAGL;
        }
        else if (strcmp(heightBuffer, "m STD") == 0)
        {
            heightTypeUpper = HeightType_mSTD;
        }
        else
        {
            printf("Unknown height type %s\n", heightBuffer);
            return;
        }
        readTilCharacter(fileptr, ':');
        int uppperLimit;
        fscanf(fileptr, "%i", &uppperLimit);
        readTilCharacter(fileptr, '{');
        fgetc(fileptr);
        temp = fgetc(fileptr);
        // TODO: Save Alt2
        if (temp == 'T')
        {
            readTilCharacter(fileptr, '{');
            readTilCharacter(fileptr, '}');
        }
        readTilCharacter(fileptr, '}');
        readTilCharacter(fileptr, '{');
        readTilCharacter(fileptr, '{');
        readTilCharacter(fileptr, '{');
        readTilCharacter(fileptr, ':');
        fgetc(fileptr);
        readToBufferTillNextChar(fileptr, '"', heightBuffer, 10);
        heightType heightTypeLower;
        if (strcmp(heightBuffer, "m QNH") == 0)
        {
            heightTypeLower = HeightType_mQNH;
        }
        else if (strcmp(heightBuffer, "m AGL") == 0)
        {
            heightTypeLower = HeightType_mAGL;
        }
        else if (strcmp(heightBuffer, "m STD") == 0)
        {
            heightTypeLower = HeightType_mSTD;
        }
        else
        {
            printf("Unknown height type %s\n", heightBuffer);
            return;
        }
        readTilCharacter(fileptr, ':');
        int lowerLimit;
        fscanf(fileptr, "%i", &lowerLimit);
        readTilCharacter(fileptr, '[');
        readTilCharacter(fileptr, '[');

        airspace *tempAirspace = readPolygonsFirst(fileptr, &hasNext, &myBoundingBox, nameBuffer);
        if (!tempAirspace)
            continue;

        airspacesCount++;

        if (!instanceAirspace.nearestAirspaceList)
            instanceAirspace.nearestAirspaceList = tempAirspace;
        tempAirspace->lowerLimit = lowerLimit;
        tempAirspace->upperLimit = uppperLimit;
        tempAirspace->heightTypeLower = heightTypeLower;
        tempAirspace->heightTypeUpper = heightTypeUpper;

        if (prev)
        {
            prev->next = tempAirspace;
        }
        prev = tempAirspace;

        hasNext = readTilCharacter(fileptr, '{');
    }
    listNearest = instanceAirspace.nearestAirspaceList;
    airspace *prev = NULL;
    while (listNearest)
    {
        if (!listNearest->isValid)
        {
            // Delete from LinkedList
            if (!prev) // First Item to delete
                instanceAirspace.nearestAirspaceList = listNearest->next;
            else // Delete other item
                prev->next = listNearest->next;
            airspace *oldObj = listNearest;
            listNearest = listNearest->next;
            free(oldObj);
            continue;
        }
        prev = listNearest;
        listNearest = listNearest->next;
    }
    // searchInNearestAirspace(myLat, myLon, 0, headerAirspace);
    fclose(fileptr);
}

void AirspaceTask()
{
    gps_t gpsData;
    vario_state_get_member(&gpsData, vario_state_gps_data, false);
    gpsData.valid = true;
    while (!gpsData.valid)
    {
        // vTaskDelay(1000 / vPorttickper);
        vario_state_get_member(&gpsData, vario_state_gps_data, false);
    }
    instanceAirspace.lastAllAirspaceUpdatePoint.lat = gpsData.latitude;
    instanceAirspace.lastAllAirspaceUpdatePoint.lon = gpsData.longitude;
    airspaceParse();
    searchInNearestAirspace(gpsData.latitude, gpsData.longitude, gpsData.altitude);
    nearestLineAirspace(gpsData.latitude, gpsData.longitude, gpsData.altitude);
    airspaceCalculateValuesToLine(gpsData.latitude, gpsData.longitude, gpsData.altitude);
    while (true)
    {
    }
}

void startAirspaceParse()
{
    // TODO: Request Airspace Update, if the file is an old one --> download from API SHV Airspace
}