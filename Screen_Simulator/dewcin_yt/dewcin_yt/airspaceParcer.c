#include "cJSON.h"
#include <stdint.h>
#include <stdio.h>
#include <Windows.h>
#include "cJSON.h"
#include "Util.h"
#include <math.h>
#include <stdbool.h>

#define MAXDISTANCETOSEARCH 10000

typedef enum
{
	HeightType_mQNH = 1,
	HeightType_mSTD = 2,
	HeightType_mAGL = 3
} heightType;

typedef struct
{
	char* name;
	uint16_t upperLimit;
	heightType heightTypeUpper;
	uint16_t lowerLimit;
	heightType heightTypeLower;
	polygonSide* polygons;
	size_t polygonSize;
	void* next;
} airspace;

void airspaceCalculateValues(airspace* nearestAirspace, int indexNearestPolygon, float myLat, float myLon)
{
	int secondNearestPolyIndex = 0;
	if (indexNearestPolygon == 0)
		secondNearestPolyIndex = 1;
	else if (indexNearestPolygon == nearestAirspace->polygonSize)
		secondNearestPolyIndex = nearestAirspace->polygonSize - 1;
	else
	{
		float distanceToNext = GET_DISTANCE(myLat, myLon, nearestAirspace->polygons[indexNearestPolygon + 1].latitude, nearestAirspace->polygons[indexNearestPolygon + 1].longitude);
		float distanceToPrev = GET_DISTANCE(myLat, myLon, nearestAirspace->polygons[indexNearestPolygon - 1].latitude, nearestAirspace->polygons[indexNearestPolygon - 1].longitude);
		if (distanceToNext < distanceToPrev)
			secondNearestPolyIndex = indexNearestPolygon + 1;
		else
			secondNearestPolyIndex = indexNearestPolygon - 1;
	}
	vectorEarth nearestPointVector;
	vectorEarth secondNearestPointVector;
	vectorEarth myPositionVector;
	getVectorRep(myLat, myLon, &myPositionVector);
	getVectorRep(nearestAirspace->polygons[indexNearestPolygon].latitude, nearestAirspace->polygons[indexNearestPolygon].longitude, &nearestPointVector);
	getVectorRep(nearestAirspace->polygons[secondNearestPolyIndex].latitude, nearestAirspace->polygons[secondNearestPolyIndex].longitude, &secondNearestPointVector);
	float nearestLat, nearestLon;
	nearestPointOnSegment(&nearestPointVector, &secondNearestPointVector, &myPositionVector, &nearestLon, &nearestLat);
	float distance = GET_DISTANCE(myLat, myLon, nearestLat, nearestLon);
	float distanceFirst = GET_DISTANCE(myLat, myLon, nearestAirspace->polygons[indexNearestPolygon].latitude, nearestAirspace->polygons[indexNearestPolygon].longitude);
	int16_t bearing = GET_BEARING(myLat, myLon, nearestLat, nearestLon);
}

void searchNearestAirspace(double myLat, double myLon, float altitude, airspace* airspaces)
{
	airspace* nearestAirspace = NULL;
	int nearestAirspaceNearestPoly = 0;
	float nearestDistanceToAirspace = MAXDISTANCETOSEARCH;
	while (airspaces != NULL)
	{
		//TODO: https://rosettacode.org/wiki/Ray-casting_algorithm
		int nearestPolyIndex = 0;
		float nearestDistance = INFINITY;
		
		for (size_t i = 0; i < airspaces->polygonSize; i++)
		{
			float distance = GET_DISTANCE(myLat, myLon, airspaces->polygons[i].latitude, airspaces->polygons[i].longitude);
			if (distance < nearestDistance)
			{
				nearestDistance = distance;
				nearestPolyIndex = i;
			}
		}
		
		if (nearestDistance < nearestDistanceToAirspace)
		{
			if (altitude < airspaces->lowerLimit)
			{
				nearestDistance += (airspaces->lowerLimit - altitude);
			}
			else if (altitude > airspaces->upperLimit)
			{
				nearestDistance += (altitude - airspaces->upperLimit);
			}
			
			if (nearestDistance < nearestDistanceToAirspace)
			{
				nearestDistanceToAirspace = nearestDistance;
				nearestAirspace = airspaces;
				nearestAirspaceNearestPoly = nearestPolyIndex;
			}
		}
		airspaceCalculateValues(nearestAirspace, nearestAirspaceNearestPoly, myLat, myLon);
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

bool readTilCharacter(FILE* file, char character)
{
	char c;
	while ((c = fgetc(file)) != character && (int)c != EOF);
	{
	}
	return (int)c != EOF;
}

bool readTilCharacterTimes(FILE* file, char character, uint8_t times)
{
	char c;
	for (size_t i = 0; i < times; i++)
	{
		while ((c = fgetc(file)) != character && (int)c != EOF);
		{
		}
		if ((int)c == EOF)
			return false;
	}
	return true;
}

void readToBufferTillNextChar(FILE* file, char character, char* buffer, int bufferSize)
{
	char c;
	int i = 0;
	while ((c = fgetc(file)) != character)
	{
		if (i < bufferSize-1)
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

//TODO: Optimize this function
airspace* readPolygonsFirst(FILE* fileptr, bool* hasNext, float myLat, float myLon)
{
	double minDistance = 1E+10;
	char nextChar = ',';
	int polygonSize = 0;
	long posPolyStart = ftell(fileptr);
	while (nextChar == ',')
	{
		polygonSize++;
		readTilCharacter(fileptr, '[');
		float lonitude;
		fscanf(fileptr, "%f", &lonitude);
		fgetc(fileptr);
		float latitude;
		fscanf(fileptr, "%f", &latitude);
		double distance = GET_DISTANCEFAST(myLat, myLon, latitude, lonitude);
		minDistance = fmin(minDistance, distance);
		fgetc(fileptr);
		nextChar = fgetc(fileptr);
	}
	if (minDistance > MAXDISTANCETOSEARCH)
	{
		*hasNext = readTilCharacter(fileptr, '{');
		return NULL;
	}
	//airspacesCount++;
	airspace* tempAirspace = calloc(sizeof(airspace), 1);
	tempAirspace->polygonSize = polygonSize;

	fseek(fileptr, posPolyStart, SEEK_SET);

	tempAirspace->polygons = calloc(sizeof(polygonSide), polygonSize);
	int i = 0;
	nextChar = ',';
	while (nextChar == ',')
	{
		polygonSide* tempPolygon = &tempAirspace->polygons[i];
		i++;
		readTilCharacter(fileptr, '[');
		fscanf(fileptr, "%f", &tempPolygon->longitude);
		char c = fgetc(fileptr);
		fscanf(fileptr, "%f", &tempPolygon->latitude);

		fgetc(fileptr);
		nextChar = fgetc(fileptr);
	}
	*hasNext = true;
	return tempAirspace;
}

void airspaceParserNew()
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	double interval;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);

	double myLat = 47.5099482;
	double myLon = 7.5206331;

	FILE* fileptr;
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

	airspace* headerAirspace = NULL;
	airspace* prev = NULL;
	uint32_t airspacesCounted = 0;
	while (hasNext)
	{
		airspacesCounted++;
		readTilCharacter(fileptr, '{');
		readTilCharacter(fileptr, ':');
		fgetc(fileptr);
		readToBufferTillNextChar(fileptr, '"', nameBuffer, 50);
		if(strcmp(nameBuffer, "TMA Bale LANGEN SW1") == 0)
			printf("Found");
		char test30 = fgetc(fileptr);
		readTilCharacterTimes(fileptr, ',', 5);
		char test40 = fgetc(fileptr);
		char temp = fgetc(fileptr);
		//Check if Freqeuncy Field exists
		if (temp == 'F')
			readTilCharacter(fileptr, ',');
		fgetc(fileptr);
		char temp1 = fgetc(fileptr);
		//check if Callsign Field exists
		if (temp1 == 'C' || temp == 'C')
			readTilCharacter(fileptr, ',');
		readTilCharacter(fileptr, ',');
		fgetc(fileptr);
		temp = fgetc(fileptr);
		//Check if Validity Field exists
		//TODO: save the validity
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
		//TODO: Save Alt2 
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
		
		airspace* tempAirspace = readPolygonsFirst(fileptr, &hasNext, myLat, myLon);

		if (!tempAirspace)
			continue;
		
		airspacesCounted++;
		
		if (!headerAirspace)
			headerAirspace = tempAirspace;
		tempAirspace->lowerLimit = lowerLimit;
		tempAirspace->upperLimit = uppperLimit;
		tempAirspace->heightTypeLower = heightTypeLower;
		tempAirspace->heightTypeUpper = heightTypeUpper;
		tempAirspace->name = calloc(strlen(nameBuffer) + 1, 1);
		strcpy(tempAirspace->name, nameBuffer);

		bool isEnclosed = isEnclosedByPolygon(tempAirspace->polygons, tempAirspace->polygonSize, myLat, myLon);
		
		if (prev)
		{
			prev->next = tempAirspace;
		}
		prev = tempAirspace;

		hasNext = readTilCharacter(fileptr, '{');
	}
	fclose(fileptr);
	QueryPerformanceCounter(&end);
	interval = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
	searchNearestAirspace(myLat, myLon, 341, headerAirspace);
}

void airspaceParcer()
{
	airspaceParserNew();

	LARGE_INTEGER frequency;
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	double interval;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	
	double myLat = 47.5099482;
	double myLon = 7.5206331;

	FILE* fileptr;
	fileptr = fopen("airspace/airspaces.json", "rb");
	fseek(fileptr, 0, SEEK_END);
	uint32_t bufferSize = ftell(fileptr);
	rewind(fileptr);                      // Jump back to the beginning of the file
	uint8_t* bufferRead = (uint8_t*)malloc(bufferSize * sizeof(uint8_t)); // Enough memory for the file
	fread(bufferRead, bufferSize, 1, fileptr); // Read in the entire file
	fclose(fileptr); // Close the file
	cJSON* root = cJSON_Parse((char*)bufferRead);
	if (root == NULL)
	{
		const char* error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}
	cJSON* features = cJSON_GetObjectItemCaseSensitive(root, "features");
	cJSON* feature;
	airspace* headerAirspace = NULL;
	airspace* prev = NULL;
	uint64_t sizeTotal = 0;
	uint32_t size = 0;
	uint32_t fsdsd = 0;
	cJSON_ArrayForEach(feature, features)
	{
		fsdsd++;
		cJSON* geometry = cJSON_GetObjectItemCaseSensitive(feature, "geometry");
		cJSON* coordinates = cJSON_GetObjectItemCaseSensitive(geometry, "coordinates");
		double minDistance = 1E+10;
		cJSON* coordinate;
		int polygonSize = 0;
		cJSON_ArrayForEach(coordinate, coordinates->child)
		{
			polygonSize++;
			double lon = coordinate->child->valuedouble;
			double lat = coordinate->child->next->valuedouble;
			double distance = GET_DISTANCEFAST(myLat, myLon, lat, lon);
			minDistance = fmin(minDistance, distance);
		}
		if(minDistance > MAXDISTANCETOSEARCH)
		{
			continue;
		}
		size++;
		airspace* tempAirspace = calloc(sizeof(airspace), 1);
		if (!headerAirspace)
			headerAirspace = tempAirspace;
		sizeTotal += sizeof(airspace);
		tempAirspace->polygons = NULL;
		if (!tempAirspace)
		{
			printf("Allocation failed\n");
			goto end;
		}
		cJSON* properties = cJSON_GetObjectItemCaseSensitive(feature, "properties");
		cJSON* name = cJSON_GetObjectItemCaseSensitive(properties, "Name");
		tempAirspace->name = malloc(strlen(name->valuestring) + 1);
		sizeTotal += strlen(name->valuestring) + 1;
		strcpy(tempAirspace->name, name->valuestring);
		cJSON* Upper = cJSON_GetObjectItemCaseSensitive(properties, "Upper");
		cJSON* metric = cJSON_GetObjectItemCaseSensitive(Upper, "Metric");
		cJSON* Alt = cJSON_GetObjectItemCaseSensitive(metric, "Alt");
		cJSON* Altitude = cJSON_GetObjectItemCaseSensitive(Alt, "Altitude");
		tempAirspace->upperLimit = Altitude->valueint;
		cJSON* Lower = cJSON_GetObjectItemCaseSensitive(properties, "Lower");
		metric = cJSON_GetObjectItemCaseSensitive(Lower, "Metric");
		Alt = cJSON_GetObjectItemCaseSensitive(metric, "Alt");
		Altitude = cJSON_GetObjectItemCaseSensitive(Alt, "Altitude");
		tempAirspace->lowerLimit = Altitude->valueint;
		tempAirspace->polygonSize = polygonSize;

		tempAirspace->polygons = calloc(sizeof(polygonSide), polygonSize);
		sizeTotal += sizeof(polygonSide) * polygonSize;
		int i = 0;
		cJSON_ArrayForEach(coordinate, coordinates->child)
		{
			if (i >= polygonSize)
			{
				printf("Error\n");
				goto end;
			}
			polygonSide* tempPolygon = &tempAirspace->polygons[i];
			i++;
			tempPolygon->longitude = coordinate->child->valuedouble;
			tempPolygon->latitude = coordinate->child->next->valuedouble;
		}
		if (prev)
		{
			prev->next = tempAirspace;
		}
		prev = tempAirspace;
	}
end:
	cJSON_Delete(root);
	free(bufferRead);
	QueryPerformanceCounter(&end);
	interval = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
	searchNearestAirspace(myLat, myLon, 341, headerAirspace);
}