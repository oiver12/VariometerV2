#ifndef QUATERNIONFILTER_H
#define QUATERNIONFILTER_H

void QuaternionFilter_init();
void QuaternionFilter_Madgwick(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float deltaT, float* q);
void QuaternionFilter_Mahony(float ax, float ay, float az, float gx, float gy, float gz, float deltaT, float* q);
#endif