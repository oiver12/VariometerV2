#include <stdint.h>
#include <stdio.h>
#include "Screen/Util.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "airspaceParcer.h"

#define BOUNDINGBOX_OPTIMIZATION

#define MAXDISTANCETOSEARCH 10000

void airspaceCalculateValues(airspace *nearestAirspace, uint32_t startLineIndex, float myLat, float myLon, float myAlt)
{
    uint32_t endLineIndex = startLineIndex + 1;
    if (nearestAirspace->polygonSize >= endLineIndex)
        endLineIndex = 0;
    float nearestLat, nearestLon = 0;
    NearestPointOnLine(nearestAirspace->polygons[startLineIndex].latitude, nearestAirspace->polygons[startLineIndex].longitude, nearestAirspace->polygons[endLineIndex].latitude, nearestAirspace->polygons[endLineIndex].longitude, myLat, myLon, &nearestLat, &nearestLon);
    float HorDist = GET_DISTANCEFAST(myLat, myLon, nearestLat, nearestLon);
    float VertDist = 0;
    if (myAlt < nearestAirspace->lowerLimit)
    {
        VertDist = nearestAirspace->lowerLimit - myAlt;
    }
    else if (myAlt > nearestAirspace->upperLimit)
    {
        VertDist = myAlt - nearestAirspace->upperLimit;
    }
}

void searchNearestAirspace(double myLat, double myLon, float altitude, airspace *airspaces)
{
    airspace *nearestAirspace = NULL;
    float nearestDistanceToAirspace = MAXDISTANCETOSEARCH;
    uint32_t lineStartIndex = 0;
    while (airspaces != NULL)
    {
        uint32_t tempStartIndex = 0;
        float minDistance = INFINITY;
        for (size_t i = 0; i < airspaces->polygonSize; i++)
        {
            size_t nextI = i + 1;
            if (nextI > airspaces->polygonSize)
                nextI = 0;
            float dist = DistancePointToLineSqr(airspaces->polygons[i].latitude, airspaces->polygons[i].longitude, airspaces->polygons[nextI].latitude, airspaces->polygons[nextI].longitude, myLat, myLon);
            if (dist < minDistance)
            {
                tempStartIndex = i;
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
                lineStartIndex = tempStartIndex;
            }
        }
        airspaces = airspaces->next;
    }
    if (nearestAirspace != NULL)
    {
        printf("Nearest airspace is %s, distance is %f\n", nearestAirspace->name, nearestDistanceToAirspace);
    }
    else
    {
        printf("No airspace found within %d meters\n", MAXDISTANCETOSEARCH);
    }
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

airspace *readPolygonsFirst(FILE *fileptr, bool *hasNext, BoundingBox_LatLong_t *myBoundingBox)
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
    airspace *tempAirspace = calloc(sizeof(airspace), 1);
    tempAirspace->polygonSize = polygonSize;

    fseek(fileptr, posPolyStart, SEEK_SET);

    tempAirspace->polygons = calloc(sizeof(polygonSide), polygonSize);
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

void airspaceParse()
{
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

    airspace *headerAirspace = NULL;
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
        if (strcmp(nameBuffer, "LS-R31 Emmen east") == 0)
            printf("Now");
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

        if (strcmp(nameBuffer, "TMA Zurich 13") == 0)
        {
            printf("Heress\n");
        }
        airspace *tempAirspace = readPolygonsFirst(fileptr, &hasNext, &myBoundingBox);
        if (!tempAirspace)
            continue;

        airspacesCount++;

        if (!headerAirspace)
            headerAirspace = tempAirspace;
        tempAirspace->lowerLimit = lowerLimit;
        tempAirspace->upperLimit = uppperLimit;
        tempAirspace->heightTypeLower = heightTypeLower;
        tempAirspace->heightTypeUpper = heightTypeUpper;
        tempAirspace->name = calloc(strlen(nameBuffer) + 1, 1);
        strcpy(tempAirspace->name, nameBuffer);

        // bool isEnclosed = isEnclosedByPolygon(tempAirspace->polygons, tempAirspace->polygonSize, myLat, myLon);

        if (prev)
        {
            prev->next = tempAirspace;
        }
        prev = tempAirspace;

        hasNext = readTilCharacter(fileptr, '{');
    }
    searchNearestAirspace(myLat, myLon, 0, headerAirspace);
    fclose(fileptr);
}