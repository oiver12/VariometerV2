#ifndef __UTIL_H__
#define __UTIL_H__

#include "stdint.h"

#define PI          3.1415926535897932384626433832795


int16_t     GET_BEARING(double lat1, double lon1, double lat2, double lon2);
double      GET_DISTANCE(double lat1, double lon1, double lat2, double lon2);


#endif
