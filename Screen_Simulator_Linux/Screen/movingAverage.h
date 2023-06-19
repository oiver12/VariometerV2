#ifndef __MOVINGAVERAGE_H
#define __MOVINGAVERAGE_H

#include "Screen/FIFO.h"

typedef struct movingAverage_t
{
    fifo_t fifo;
    float sum;
    size_t count;
} movingAverage_t;

void movingAverage_init(movingAverage_t *avg, uint16_t num);
void movingAverage_nextSTep(float value, movingAverage_t *avg);
float movingAverage_getAvg(movingAverage_t *avg);
void movingAverage_deinit(movingAverage_t *avg);

#endif