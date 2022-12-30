#include "E_Ink/EPD_3in7.h"
#include "E_Ink/GUI_Paint.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include <inttypes.h>
#include <string.h>

void testFunction_EPD3IN7()
{
    DEV_Module_Init();
    //Create a new image cache
    UBYTE *ScreenBuffer;
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    UWORD Imagesize = ((EPD_3IN7_WIDTH % 4 == 0)? (EPD_3IN7_WIDTH / 4 ): (EPD_3IN7_WIDTH / 4 + 1)) * EPD_3IN7_HEIGHT;
    if((ScreenBuffer = (UBYTE *)heap_caps_malloc(Imagesize, MALLOC_CAP_DMA)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }

    printf("Paint_NewImage \r\n");
    Paint_NewImage(ScreenBuffer, EPD_3IN7_WIDTH, EPD_3IN7_HEIGHT, 90, WHITE);
    EPD_3IN7_1Gray_Init();       //init 1 Gray mode
    EPD_3IN7_1Gray_Clear();
    
    printf("Simulation Start......\n");
    Paint_SelectImage(ScreenBuffer);
    Paint_SetScale(2);

    Paint_Clear(WHITE);
    uint64_t startTime = esp_timer_get_time();
    Paint_DrawRectangle(2, 2, 100, 100, BLACK, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    EPD_3IN7_1Gray_Display(ScreenBuffer, PART_BW_T_BW);

    // printf("Time: %" PRId64 "\n",(esp_timer_get_time()-startTime) / 1000);
    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // Paint_Clear(WHITE);
    // EPD_3IN7_1Gray_Display(ScreenBuffer, true);   

    // Paint_DrawRectangle(1, 1, 100, 100, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    // startTime = esp_timer_get_time();
    // EPD_3IN7_1Gray_Display(ScreenBuffer, FULL_G_T_BW);  
    // printf("Time: %" PRId64 "\n",(esp_timer_get_time()-startTime) / 1000);
    // Paint_Clear(WHITE);

    vTaskDelay(10000 / portTICK_PERIOD_MS);
    EPD_3IN7_4Gray_Init();
    EPD_3IN7_4Gray_Clear();
    
    // Sleep & close 5V
    printf("Goto Sleep...\r\n");
    EPD_3IN7_Sleep();

    free(ScreenBuffer);
    ScreenBuffer = NULL;

    printf("close 5V, Module enters 0 power consumption ...\r\n");
    DEV_Module_Exit();
}