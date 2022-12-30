#include <stdio.h>

typedef struct {
	int tempData;
} Semaphore;

typedef Semaphore *SemaphoreHandle_t;

SemaphoreHandle_t xSemaphoreCreateMutex();
void xSemaphoreTake(SemaphoreHandle_t handle, int portDelay);
void xSemaphoreGive(SemaphoreHandle_t handle);