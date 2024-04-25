#include "RenderSystem.h"
#include "EPD_3in7.h"
#include "SDLHelper/SDL_helper.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#define EPD_3IN7_WIDTH 280
#define EPD_3IN7_HEIGHT 480

void simulationKeyHandlerTask(void *arg)
{
    while (1)
    {
        SDLH_HandleKey();
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void simulationDisplayTask(void *arg)
{
    SDLH_initializeScreen(EPD_3IN7_HEIGHT, EPD_3IN7_WIDTH);
    ScreenRender_arg *arguments = (ScreenRender_arg *)arg;
    uint8_t *ScreenBuffer = (uint8_t *)arguments->ScreenBuffer;
    QueueHandle_t renderQueue = (QueueHandle_t)arguments->renderQueue;
    SemaphoreHandle_t buffer_Mutex = (SemaphoreHandle_t)arguments->buffer_Mutex;
    Display_RefreshMode refreshMode;
    while (1)
    {
        xQueueReceive(renderQueue, &refreshMode, portMAX_DELAY);
        xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
        SDLH_update(ScreenBuffer);
        xSemaphoreGive(buffer_Mutex);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}