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
    char *vario = getenv("VARIO_REPLAY_VARIO");
    char *gps = getenv("VARIO_REPLAY_GPS");
    simulation_start(vario ? vario : "VarioLogs/vario2.txt",
                     gps ? gps : "VarioLogs/gps2.txt");
    xTaskCreate(app_main, "Main App", 1024 * 3, NULL, 2, NULL);
    portENABLE_INTERRUPTS();
    vTaskStartScheduler();
    for (;;)
    {
    }
    return 0;
}
