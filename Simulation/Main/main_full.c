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

extern void app_main(void);

int main_full(void)
{
    simulation_start("vario2.txt", "gps2.txt");
    xTaskCreate(app_main, "Main App", 1024, NULL, 2, NULL);
    portENABLE_INTERRUPTS();
    vTaskStartScheduler();
    for (;;)
    {
    }
    return 0;
}