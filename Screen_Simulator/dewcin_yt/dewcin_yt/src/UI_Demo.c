#include <stdio.h>
#include "Screen/DeviceContext.h"
#include "Screen/Vario_Screen.h"
#include "Screen/E-Ink/GUI_Paint.h"
#include <time.h>

float startTimeDown = 0.0f;
uint8_t startedDown = FALSE;
uint8_t startedLeft = FALSE;
uint8_t startedRight = FALSE;
uint8_t startedEnter = FALSE;

void startRender()
{
    init_DeviceContext();

    ScreenManager_init();
    
    static WidgetData widgetDataScreen0[5] = {
        {.extraStyle = WS_FONT_24_CONTENT, .h = 80, .w = 150, .style = Widget_TextBox, .type = WidgetContent_Vario_Active, .x = 1, .y = 27},
        {.extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 100, .style = Widget_Compass, .type = WidgetContent_CompassSensor, .x = 1, .y = 107},
        {.extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 150, .style = Widget_Chart, .type = WidgetContent_AltitudeBaroChart, .x = 101, .y = 107},
        {.extraStyle = WS_FONT_24_CONTENT, .h = 80, .w = 150, .style = Widget_TextBox, .type = WidgetContent_VertAcc, .x = 151, .y = 27},
        {.extraStyle = WS_FONT_16_CONTENT, .h = 80, .w = 150, .style = Widget_TextBox, .type = WidgetContent_Pressure, .x = 301, .y = 27},
    };
    ScreenManager_add_Screen_add_widgets(widgetDataScreen0, 5);
    static WidgetData widgetDataScreen1[1] = {
        {.extraStyle = WS_FONT_32_CONTENT, .h = 150, .w = 200, .style = Widget_SdCardInfo, .type = WidgetContent_SDCardInfo, .x = 1, .y = 27},
    };
    ScreenManager_add_Screen_add_widgets(widgetDataScreen1, 1);
    ScreenManager_show_Screen(1);

    float pressure = 101325;
    vario_state_set_member(&pressure, vario_state_pressure);
}

void updateRender()
{
    ScreenManager_update();
}

void startLeftArrow()
{
    if (!startedLeft)
        startedLeft = TRUE;
}

void LeftArrowReleased()
{
    if (startedLeft)
    {
        ScreenManager_LeftArrowPressed();
        startedLeft = FALSE;
    }
}

void startRightArrow()
{
    if (!startedRight)
        startedRight = TRUE;
}

void RightArrowReleased()
{
    if (startedRight)
    {
        ScreenManager_RightArrowPressed();
        startedRight = FALSE;
    }
}

void startEnterKey()
{
    if (!startedEnter)
        startedEnter = TRUE;
}

void EnterKeyReleased()
{
    if (startedEnter)
    {
        ScreenManager_EnterKeyPressed();
        startedEnter = FALSE;
    }
}

void startMenuKey()
{
    if (!startedDown)
    {
        startTimeDown = (float)clock() / CLOCKS_PER_SEC; //in seconds
        startedDown = TRUE;
    }
}

void MenuKeyReleased()
{
    if (startedDown)
    {
        float difference = ((float)clock() / CLOCKS_PER_SEC) - startTimeDown;
        startedDown = FALSE;
        ScreenManager_powerButtonPressed(difference * 1000);
    }
}