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

    sd_card_state_t sdcard_state = {
        .status = SD_CARD_STATE_INIT | SD_CARD_STATE_IGC_LOGGING,
        .maxSpace = 4294967296,
        .freeSpace = 2147483648
    };
    device_state_set_member(&sdcard_state, device_state_statusSDCard);

    float pressure = 101325;
    vario_state_set_member(&pressure, vario_state_pressure);

    uint16_t volume = 10000;
    vario_settings_set_member(&volume, vario_settings_soundVolume);

    ScreenManager_init();
    
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
    static WidgetData widgetDataScreen1[1] = {
        {.extraStyle = WS_FONT_32_CONTENT, .h = 90, .w = 200, .style = Widget_SdCardInfo, .type = WidgetContent_SDCardInfo, .x = 1, .y = 27},
    };
    ScreenManager_add_Screen_add_widgets(widgetDataScreen1, 1);
    ScreenManager_show_Screen(0);
}

void updateRender()
{
    ScreenManager_update();
}

void startDownArrow()
{
    if (!startedLeft)
        startedLeft = TRUE;
}

void DownArrowReleased()
{
    if (startedLeft)
    {
        ScreenManager_DownArrowPressed();
        startedLeft = FALSE;
    }
}

void startUpArrow()
{
    if (!startedRight)
        startedRight = TRUE;
}

void UpArrowReleased()
{
    if (startedRight)
    {
        ScreenManager_UpArrowPressed();
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