#include "SemaphorTemp.h"

SemaphoreHandle_t xSemaphoreCreateMutex()
{
	return malloc(sizeof(Semaphore));
}
void xSemaphoreTake(SemaphoreHandle_t handle, int portDelay)
{
	return;
}
void xSemaphoreGive(SemaphoreHandle_t handle)
{
	return;
}