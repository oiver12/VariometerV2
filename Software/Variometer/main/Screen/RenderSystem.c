#include "RenderSystem.h"
#include "E_Ink/EPD_3in7.h"
#include "E_Ink/GUI_Paint.h"
#include "Vario_Screen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

RenderModes_t currentRenderMode = RenderMode_Normal;
bool isSending = false;
bool settingsRender = false;
bool fullRedraw = true;
static SemaphoreHandle_t buffer_Mutex = NULL;
QueueHandle_t renderQueue = NULL;

void rendering_Task(void *pvParameters)
{
    uint8_t* ScreenBuffer = (uint8_t*)pvParameters;
    uint32_t counter = 0;
    Display_RefreshMode refreshMode = PART_G_T_BW;
    while (1)
    {
        if (currentRenderMode == RenderMode_Normal)
        {
            if(counter % 10 == 0)
                ScreenManager_draw_status_bar();

            xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
            bool screenUpdate = ScreenManager_update();
            xSemaphoreGive(buffer_Mutex);  
            if(screenUpdate)
            {
                refreshMode = fullRedraw || counter % 100 == 0 ? FULL_G_T_BW : PART_BW_T_BW;
                xQueueSend(renderQueue, &refreshMode, portMAX_DELAY);
            }
            counter++;
            fullRedraw = false;
        }
        else
            vTaskDelay(20 / portTICK_RATE_MS);
    }
}

void displayRefreshTask(void *pvParameters)
{
    uint8_t* ScreenBuffer = (uint8_t*)pvParameters;
    Display_RefreshMode refreshMode;
    while(1)
    {
        xQueueReceive(renderQueue, &refreshMode, portMAX_DELAY);
        printf("sixth %i\n", (int)esp_timer_get_time());
        xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
        printf("seventh %i\n", (int)esp_timer_get_time());
        EDP_3IN7_1Gray_Start(ScreenBuffer, refreshMode);
        printf("eight %i\n", (int)esp_timer_get_time());
        xSemaphoreGive(buffer_Mutex);
        EPD_3IN7_1Gray_WaitBusy();
    }
}

void startRendering(uint8_t* ScreenBuffer)
{
    DEV_Module_Init();
    EPD_3IN7_1Gray_Init();       //init 1 Gray mode
    EPD_3IN7_1Gray_Clear();

    buffer_Mutex = xSemaphoreCreateMutex();
    ScreenManager_init(buffer_Mutex);
    
    static WidgetData widgetDataScreen0[7] = {
        {.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Vario_Active, .x = 0, .y = 27},
        {.extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 100, .style = Widget_Compass, .type = WidgetContent_CompassSensor, .x = 0, .y = 87},
        {.extraStyle = WS_FONT_20_CONTENT, .h = 193, .w = 180, .style = Widget_Chart, .type = WidgetContent_Altitude_Baro, .x = 100, .y = 87},
        {.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_VertAcc, .x = 160, .y = 27},
        {.extraStyle = WS_FONT_16_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Pressure, .x = 320, .y = 27},
        {.extraStyle = WS_FONT_32_CONTENT, .h = 100, .w = 200, .style = Widget_SdCardInfo, .type = WidgetContent_SDCardInfo, .x = 280, .y = 87},
		{.extraStyle = WS_FONT_24_CONTENT, .h = 63, .w = 200, .style = Widget_TextBox, .type = WidgetContent_Vario_Lazy, .x = 280, .y = 187},
    };
    ScreenManager_add_Screen_add_widgets(widgetDataScreen0, 7);
    ScreenManager_show_Screen(0);
    
    renderQueue = xQueueCreate(1, sizeof(Display_RefreshMode));

    xTaskCreate(rendering_Task, "rendering_Task", 4096, ScreenBuffer, 10, NULL);
    xTaskCreate(displayRefreshTask, "displayRefreshTask", 4096, ScreenBuffer, 10, NULL);
}
void stopRendering_disableScreen()
{
}
void rendering_Settings(bool varfullRedraw)
{
    currentRenderMode = RenderMode_Settings;
    settingsRender = true;
    fullRedraw = varfullRedraw;
    Display_RefreshMode refreshMode = fullRedraw ? FULL_G_T_BW : PART_BW_T_BW;
    printf("fifth %i\n", (int)esp_timer_get_time());
    xQueueOverwrite(renderQueue, &refreshMode);
}
void rendering_Normal(bool varfullRedraw)
{
    currentRenderMode = RenderMode_Normal;
    fullRedraw = varfullRedraw;
}