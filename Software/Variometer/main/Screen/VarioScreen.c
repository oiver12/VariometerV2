#include "VarioScreen.h"
#include <assert.h>
#include <esp_heap_caps.h>
#include <esp_log.h>

UBYTE* init_Screen(vario_state_t* state, SemaphoreHandle_t Mutex)
{
    DEV_Module_Init();
    //Create a new image cache
    UBYTE *ScreenBuffer;
    volatile UWORD ImageSize = ((EPD_3IN7_WIDTH % 4 == 0)? (EPD_3IN7_WIDTH / 4 ): (EPD_3IN7_WIDTH / 4 + 1)) * EPD_3IN7_HEIGHT;
    if((ScreenBuffer = (UBYTE *)heap_caps_malloc(ImageSize, MALLOC_CAP_DMA)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return NULL;
    }
    printf("Paint_NewImage \r\n");
    Paint_NewImage(ScreenBuffer, EPD_3IN7_WIDTH, EPD_3IN7_HEIGHT, 270, WHITE);
    EPD_3IN7_1Gray_Init();       //init 1 Gray mode
    EPD_3IN7_1Gray_Clear();
    init_Widgets_Handler(state, Mutex);
    return ScreenBuffer;
}

void deinit_Screen(VarioScreen_t* screen)
{
    EPD_3IN7_4Gray_Init();
    EPD_3IN7_4Gray_Clear();
    EPD_3IN7_Sleep();
    free(screen->Screen_Buffer);
    screen->Screen_Buffer = NULL;
    free(screen->widgets);
    screen->widgets = NULL;
    printf("Close Screen ...\r\n");
    DEV_Module_Exit();
}

void CreateScreen(VarioScreen_t * screen, UBYTE* buffer)
{
    screen->widgets = malloc(max_Widgets*sizeof(WidgetData));
    screen->widgetsAdded = 0;
    screen->Screen_Buffer = buffer;
}

void selectScreen(VarioScreen_t* screen)
{
    Paint_SelectImage(screen->Screen_Buffer);
    Paint_SetScale(2);
    Paint_Clear(WHITE);
}

void VarioScreen_add_Widget(WidgetData* widget, VarioScreen_t* screen)
{
    assert(screen->widgetsAdded < max_Widgets);
    screen->widgets[screen->widgetsAdded] = *widget;
    screen->widgetsAdded++;
}

void VarioScreen_render(VarioScreen_t* screen)
{ 
    uint64_t lastTime = esp_timer_get_time();
    Paint_Clear(WHITE);
    for (size_t i = 0; i < screen->widgetsAdded; i++)
    {
        draw_Widget(&(screen->widgets[i]));
    }
    ESP_LOGI("VarioScreen", "TIMEDraw: %lli", (esp_timer_get_time() - lastTime) / 1000);
    lastTime = esp_timer_get_time();
    EPD_3IN7_1Gray_Display(screen->Screen_Buffer);
}