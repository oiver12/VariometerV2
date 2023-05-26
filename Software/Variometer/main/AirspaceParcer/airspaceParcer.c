#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "airspaceParcer.h"
#include "file_layer.h"
#include "freertos/FreeRTOS.h"
#include "../SDCard.h"
#include <esp_timer.h>

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
    float secondNearestDistanceToAirspace = INFINITY;
    airspace *airspaces = instanceAirspace.nearestAirspaceList;
    if (airspaces == NULL)
        printf("Is null!!!!\n");
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
        bool hasUpdated = false;
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
                hasUpdated = true;
                if (secondNearestDistanceToAirspace > nearestDistanceToAirspace)
                    secondNearestDistanceToAirspace = nearestDistanceToAirspace;
                nearestDistanceToAirspace = minDistance;
                nearestAirspace = airspaces;
            }
        }
        if (!hasUpdated && minDistance < secondNearestDistanceToAirspace)
        {
            secondNearestDistanceToAirspace = minDistance;
        }
        airspaces = airspaces->next;
    }
    instanceAirspace.currentNearestAirspace = nearestAirspace;
    instanceAirspace.diffFirstToSecondAirspace = MAXDISTANCETOSEARCH;
    if (secondNearestDistanceToAirspace != INFINITY)
        instanceAirspace.diffFirstToSecondAirspace = secondNearestDistanceToAirspace - nearestDistanceToAirspace;
    instanceAirspace.lastInNearAirspacesUpdatePoint.lat = myLat;
    instanceAirspace.lastInNearAirspacesUpdatePoint.lon = myLon;
}

static airspace *createAirspaceOnHeapOrInArray(size_t polygonSize)
{
    airspace *tempAirspace;
    if (instanceAirspace.currentIndexAirspaces < PREALLOCATEDAIRSPACES)
    {
        tempAirspace = &instanceAirspace.nearestAirspaceList[instanceAirspace.currentIndexAirspaces];
        tempAirspace->isAllocated = false;
        instanceAirspace.currentIndexAirspaces++;
    }
    else
    {
        tempAirspace = calloc(sizeof(airspace), 1);
        tempAirspace->isAllocated = true;
    }
    if (polygonSize > PREALLOCATEDPOLYGONS)
    {
        tempAirspace->allocatedPolygons = calloc(sizeof(polygonSide), polygonSize - PREALLOCATEDPOLYGONS);
    }
    return tempAirspace;
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

airspace *readPolygonsFirst(bool *hasNext, BoundingBox_LatLong_t *myBoundingBox, char *nameAirspace, FILE *cachedFile, long airspaceStartPos)
{
    char nextChar = ',';
    int polygonSize = 0;
    long posPolyStart = ftellFileLayer(&instanceAirspace.layer);
    BoundingBox_LatLong_t boundingBoxAirspace = {.maxLat = 0, .maxLong = 0, .minLat = INFINITY, .minLong = INFINITY};
    while (nextChar == ',')
    {
        polygonSize++;
        readTilCharFileLayer(&instanceAirspace.layer, '[');
        float lonitude;
        getFloatSize(&instanceAirspace.layer, 16, &lonitude);
        skipNextFileLayer(&instanceAirspace.layer);
        boundingBoxAirspace.maxLong = fmax(boundingBoxAirspace.maxLong, lonitude);
        boundingBoxAirspace.minLong = fmin(boundingBoxAirspace.minLong, lonitude);
        float latitude;
        getFloatSize(&instanceAirspace.layer, 16, &latitude);
        boundingBoxAirspace.maxLat = fmax(boundingBoxAirspace.maxLat, latitude);
        boundingBoxAirspace.minLat = fmin(boundingBoxAirspace.minLat, latitude);
        skipNextFileLayer(&instanceAirspace.layer);
        nextChar = fgetCFileLayer(&instanceAirspace.layer);
    }
    if (cachedFile != NULL)
    {
        printf("Write: %.2f, %.2f, %.2f, %.2f %i\n", boundingBoxAirspace.maxLat, boundingBoxAirspace.maxLong, boundingBoxAirspace.minLat, boundingBoxAirspace.minLong, (int)airspaceStartPos);
        fwrite(&boundingBoxAirspace.maxLat, sizeof(float), 1, cachedFile);
        fwrite(&boundingBoxAirspace.maxLong, sizeof(float), 1, cachedFile);
        fwrite(&boundingBoxAirspace.minLat, sizeof(float), 1, cachedFile);
        fwrite(&boundingBoxAirspace.minLong, sizeof(float), 1, cachedFile);
        fwrite(&airspaceStartPos, sizeof(long), 1, cachedFile);
    }
    if (overlappBoundingBox(myBoundingBox, &boundingBoxAirspace))
    {
        *hasNext = readTilCharFileLayer(&instanceAirspace.layer, '{');
        return NULL;
    }
    airspace *tempAirspace = &instanceAirspace.nearestAirspaceList[0];
    while (tempAirspace)
    {
        // Already exists
        if (strcmp(nameAirspace, tempAirspace->name) == 0)
        {
            tempAirspace->isValid = true;
            *hasNext = readTilCharFileLayer(&instanceAirspace.layer, '{');
            return NULL;
        }
        // Airspace does not yet exist append to end
        if (!tempAirspace->next)
        {
            airspace *newAirspace = createAirspaceOnHeapOrInArray(polygonSize);
            tempAirspace->next = newAirspace;
            tempAirspace = tempAirspace->next;
            tempAirspace->next = NULL;
            break;
        }
        tempAirspace = tempAirspace->next;
    }
    // No Airspace does yet exist at all
    if (!tempAirspace)
    {
        tempAirspace = createAirspaceOnHeapOrInArray(polygonSize);
        tempAirspace->next = NULL;
    }
    tempAirspace->isValid = true;
    tempAirspace->polygonSize = polygonSize;

    startFileLayer(&instanceAirspace.layer, posPolyStart, instanceAirspace.layer.f);

    int i = 0;
    nextChar = ',';
    while (nextChar == ',')
    {
        polygonSide *tempPolygon;
        if (i < PREALLOCATEDPOLYGONS)
            tempPolygon = &tempAirspace->polygons[i];
        else
            tempPolygon = &tempAirspace->allocatedPolygons[i - PREALLOCATEDPOLYGONS];
        i++;
        readTilCharFileLayer(&instanceAirspace.layer, '[');
        getFloatSize(&instanceAirspace.layer, 16, &tempPolygon->longitude);
        skipNextFileLayer(&instanceAirspace.layer);
        getFloatSize(&instanceAirspace.layer, 16, &tempPolygon->latitude);

        skipNextFileLayer(&instanceAirspace.layer);
        nextChar = fgetCFileLayer(&instanceAirspace.layer);
    }
    *hasNext = true;
    return tempAirspace;
}

void deInitAirspaceList()
{
}

airspace *readSingleAirspaceFromLayer(int *error, bool *hasNext, BoundingBox_LatLong_t *myBoundingBox, FILE *cachedFile, long airspaceStarPos)
{
    char nameBuffer[50];
    char heightBuffer[10];
    readTilCharFileLayer(&instanceAirspace.layer, '{');
    readToBufferTillNextCharFileLayer(&instanceAirspace.layer, ':', nameBuffer, 50);
    if (strcmp(nameBuffer, "\"Name\"") != 0)
    {
        printf("Not in the right position\n");
        *error = 1;
        return NULL;
    }
    skipNextFileLayer(&instanceAirspace.layer);
    readToBufferTillNextCharFileLayer(&instanceAirspace.layer, '"', nameBuffer, 50);
    char nextChar = fgetCFileLayer(&instanceAirspace.layer);
    while (1)
    {
        skipNextFileLayer(&instanceAirspace.layer);
        nextChar = fgetCFileLayer(&instanceAirspace.layer);
        if (nextChar == 'V')
        {
            // TODO: parse Validity
            readTilCharFileLayer(&instanceAirspace.layer, '{');
            readTilCharFileLayer(&instanceAirspace.layer, '{');
            break;
        }
        if (nextChar == 'U')
        {
            readTilCharFileLayer(&instanceAirspace.layer, '{');
            break;
        }
        readTilCharFileLayer(&instanceAirspace.layer, ',');
    }
    readTilCharTimesFileLayer(&instanceAirspace.layer, '{', 2);
    readTilCharFileLayer(&instanceAirspace.layer, ':');
    skipNextFileLayer(&instanceAirspace.layer);
    heightType heightTypeUpper;
    readToBufferTillNextCharFileLayer(&instanceAirspace.layer, '"', heightBuffer, 10);
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
        *error = 1;
        return NULL;
    }
    readTilCharFileLayer(&instanceAirspace.layer, ':');
    int uppperLimit;
    getIntTilCharacter(&instanceAirspace.layer, '}', &uppperLimit);
    readTilCharFileLayer(&instanceAirspace.layer, '{');
    skipNextFileLayer(&instanceAirspace.layer);
    char temp = fgetCFileLayer(&instanceAirspace.layer);
    // TODO: Save Alt2
    if (temp == 'T')
    {
        readTilCharFileLayer(&instanceAirspace.layer, '{');
        readTilCharFileLayer(&instanceAirspace.layer, '}');
    }
    readTilCharFileLayer(&instanceAirspace.layer, '}');
    readTilCharTimesFileLayer(&instanceAirspace.layer, '{', 3);
    readTilCharFileLayer(&instanceAirspace.layer, ':');
    skipNextFileLayer(&instanceAirspace.layer);
    readToBufferTillNextCharFileLayer(&instanceAirspace.layer, '"', heightBuffer, 10);
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
        *error = 1;
        return NULL;
    }
    readTilCharFileLayer(&instanceAirspace.layer, ':');
    int lowerLimit;
    getIntTilCharacter(&instanceAirspace.layer, '}', &lowerLimit);
    readTilCharTimesFileLayer(&instanceAirspace.layer, '[', 2);
    airspace *tempAirspace = readPolygonsFirst(hasNext, myBoundingBox, nameBuffer, cachedFile, airspaceStarPos);
    if (!tempAirspace)
    {
        *error = 0;
        return NULL;
    }

    tempAirspace->lowerLimit = lowerLimit;
    tempAirspace->upperLimit = uppperLimit;
    tempAirspace->heightTypeLower = heightTypeLower;
    tempAirspace->heightTypeUpper = heightTypeUpper;
    if (strlen(nameBuffer) > AIRSPACE_NAMEBUFFERSIZE)
    {
        printf("Can not save NameBuffer: To Large for buffer\n");
        *error = 1;
        return NULL;
    }
    strcpy(tempAirspace->name, nameBuffer);
    *error = 0;
    return tempAirspace;
}

int ParseAirspaceJSON(float myLat, float myLon, FILE *cachedFile, FILE *jsonFile)
{
    startFileLayer(&instanceAirspace.layer, 0, jsonFile);
    printf("Start parse with: %.2f, %.2f", myLat, myLon);
    airspace *listNearest = instanceAirspace.nearestAirspaceList;
    while (listNearest)
    {
        listNearest->isValid = false;
        listNearest = listNearest->next;
    }

    // FILE *fileptr = openFile_sdcard("/airspace", "airspaces.json", "r");
    if (!jsonFile)
    {
        printf("Error opening file\n");
        return 1;
    }
    readTilCharFileLayer(&instanceAirspace.layer, '{');
    readTilCharFileLayer(&instanceAirspace.layer, '{');
    bool hasNext = true;
    uint32_t airspacesCount = 0;

    airspace *prev = NULL;
    uint32_t airspacesCounted = 0;
    BoundingBox_LatLong_t myBoundingBox;
    boundingBoxFromPoint(&myBoundingBox, myLat, myLon, MAXDISTANCETOSEARCH);
    while (hasNext)
    {
        airspacesCounted++;
        long airspaceStartPos = ftellFileLayer(&instanceAirspace.layer);
        int errorValue;
        airspace *tempAirspace = readSingleAirspaceFromLayer(&errorValue, &hasNext, &myBoundingBox, cachedFile, airspaceStartPos);
        if (errorValue != 0)
            return 1;
        if (!tempAirspace)
            continue;
        airspacesCount++;
        if (prev)
        {
            prev->next = tempAirspace;
        }
        prev = tempAirspace;

        hasNext = readTilCharFileLayer(&instanceAirspace.layer, '{');
    }
    listNearest = instanceAirspace.nearestAirspaceList;
    prev = NULL;
    while (listNearest)
    {
        if (!listNearest->isValid)
        {
            // Delete from LinkedList
            // if (!prev) // First Item to delete
            //     instanceAirspace.nearestAirspaceList = listNearest->next;
            // else // Delete other item
            prev->next = listNearest->next;
            airspace *oldObj = listNearest;
            listNearest = listNearest->next;
            oldObj->next = NULL;
            if (oldObj->allocatedPolygons != NULL)
                free(oldObj->allocatedPolygons);
            if (oldObj->isAllocated)
                free(oldObj);
            continue;
        }
        prev = listNearest;
        listNearest = listNearest->next;
    }
    return 0;
}

int ParseFromCached(float myLat, float myLon, FILE *cachedFile, FILE *jsonFile)
{
    airspace *listNearest = instanceAirspace.nearestAirspaceList;
    while (listNearest)
    {
        listNearest->isValid = false;
        listNearest = listNearest->next;
    }
    fseek(cachedFile, 0, SEEK_END);
    long pos = ftell(cachedFile);
    fseek(cachedFile, 0, SEEK_SET);
    if (pos % (sizeof(BoundingBox_LatLong_t) + sizeof(long)) != 0)
    {
        printf("This file is not valid\n");
        return 1;
    }
    uint32_t allAirspacesLength = pos / (sizeof(BoundingBox_LatLong_t) + sizeof(long));
    BoundingBox_LatLong_t myBoundingBox;
    boundingBoxFromPoint(&myBoundingBox, myLat, myLon, MAXDISTANCETOSEARCH);
    airspace *prev = NULL;
    for (size_t i = 0; i < allAirspacesLength; i++)
    {
        BoundingBox_LatLong_t boundingBox;
        long airspacePos;
        fread(&boundingBox.maxLat, sizeof(float), 1, cachedFile);
        fread(&boundingBox.maxLong, sizeof(float), 1, cachedFile);
        fread(&boundingBox.minLat, sizeof(float), 1, cachedFile);
        fread(&boundingBox.minLong, sizeof(float), 1, cachedFile);
        fread(&airspacePos, sizeof(long), 1, cachedFile);
        printf("%.2f, %.2f, %.2f, %.2f, %.2f\n", boundingBox.maxLat, boundingBox.maxLong, boundingBox.minLat, boundingBox.minLong, (float)airspacePos);
        if (overlappBoundingBox(&myBoundingBox, &boundingBox))
            continue;
        startFileLayer(&instanceAirspace.layer, airspacePos, jsonFile);
        bool hasNext;
        int errorValue;
        airspace *tempAirspace = readSingleAirspaceFromLayer(&errorValue, &hasNext, &myBoundingBox, NULL, airspacePos);
        if (errorValue != 0)
            return 1;
        if (!tempAirspace)
        {
            printf("This sould definitly not happen!!\n");
            continue;
        }
        if (prev)
        {
            prev->next = tempAirspace;
        }
        prev = tempAirspace;
        printf("Found: %s\n", tempAirspace->name);
    }
    listNearest = instanceAirspace.nearestAirspaceList;
    prev = NULL;
    while (listNearest)
    {
        if (!listNearest->isValid && prev)
        {
            prev->next = listNearest->next;
            airspace *oldObj = listNearest;
            listNearest = listNearest->next;
            oldObj->next = NULL;
            if (oldObj->allocatedPolygons != NULL)
                free(oldObj->allocatedPolygons);
            if (oldObj->isAllocated)
                free(oldObj);
            continue;
        }
        prev = listNearest;
        listNearest = listNearest->next;
    }
    return 0;
}

void AirspaceTask()
{
    gps_t gpsData;
    vario_state_get_member(&gpsData, vario_state_gps_data, false);
    gpsData.valid = true;
    // while (!gpsData.valid)
    //{
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // vario_state_get_member(&gpsData, vario_state_gps_data, false);
    //}
    gpsData.latitude = 47.50733020688501f;
    gpsData.longitude = 7.525586773402665f;
    gpsData.date.year = 2023;
    gpsData.date.month = 05;
    gpsData.date.day = 26;
    char nameBuffer[50];
    snprintf(nameBuffer, 50, "airspace%i_%i_%i.txt", gpsData.date.year, gpsData.date.month, gpsData.date.day);
    // FILE *cachedFile = openFile_sdcard("/cache", nameBuffer, "r");
    FILE *cachedFile = NULL;
    FILE *jsonFile = openFile_sdcard("/airspace", "airspaces.json", "r");
    if (jsonFile == NULL)
    {
        printf("Cant open JSON File, please donwload it\n");
        return;
    }
    if (cachedFile == NULL)
    {
        cachedFile = openFile_sdcard("/cache", nameBuffer, "w");
        if (cachedFile == NULL)
        {
            printf("Cached File should always be able to open\n");
            fclose(jsonFile);
            vTaskDelete(NULL);
            return;
        }
        uint64_t timeStart = esp_timer_get_time();
        if (ParseAirspaceJSON(gpsData.latitude, gpsData.longitude, cachedFile, jsonFile) != 0)
        {
            printf("Cant parse JSON File");
            fclose(jsonFile);
            fclose(cachedFile);
            vTaskDelete(NULL);
            return;
        }
        printf("Took to parse: %i\n", (int)((esp_timer_get_time() - timeStart) / 1000));
        fclose(jsonFile);
        fclose(cachedFile);
        cachedFile = openFile_sdcard("/cache", nameBuffer, "r");
    }
    uint64_t startTime = esp_timer_get_time();
    ParseFromCached(gpsData.latitude, gpsData.longitude, cachedFile, jsonFile);
    printf("All took: %i\n", (int)((esp_timer_get_time() - startTime) / 1000));
    //  TODO: handle this case
    instanceAirspace.lastAllAirspaceUpdatePoint.lat = gpsData.latitude;
    instanceAirspace.lastAllAirspaceUpdatePoint.lon = gpsData.longitude;
    if (instanceAirspace.currentIndexAirspaces > 0)
    {
        // startTime = esp_timer_get_time();
        searchInNearestAirspace(gpsData.latitude, gpsData.longitude, gpsData.altitude);
        // printf("In Airspace took: %i\n", (int)((esp_timer_get_time() - startTime) / 1000));
        // startTime = esp_timer_get_time();
        nearestLineAirspace(gpsData.latitude, gpsData.longitude, gpsData.altitude);
        // printf("To all Lines took: %i\n", (int)(esp_timer_get_time() - startTime));
        // startTime = esp_timer_get_time();
        airspaceCalculateValuesToLine(gpsData.latitude, gpsData.longitude, gpsData.altitude);
        // printf("In Line took: %i\n", (int)(esp_timer_get_time() - startTime));
    }
    else
    {
        printf("Cant find airspace nearby\n");
        instanceAirspace.nearestAirspaceInfo.name[0] = '\0';
    }
    while (true)
    {
        vario_state_get_member(&gpsData, vario_state_gps_data, false);
        gpsData.latitude = 47.38407637662871,
        gpsData.longitude = 8.525115677931941f;
        float distanceToNearest = instanceAirspace.nearestAirspaceInfo.distanceToAirspaceVert;
        // Parse whole JSON, when distance to last parsed of 7km
        if (GET_DISTANCEFASTSQ(gpsData.latitude, gpsData.longitude, instanceAirspace.lastAllAirspaceUpdatePoint.lat, instanceAirspace.lastAllAirspaceUpdatePoint.lon) > (7 * 7))
        {
            ParseFromCached(gpsData.latitude, gpsData.longitude, cachedFile, jsonFile);
            instanceAirspace.lastAllAirspaceUpdatePoint.lat = gpsData.latitude;
            instanceAirspace.lastAllAirspaceUpdatePoint.lon = gpsData.longitude;
            searchInNearestAirspace(gpsData.latitude, gpsData.longitude, gpsData.altitude);
            nearestLineAirspace(gpsData.latitude, gpsData.longitude, gpsData.altitude);
            airspaceCalculateValuesToLine(gpsData.latitude, gpsData.longitude, gpsData.altitude);
        }
        // Search in all nearest Airspace for nearestAirspace, when (diff1-2)/2 is bigger than distance to last calc
        else if (GET_DISTANCEFASTSQ(gpsData.latitude, gpsData.longitude, instanceAirspace.lastInNearAirspacesUpdatePoint.lat, instanceAirspace.lastInNearAirspacesUpdatePoint.lon))
        {
            searchInNearestAirspace(gpsData.latitude, gpsData.longitude, gpsData.altitude);
            nearestLineAirspace(gpsData.latitude, gpsData.longitude, gpsData.altitude);
            airspaceCalculateValuesToLine(gpsData.latitude, gpsData.longitude, gpsData.altitude);
        }
        // Search for Line in nearest Airspace when distance to nearestAirspace is < 1km -> all 100m, when farer away all 500m
        else if ((distanceToNearest < 1000 && GET_DISTANCEFASTSQ(gpsData.latitude, gpsData.longitude, instanceAirspace.lastInNearestAirspaceUpdatePoint.lat, instanceAirspace.lastInNearestAirspaceUpdatePoint.lon) > 100) ||
                 GET_DISTANCEFASTSQ(gpsData.latitude, gpsData.longitude, instanceAirspace.lastInNearestAirspaceUpdatePoint.lat, instanceAirspace.lastInNearestAirspaceUpdatePoint.lon) > 500)
        {
            nearestLineAirspace(gpsData.latitude, gpsData.longitude, gpsData.altitude);
            airspaceCalculateValuesToLine(gpsData.latitude, gpsData.longitude, gpsData.altitude);
        }
        else
        {
            airspaceCalculateValuesToLine(gpsData.latitude, gpsData.longitude, gpsData.altitude);
            // vTaskDelay(4000 / portTICK_PERIOD_MS);
        }
    }
}

void StartAirspaceParse()
{
    // AirspaceTask();
    xTaskCreate(AirspaceTask, "Airspace Parcer", 1024 * 3, NULL, 0, NULL);
}