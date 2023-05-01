#pragma once

typedef struct
{
	float height;
	float velocity;
	float accX;
	float rotation[4];
}vario_state_file_t;

typedef struct
{
	float height;
	float velocity;
	float accX;
	float rotation[4];
	long long time;
} vario_state_Simulation;