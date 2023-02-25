#ifndef __UTIL_H__
#define __UTIL_H__

#include "stdint.h"

#define PI          3.1415926535897932384626433832795

inline double TO_RADIAN(double deg)
{
    return deg / 180.0 * PI;
}

inline double TO_DEGREE(double rad)
{
    return rad * (180.0 / PI);
}


int16_t     GET_BEARING(double lat1, double lon1, double lat2, double lon2);
double      GET_DISTANCE(double lat1, double lon1, double lat2, double lon2);


#endif
