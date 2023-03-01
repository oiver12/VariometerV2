#include "Util.h"
#include "math.h"
#include <string.h>
#include <stdio.h>

//https://www.movable-type.co.uk/scripts/latlong.html
//https://www.movable-type.co.uk/scripts/latlong-vectors.html#intersection

static void cross(vectorEarth *v1, vectorEarth *v2, vectorEarth *v3)
{
	v3->x = v1->y * v2->z - v1->z * v2->y;
	v3->y = v1->z * v2->x - v1->x * v2->z;
	v3->z = v1->x * v2->y - v1->y * v2->x;
}

static float dot(vectorEarth* v1, vectorEarth* v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

static float angleTo(vectorEarth* from, vectorEarth* to, vectorEarth* n)
{
	vectorEarth temp;
	cross(from, to, &temp);
	const int sign = (dot(&temp, n) < 0) ? -1 : 1;
	const float sinV = sqrt(temp.x * temp.x + temp.y * temp.y + temp.z * temp.z) * sign;
	const float cosV = dot(from, to);
	return atan2(sinV, cosV);
}

void getLatLon(vectorEarth* v, float* lat, float* lon)
{
	*lat = TO_DEGREE(atan2(v->z, sqrt(v->x * v->x + v->y * v->y)));
	*lon = TO_DEGREE(atan2(v->y, v->x));
}

void getVectorRep(double lat1, double lon1, vectorEarth* v)
{
	const double sinLat = sin(TO_RADIAN(lat1));
	const double cosLat = cos(TO_RADIAN(lat1));
	const double sinLon = sin(TO_RADIAN(lon1));
	const double cosLon = cos(TO_RADIAN(lon1));
	const double x = cosLat * cosLon;
	const double y = cosLat * sinLon;
	const double z = sinLat;
	v->x = x;
	v->y = y;
	v->z = z;
}

double GET_DISTANCEVECTOR(vectorEarth* v1, vectorEarth* v2)
{
	vectorEarth v3;
	cross(v1, v2, &v3);
	float sinVector = sqrt(v3.x * v3.x + v3.y * v3.y + v3.z * v3.z);
	float cosVector = dot(v1, v2);
	return atan2(sinVector, cosVector) * 6371e3;
}

static bool vectorEquals(vectorEarth* v1, vectorEarth* v2)
{
	return v1->x == v2->x && v1->y == v2->y && v1->z == v2->z;
}

bool pointIsWithtinExtent(vectorEarth* startPoint, vectorEarth* endPoint, vectorEarth* point)
{
    if (vectorEquals(startPoint, endPoint)) return vectorEquals(point, startPoint);
	const vectorEarth f10 = { point->x - startPoint->x, point->y - startPoint->y, point->z - startPoint->z };
	const vectorEarth f12 = { endPoint->x - startPoint->x, endPoint->y - startPoint->y, endPoint->z - startPoint->z };
	const vectorEarth f20 = { point->x - endPoint->x, point->y - endPoint->y, point->z - endPoint->z };
	const vectorEarth f21 = { startPoint->x - endPoint->x, startPoint->y - endPoint->y, startPoint->z - endPoint->z };
	const float extent1 = dot(&f10, &f12);
	const float extent2 = dot(&f20, &f21);
	const bool isSameHemisphere = dot(point, startPoint) >= 0 && dot(point, endPoint) >= 0;
	return extent1 >= 0 && extent2 >= 0 && isSameHemisphere;
}

void nearestPointOnSegment(vectorEarth* startPoint, vectorEarth* endPoint, vectorEarth* point, float* nearetsLon, float* nearestLat)
{
	if (pointIsWithtinExtent(startPoint, endPoint, point) && !vectorEquals(startPoint, endPoint))
	{
		const vectorEarth c1;
		cross(startPoint, endPoint, &c1);
		const vectorEarth c2;
		cross(point, &c1, &c2);
		const vectorEarth n;
		cross(&c1, &c2, &n);
		getLatLon(&n, nearestLat, nearetsLon);
	}
	else
	{
		const double d1 = GET_DISTANCEVECTOR(point, startPoint);
		const double d2 = GET_DISTANCEVECTOR(point, endPoint);
		if (d1 < d2)
		{
			getLatLon(startPoint, nearestLat, nearetsLon);
		}
		else
		{
			getLatLon(endPoint, nearestLat, nearetsLon);
		}
	}
}

bool isEnclosedByPolygon(polygonSide* polyArray, int numPolygons, float lat, float lon)
{
	if (numPolygons < 3) return false;
	vectorEarth point;
	getVectorRep(lat, lon, &point);
	float sumAngle = 0;
	vectorEarth polySideVec;
	getVectorRep(polyArray[0].latitude, polyArray[0].longitude, &polySideVec);
	vectorEarth lastPoint = { .x = point.x - polySideVec.x, .y = point.y - polySideVec.y, .z = point.z - polySideVec.z };
	char buffer[5000];
	snprintf(buffer, 5000, "const testArray = [");
	char tempBuffer[100];
	for (size_t i = 1; i < numPolygons + 1; i++)
	{
		snprintf(tempBuffer, 100, "{lat: %f, lon: %f}, ", polyArray[i - 1].latitude, polyArray[i - 1].longitude);
		strcat(buffer, tempBuffer);
		if (i == numPolygons)
		{
			getVectorRep(polyArray[0].latitude, polyArray[0].longitude, &polySideVec);
		}
		else
		{
			getVectorRep(polyArray[i].latitude, polyArray[i].longitude, &polySideVec);
		}
		vectorEarth vecTemp = { .x = point.x - polySideVec.x, .y = point.y - polySideVec.y, .z = point.z - polySideVec.z};
		sumAngle += angleTo(&lastPoint, &vecTemp, &point);
		lastPoint = vecTemp;
	}
	strcat(buffer, "];");
	float test2 = fabs(sumAngle);
	bool test = fabs(sumAngle) > PI;
	return fabs(sumAngle) > PI;
}

int16_t GET_BEARING(double lat1, double lon1, double lat2, double lon2)
{
    double teta1 = TO_RADIAN(lat1);
    double teta2 = TO_RADIAN(lat2);

    double delta2 = TO_RADIAN(lon2 - lon1);

    double y = sin(delta2) * cos(teta2);
    double x = cos(teta1) * sin(teta2) - sin(teta1) * cos(teta2) * cos(delta2);

    double bearing = atan2(y, x);
    bearing = TO_DEGREE(bearing);// radians to degrees

    return (((int16_t)bearing + 360) % 360);

#if 0
    float dx = (lon2 - lon1);
    float dy = (lat2 - lat1);

    return (-(int16_t)TO_DEGREE(atan2(dx, dy)) + 360) % 360;
#endif
}

double GET_DISTANCE(double lat1, double lon1, double lat2, double lon2)
{
    double theta, dist;

    if ((lat1 == lat2) && (lon1 == lon2))
        return 0;

    theta = lon1 - lon2;
    dist = sin(TO_RADIAN(lat1)) * sin(TO_RADIAN(lat2)) + cos(TO_RADIAN(lat1)) * cos(TO_RADIAN(lat2)) * cos(TO_RADIAN(theta));
    dist = acos(dist);
    dist = TO_DEGREE(dist);
    dist = dist * 60 * 1.1515;

    return dist * 1609.344; // meters
}

double GET_DISTANCEFAST(double lat1, double lon1, double lat2, double lon2)
{
	double x = TO_RADIAN(lon1 - lon2) * cos(TO_RADIAN(lat1 + lat2) / 2);
	double y = TO_RADIAN(lat1 - lat2);
	return sqrt(x * x + y * y) * 6371e3; //meters
}
