#pragma once

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
	gps_date_t timeToValid;
	gps_date_t timeToGPS;
	gps_date_t timeToDGPS;
	gps_date_t timeTo2D;
	gps_date_t timeTo3D;
	double meanDopH;
	double meanDopP;
	double meanDopV;
	float meanSatsInViewValid;
	float meanSatsInUseValid;
} gps_analyze_t;