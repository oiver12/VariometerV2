#include "movingAverage.h"

void movingAverage_init(movingAverage_t *avg, uint16_t num)
{
    avg->fifo = fifo_create(num, sizeof(float));
    avg->count = 0;
    avg->sum = 0;
}

void movingAverage_nextSTep(float value, movingAverage_t *avg)
{
    if (fifo_is_full(avg->fifo))
    {
        float temp;
        fifo_get(avg->fifo, &temp);
        avg->sum -= temp;
        avg->count++;
    }
    avg->sum += value;
    fifo_add(avg->fifo, &value);
}

float movingAverage_getAvg(movingAverage_t *avg)
{
    return avg->sum / (float)avg->count;
}

void movingAverage_deinit(movingAverage_t *avg)
{
    fifo_deinit(avg->fifo);
}