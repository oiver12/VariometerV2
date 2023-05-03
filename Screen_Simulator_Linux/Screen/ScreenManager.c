#include "ScreenManager.h"
#include <stdio.h>
#include "E_Ink/GUI_Paint.h"
#include <string.h>
#include <math.h>
#include "DeviceContext.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "Display_Vario.h"
#include "RenderSystem.h"
#include "assets/assets.h"

const unsigned char gImage_PowerOff[1300] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xE0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xC0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7F, 0xFF, 0xF0, 0xFF, 0xFF, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x60, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0xF0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7, 0x1, 0xF8, 0xE, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x1F, 0x81, 0xF8, 0x1F, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x1F, 0x81, 0xF8, 0x1F, 0x80, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x3F, 0x81, 0xF8, 0x1F, 0xC0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7F, 0x1, 0xF8, 0xF, 0xE0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xFE, 0x1, 0xF8, 0x7, 0xE0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xFC, 0x0, 0xF0, 0x3, 0xF0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x3, 0xF0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xF0, 0x0, 0x0, 0x0, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xF0, 0x0, 0x0, 0x0, 0xFC, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0xFC, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0x7C, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xE0, 0x0, 0x0, 0x0, 0xFC, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xF0, 0x0, 0x0, 0x0, 0xFC, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x3, 0xF0, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x1, 0xF8, 0x0, 0x0, 0x1, 0xF8, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xFC, 0x0, 0x0, 0x3, 0xF0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xFE, 0x0, 0x0, 0x7, 0xF0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7F, 0x0, 0x0, 0xF, 0xE0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7F, 0x80, 0x0, 0x1F, 0xC0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x3F, 0xC0, 0x0, 0x3F, 0xC0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x1F, 0xF0, 0x0, 0xFF, 0x80, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0xF, 0xFF, 0xF, 0xFF, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x7, 0xFF, 0xFF, 0xFE, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x3, 0xFF, 0xFF, 0xF8, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xF0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x3F, 0xFF, 0xC0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x7, 0xFE, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3F, 0xFF, 0xF0, 0xFF, 0xFF, 0xC0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7F, 0xFF, 0xF0, 0xFF, 0xFF, 0xE0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0 };

WidgetData widgetData_Audio_Settings[2] = {
        {.extraStyle = 0, .h = 40, .w = 459, .style = Widget_Settings_Slider, .type = WidgetContent_Settings_VolumeSlider},
        {.extraStyle = 0, .h = 100, .w = 459, .style = Widget_Settings_SoundThreshold, .type = WidgetContent_Settings_SoundLiftThreshold},
};

WidgetData widgetData_Config_Settings[1] = {
        {.extraStyle = 0, .h = 60, .w = 459, .style = Widget_Settings_CalibrationPageButton, .type = WidgetContent_Settings_MagnCalibrationPageButton},
};

Vario_Screen_Manager_t screenManagerInstance;
static SemaphoreHandle_t buffer_Mutex = NULL;

//Zeichne die Status Bar am oberen Rand.
void ScreenManager_draw_status_bar()
{
    //Lösche die alte Anzeige
    Paint_ClearWindows(0, 0, Paint.Width, 24, WHITE);

    gps_fix_mode_t gpsState;
    device_state_get_member(&gpsState, device_state_statusGPS, true);
    //Draw GPS Icon
    if (gpsState == GPS_MODE_INVALID)
        Paint_DrawBitMap_Paste(gImage_gps_aus, 0, 0, 24, 24, FALSE);
    else
        Paint_DrawBitMap_Paste(gImage_gps_an, 0, 0, 24, 24, FALSE);

    uint16_t volume = 0;
    vario_settings_get_member(&volume, vario_settings_soundVolume, true);
    //Draw Lautsprecher Icon
    if (volume == 0)
        Paint_DrawBitMap_Paste(gImage_lautsprecher_aus, 30, 0, 24, 24, FALSE);
    else
        Paint_DrawBitMap_Paste(gImage_lautsprecher_an, 30, 0, 24, 24, FALSE);

    //Draw Sd-Card Icon
    sd_card_state_t sdcard_state;
    device_state_get_member(&sdcard_state, device_state_statusSDCard, true);
    if (sdcard_state.status | SD_CARD_STATE_INIT)
        Paint_DrawBitMap_Paste(gImage_sd_card_good, 60, 0, 24, 24, FALSE);
    else
        Paint_DrawBitMap_Paste(gImage_sd_card_error, 60, 0, 24, 24, FALSE);

    float percentage = 0;
    device_state_get_member(&percentage, device_state_batteryPercentage, true);
    //Draw Battery Icon and Texts
    if (percentage < 10)
        Paint_DrawBitMap_Paste(gImage_battery_empty, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
    else if (percentage < 25)
        Paint_DrawBitMap_Paste(gImage_battery_halfEmpty, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
    else if (percentage < 50)
        Paint_DrawBitMap_Paste(gImage_battery_half, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
    else if (percentage < 75)
        Paint_DrawBitMap_Paste(gImage_battery_halfFull, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
    else
    {
        Paint_DrawBitMap_Paste(gImage_battery_full, Paint.Width - 24 - 2, 0, 24, 24, FALSE);
        if (percentage >= 100)
            percentage = 99;
    }
    //Clear Surface from old Text
    Paint_ClearWindows(Paint.Width - 26 - 50, 5, Paint.Width - 26, 25, WHITE);
    char buffer[5];
    snprintf(buffer, 5, "%i%%", (int)(percentage));
    uint16_t stringWidth = strlen(buffer) * Font16.Width;
    Paint_DrawString_EN(Paint.Width - 26 - stringWidth, 8, buffer, &Font16, WHITE, BLACK, FALSE);
}

static void init_SettingsWidgets()
{
    WidgetData pageButtonSettingsWidget[2] = {
        {.x = 10, .y = 27, .w = 459, .h = 40, .style = Widget_Settings_PageButton, .type = WidgetContent_Settings_PageButton_Audio, .extraStyle = 0},
        {.x = 10, .y = 67, .w = 459, .h = 40, .style = Widget_Settings_PageButton, .type = WidgetContent_Settings_PageButton_Config, .extraStyle = 0}
    };
    WidgetData backButtonWidget = { .x = 10, .y = 27, .w = 459, .h = 40, .style = Widget_settings_BackButton, .extraStyle = 0 };

    /* Init Page Buttons Setting Widgets */
    widget_Settings_linkedList* backButtonItem = calloc(1, sizeof(widget_Settings_linkedList));
    backButtonWidget.type = WidgetContent_Settings_PageButton;
    backButtonItem->data = backButtonWidget;
    screenManagerInstance.settingsScreen.headListPageButtons = backButtonItem;
    screenManagerInstance.settingsScreen.topFielPageButtons = backButtonItem;
    widget_Settings_linkedList* prev = backButtonItem;
    widget_Settings_linkedList* listElement;
    for (size_t i = 0; i < sizeof(pageButtonSettingsWidget) / sizeof(WidgetData); i++)
    {
        listElement = calloc(1, sizeof(widget_Settings_linkedList));
        listElement->prev = prev;
        prev->next = listElement;
        listElement->data = pageButtonSettingsWidget[i];
        listElement->data.x = 10;
        prev = listElement;
    }
    //make it circular (skip to to top when all the way down)
    listElement->next = backButtonItem;
    backButtonItem->prev = listElement;

    /* Init Audio Setting Widgets */
    backButtonItem = calloc(1, sizeof(widget_Settings_linkedList));
    backButtonWidget.type = WidgetContent_Settings_PageButton_Audio;
    backButtonItem->data = backButtonWidget;
    screenManagerInstance.settingsScreen.headListAudio = backButtonItem;
    screenManagerInstance.settingsScreen.topFieldAudio = backButtonItem;
    prev = backButtonItem;
    for (size_t i = 0; i < sizeof(widgetData_Audio_Settings) / sizeof(WidgetData); i++)
    {
        listElement = calloc(1, sizeof(widget_Settings_linkedList));
        listElement->prev = prev;
        prev->next = listElement;
        listElement->data = widgetData_Audio_Settings[i];
        listElement->data.x = 10;
        prev = listElement;
    }
    listElement->next = backButtonItem;
    backButtonItem->prev = listElement;

    /* Init Config Setting Widget */
    backButtonItem = calloc(1, sizeof(widget_Settings_linkedList));
    backButtonWidget.type = WidgetContent_Settings_PageButton_Config;
    backButtonItem->data = backButtonWidget;
    screenManagerInstance.settingsScreen.headListConfig = backButtonItem;
    screenManagerInstance.settingsScreen.topFieldConfig = backButtonItem;
    prev = backButtonItem;
    for (size_t i = 0; i < sizeof(widgetData_Config_Settings) / sizeof(WidgetData); i++)
    {
        listElement = calloc(1, sizeof(widget_Settings_linkedList));
        listElement->prev = prev;
        prev->next = listElement;
        listElement->data = widgetData_Config_Settings[i];
        listElement->data.x = 10;
        prev = listElement;
    }
    listElement->next = backButtonItem;
    backButtonItem->prev = listElement;
}

/* Initialisiere den Bildschirm Manager(initialisiere eine Singleton)
* Zeichne auch die Status Bar
*/
void ScreenManager_init(SemaphoreHandle_t semaphore)
{
    buffer_Mutex = semaphore;
    memset(screenManagerInstance.screens, 0, sizeof(screenManagerInstance.screens));
    screenManagerInstance.curr_Screen = 0;
    screenManagerInstance.screensAdded = 0;
    screenManagerInstance.screenMode = ScreenMode_Widget;

    ScreenManager_draw_status_bar();

    init_WidgetHandler();

    init_SettingsWidgets();
}

//TODO: Implement this function. Free different buffers
void ScreenManager_deinit()
{

}

/*
* Zeige einen anderen Bildschirm an mit anderen Widgets
* Es wird zuerst mit Screeen_redraw_Screen() die alte Anzeige gelöscht
* und die Widgets initialisieren. Wenn Widgets refreshed werden, wird dann der
* neue Bildschirm refreshed
*/
void ScreenManager_show_Screen(size_t screenIndex)
{
    if (screenManagerInstance.screensAdded <= screenIndex)
    {
        printf("Screen not added yet!\n");
        return;
    }
    if (screenManagerInstance.curr_Screen == screenIndex)
    {
        printf("Already displaying this Screen, force redraw with Screen_redraw_Screen()");
        return;
    }
    screenManagerInstance.curr_Screen = screenIndex;
    Screen_redraw_Screen();
    rendering_Update(true);
}

/*
* Füge einen neuen Bildschirm mit den mitgegeben widgets hinzu.
* Ein neuer Bildschirm wird automatisch angelegt
*/
void ScreenManager_add_Screen_add_widgets(WidgetData* widgets, size_t size)
{
    if (screenManagerInstance.screensAdded >= MAX_SCREENS - 1)
    {
        printf("To many Screens added!\n");
        return;
    }
    VarioScreen_t* screen = &screenManagerInstance.screens[screenManagerInstance.screensAdded];
    screenManagerInstance.screensAdded++;
    for (size_t i = 0; i < size; i++)
    {
        //screen->widgets[screen->widgetsAdded++] = *(widgets + i);
        memcpy(&screen->widgets[screen->widgetsAdded], (widgets + i), sizeof(WidgetData));
        //Render this Screen, if it's the current Screen
        if (screenManagerInstance.curr_Screen == screenManagerInstance.screensAdded - 1)
            vario_display_add_widget(&screen->widgets[screen->widgetsAdded]);
        screen->widgetsAdded++;
    }
}

/*
* Update den Screen je nach screenMode momentan
*/
bool ScreenManager_update()
{
    if (screenManagerInstance.screenMode == ScreenMode_Widget)
    {
        for (size_t i = 0; i < screenManagerInstance.screens[screenManagerInstance.curr_Screen].widgetsAdded; i++)
        {
            if (screenManagerInstance.screenMode != ScreenMode_Widget)
                return false;
            vario_update_widget(&screenManagerInstance.screens[screenManagerInstance.curr_Screen].widgets[i]);
        }
        return true;
    }
    return false;
}

static void draw_Settings_Scrollbar()
{
    uint32_t currY = 28;
    widget_Settings_linkedList* startElement = screenManagerInstance.settingsScreen.topFielPageButtons;
    if (screenManagerInstance.settingsScreen.currPage == Settings_Page_Audio)
    {
        startElement = screenManagerInstance.settingsScreen.topFieldAudio;
    }
    else if (screenManagerInstance.settingsScreen.currPage == Settings_Page_Config)
    {
        startElement = screenManagerInstance.settingsScreen.topFieldConfig;
    }
    Paint_ClearWindows(0, currY, Paint.Width, Paint.Height, WHITE);
    widget_Settings_linkedList* element = startElement;
    do
    {
        element->data.y = currY;
        if (screenManagerInstance.settingsScreen.hoveredField != element)
            vario_display_add_settings_widget(&element->data, screenManagerInstance.settingsScreen.currPage, false, screenManagerInstance.settingsScreen.selectedField == element, false);
        currY += element->data.h;
        element = element->next;
    } while(element != startElement && currY + element->data.h <= Paint.Height);
    vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, true, screenManagerInstance.settingsScreen.selectedField == element, false);
}

static void settings_enter_press()
{
    //Erste Page Button Seite
    if (screenManagerInstance.settingsScreen.currPage == Settings_Page_PageButtons)
    {
        switch (screenManagerInstance.settingsScreen.hoveredField->data.type)
        {
        case WidgetContent_Settings_PageButton:
            screenManagerInstance.screenMode = ScreenMode_Widget;
            Screen_redraw_Screen();
            rendering_Update(true);
            return;
        case WidgetContent_Settings_PageButton_Audio:
            screenManagerInstance.settingsScreen.currPage = Settings_Page_Audio;
            screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.topFieldAudio;
            break;
        case WidgetContent_Settings_PageButton_Config:
            screenManagerInstance.settingsScreen.currPage = Settings_Page_Config;
            screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.topFieldConfig;
            break;
        default:
            printf("Page Button not found\n");
            break;
        }
        xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
        draw_Settings_Scrollbar();
        xSemaphoreGive(buffer_Mutex);
        rendering_Push(false);
    }
    else
    {
        //deselect field
        if (screenManagerInstance.settingsScreen.selectedField)
        {
            screenManagerInstance.settingsScreen.selectedField = NULL;
            xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
            vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, true, false, true);
            xSemaphoreGive(buffer_Mutex);
            rendering_Push(false);
        }
        else
        {
            if (screenManagerInstance.settingsScreen.hoveredField->data.style == Widget_settings_BackButton)
            {
                screenManagerInstance.settingsScreen.currPage = Settings_Page_PageButtons;
                screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.topFielPageButtons;
                xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
                draw_Settings_Scrollbar();
                xSemaphoreGive(buffer_Mutex);
                rendering_Push(false);
                return;
            }
            //select field
            screenManagerInstance.settingsScreen.selectedField = screenManagerInstance.settingsScreen.hoveredField;
            xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
            vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.selectedField->data, screenManagerInstance.settingsScreen.currPage, true, true, false);
            xSemaphoreGive(buffer_Mutex);
            printf("Finished\n");
            rendering_Push(false);
        }
    }
}

void ScreenManager_deselctSettingsWidget()
{
    screenManagerInstance.settingsScreen.selectedField = NULL;
    xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
    vario_display_add_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, true, false, true);
    xSemaphoreGive(buffer_Mutex);
    rendering_Push(false);
}

static void ScreenManager_openSettings()
{
    xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
    Paint_Clear(WHITE);
    ScreenManager_draw_status_bar();
    screenManagerInstance.screenMode = ScreenMode_Settings;
    screenManagerInstance.settingsScreen.currPage = Settings_Page_PageButtons;
    screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.topFielPageButtons;
    screenManagerInstance.settingsScreen.selectedField = NULL;
    draw_Settings_Scrollbar();
    xSemaphoreGive(buffer_Mutex);
    rendering_Push(true);
}

static void settings_up_press()
{
    if (!screenManagerInstance.settingsScreen.selectedField)
    {
        if (screenManagerInstance.settingsScreen.hoveredField->prev)
        {
            xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
            vario_display_select_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, false, false);
            screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.hoveredField->prev;
            //we are hovering a field
            vario_display_select_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, true, false);
            xSemaphoreGive(buffer_Mutex);
            rendering_Push(false);
        }
    }
    else
    {
        xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
        //change Value in Field
        vario_display_settings_widget_handle_input(&screenManagerInstance.settingsScreen.selectedField->data, Button_Direction_up);
        xSemaphoreGive(buffer_Mutex);
        rendering_Push(false);
    }
}

static void settings_down_press()
{
    if (!screenManagerInstance.settingsScreen.selectedField)
    {
        //we are hovering a field
        if (screenManagerInstance.settingsScreen.hoveredField->next)
        {
            //TODO: Go to top of list, when last element
            xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
            vario_display_select_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, false, false);
            screenManagerInstance.settingsScreen.hoveredField = screenManagerInstance.settingsScreen.hoveredField->next;
            vario_display_select_settings_widget(&screenManagerInstance.settingsScreen.hoveredField->data, screenManagerInstance.settingsScreen.currPage, true, false);
            xSemaphoreGive(buffer_Mutex);
            rendering_Push(false);
        }
    }
    else
    {
        xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
        //change Value in Field
        vario_display_settings_widget_handle_input(&screenManagerInstance.settingsScreen.selectedField->data, Button_Direction_down);
        xSemaphoreGive(buffer_Mutex);
        rendering_Push(false);
    }
}

/*
* Wenn der PowerButton gedrüct wird, kann entweder das Menu oder das Abschalten angezeigt werden
*/
void ScreenManager_powerButtonPressed(bool longPress)
{
    if (screenManagerInstance.screenMode != ScreenMode_PowerAccept)
    {
        if (longPress)
        {
            screenManagerInstance.screenMode = ScreenMode_PowerAccept;
            xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
            Paint_Clear(WHITE);
            Paint_DrawBitMap_Paste(gImage_PowerOff, Paint.Width / 2 - 50, 50, 100, 100, FALSE);
            const char* stringToShow = "Confirm Power Off";
            uint32_t stringLength = strlen(stringToShow) * Font24.Width;
            Paint_DrawString_EN(Paint.Width / 2 - stringLength / 2, 175, stringToShow, &Font24, WHITE, BLACK, FALSE);
            const char* nextString = "Press the power button again to turn off the device";
            const char* nextString1 = "Press any other button to cancel";
            stringLength = strlen(nextString) * Font12.Width;
            Paint_DrawString_EN(Paint.Width / 2 - stringLength / 2, 220, nextString, &Font12, WHITE, BLACK, FALSE);
            stringLength = strlen(nextString1) * Font12.Width;
            Paint_DrawString_EN(Paint.Width / 2 - stringLength / 2, 240, nextString1, &Font12, WHITE, BLACK, FALSE);
            xSemaphoreGive(buffer_Mutex);
            rendering_Push(false);
            return;
        }
        else
        {
            if (screenManagerInstance.screenMode == ScreenMode_Widget)
                ScreenManager_openSettings();
            else if (screenManagerInstance.screenMode == ScreenMode_Settings)
            {
                screenManagerInstance.screenMode = ScreenMode_Widget;
                rendering_Update(true);
                Screen_redraw_Screen();
            }
        }
    }
    else
    {
        //PowerOff
        ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(2, 0));
        Display_powerOff_Screen();
        gpio_set_level(13, 0);
        esp_deep_sleep_start();
    }
}

/*
* Bei Widgets anzeige, wird der nächste Bildschirm angezeigt
* Bei Power Accept, soll gecancelt werden
*/
void ScreenManager_DownArrowPressed()
{
    if (screenManagerInstance.screenMode == ScreenMode_Widget)
    {
        //When there is only one Screen, we dont have to rerender something
        if (screenManagerInstance.screensAdded > 1)
        {
            if (screenManagerInstance.curr_Screen == 0)
                ScreenManager_show_Screen(screenManagerInstance.screensAdded - 1);
            else
                ScreenManager_show_Screen(screenManagerInstance.curr_Screen - 1);
        }
    }
    else if (screenManagerInstance.screenMode == ScreenMode_PowerAccept)
    {
        //TODO: evtl. in letzte Seite zurückkehren und nicht einfach Widgets laden
        screenManagerInstance.screenMode = ScreenMode_Widget;
        Screen_redraw_Screen();
        rendering_Update(false);
    }
    else if (screenManagerInstance.screenMode == ScreenMode_Settings)
    {
        settings_down_press();
    }
}

void ScreenManager_UpArrowPressed()
{
    if (screenManagerInstance.screenMode == ScreenMode_Widget)
    {
        //When there is only one Screen, we dont have to Rerender something
        if (screenManagerInstance.screensAdded > 1)
        {
            if (screenManagerInstance.curr_Screen == screenManagerInstance.screensAdded - 1)
                ScreenManager_show_Screen(0);
            else
                ScreenManager_show_Screen(screenManagerInstance.curr_Screen + 1);
        }
    }
    else if (screenManagerInstance.screenMode == ScreenMode_PowerAccept)
    {
        //TODO: evtl. in letzte Seite zurückkehren und nicht einfach Widgets laden
        screenManagerInstance.screenMode = ScreenMode_Widget;
        Screen_redraw_Screen();
        rendering_Update(false);
    }
    else if (screenManagerInstance.screenMode == ScreenMode_Settings)
    {
        settings_up_press();
    }
}

void ScreenManager_EnterKeyPressed()
{
    if (screenManagerInstance.screenMode == ScreenMode_PowerAccept)
    {
        //TODO: evtl. in letzte Seite zurückkehren und nicht einfach Widgets laden
        screenManagerInstance.screenMode = ScreenMode_Widget;
        Screen_redraw_Screen();
        rendering_Update(false);
    }
    else if (screenManagerInstance.screenMode == ScreenMode_Settings)
    {
        settings_enter_press();
    }
}

/*
* Zeichen den ganzen Bildschirm neu. Z.B. wenn ein neuer Bildschirm geladen wird (es werden z.B. alle Frames von den Widgets gezeichnet)
*/
void Screen_redraw_Screen()
{
    //Clear Screen and redraw all widgets
    if (screenManagerInstance.screenMode == ScreenMode_Widget)
    {
        xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
        Paint_Clear(WHITE);
        ScreenManager_draw_status_bar();
        for (size_t i = 0; i < screenManagerInstance.screens[screenManagerInstance.curr_Screen].widgetsAdded; i++)
        {
            vario_display_add_widget(&screenManagerInstance.screens[screenManagerInstance.curr_Screen].widgets[i]);
        }
        xSemaphoreGive(buffer_Mutex);
    }
}

bool ScreenManager_showPopUp(PopUpConfig_t config)
{
    //TODO: add Posibility to do Pop Up over Pop Up or save PopUp in Queue or something like that.
    if (!screenIsInit() || screenManagerInstance.screenMode == ScreenMode_PopUp)
        return false;
    screenManagerInstance.screenMode = ScreenMode_PopUp; 
    xSemaphoreTake(buffer_Mutex, portMAX_DELAY);
    vario_display_showPopUp(config);
    xSemaphoreGive(buffer_Mutex);
    rendering_Push(false); 
    return true; 
}

void ScreenManager_closePopUp()
{
    screenManagerInstance.screenMode = ScreenMode_Widget;
    Screen_redraw_Screen();
    rendering_Update(false);
}
