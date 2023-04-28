#include <stdio.h>

#define portMAX_DELAY 10

typedef struct {
	int tempData;
} Semaphore;

typedef Semaphore *SemaphoreHandle_t;

SemaphoreHandle_t xSemaphoreCreateMutex();
void xSemaphoreTake(SemaphoreHandle_t handle, int portDelay);
void xSemaphoreGive(SemaphoreHandle_t handle);