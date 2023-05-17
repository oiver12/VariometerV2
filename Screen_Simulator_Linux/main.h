#pragma once

#include "Screen/nmea_parser.h"

typedef struct
{
	float height;
	float velocity;
	float accX;
	float rotation[4];
} vario_state_file_t;

typedef struct
{
	float height;
	float velocity;
	float accX;
	float rotation[4];
	long long time;
} vario_state_Simulation;

typedef struct
{
	gps_time_t timeToValid;
	gps_time_t timeToGPS;
	gps_time_t timeToDGPS;
	gps_time_t timeTo2D;
	gps_time_t timeTo3D;
	double meanDopH;
	double meanDopP;
	double meanDopV;
	float meanSatsInViewValid;
	float meanSatsInUseValid;
} gps_analyze_t;