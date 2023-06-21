#ifndef __SIMULATION_
#define __SIMULATION_

#include <stdio.h>
#include <stdbool.h>

typedef struct
{
    float presHeight;
    float gpsHeight;
    float velocity;
    float accX;
    float rotation[4];
    long long time;
    float lat;
    float lon;
} vario_state_Simulation;

void simulation_init(bool igc, char *fileName);
// in miliseconds
void simulation_nextStep();
void simulation_fastForward(float percentage);

#endif