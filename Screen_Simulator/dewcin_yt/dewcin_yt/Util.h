#ifndef __UTIL_H__
#define __UTIL_H__

#include "stdint.h"
#include <stdbool.h>

#define PI          3.1415926535897932384626433832795

typedef struct 
{
    float x;
    float y;
    float z;
} vectorEarth;

typedef struct
{
    float minLat;
    float maxLat;
    float minLong;
    float maxLong;
} BoundingBox_LatLong_t;

typedef struct
{
    float latitude;
    float longitude;
} polygonSide;

inline double TO_RADIAN(double deg)
{
    return deg / 180.0 * PI;
}

inline double TO_DEGREE(double rad)
{
    return rad * (180.0 / PI);
}

double GET_DISTANCEVECTOR(vectorEarth* v1, vectorEarth* v2);
int16_t     GET_BEARING(double lat1, double lon1, double lat2, double lon2);
double      GET_DISTANCE(double lat1, double lon1, double lat2, double lon2);
double      GET_DISTANCEFAST(double lat1, double lon1, double lat2, double lon2);
void getVectorRep(double lat1, double lon1, vectorEarth* v);
void getLatLon(vectorEarth* v, float* lat, float* lon);
bool pointIsWithtinExtent(vectorEarth* startPoint, vectorEarth* endPoint, vectorEarth* point);
void nearestPointOnSegment(vectorEarth* startPoint, vectorEarth* endPoint, vectorEarth* point, float* nearetsLon, float* nearestLat);
float distandeToNearestPointOnSegment(vectorEarth* startPoint, vectorEarth* endPoint, vectorEarth* point);
bool isEnclosedByPolygon(polygonSide* polyArray, int numPolygons, float lat, float lon);

float minimumDistancePointToLineSqr(float startLat, float startLon, float endLat, float endLon, float pointLat, float pointLon);

bool overlappBoundingBox(BoundingBox_LatLong_t* box1, BoundingBox_LatLong_t* box2);
void boundingBoxFromPoint(BoundingBox_LatLong_t* box, float lat, float lon, float distanceMeters);
//float initzialBearing(vectorEarth* v1, vectorEarth* v2);

#endif
