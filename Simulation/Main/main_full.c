#include "SDLHelper/SDL_helper.h"
#include "SimulationData.h"

/* Standard includes. */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

#define EPD_3IN7_WIDTH 280
#define EPD_3IN7_HEIGHT 480

extern void app_main(void);

int main_full(void)
{
    SDLH_initializeScreen(EPD_3IN7_HEIGHT, EPD_3IN7_WIDTH);
    simulation_start("screenlog.0", SIMULATION_SERIAL);
    xTaskCreate(app_main, "Main App", 1024, NULL, 2, NULL);
    portENABLE_INTERRUPTS();
    vTaskStartScheduler();
    for (;;)
    {
    }
    return 0;
}