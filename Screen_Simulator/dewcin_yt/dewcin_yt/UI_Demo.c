#include <stdio.h>

void startRender()
{
    init_DeviceContext();

    static Vario_Screen_Manager_t screenManager;
    ScreenManager_init(&screenManager);

    static VarioScreen_t screen;
    ScreenManager_add_init_Screen(&screenManager, &screen);

    ScreenManager_show_Screen(&screenManager, 0);

    static WidgetData widgetData[4] = {
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_30_CONTENT, .h = 80, .w = 150, .style = Widget_TextBox, .type = WidgetContent_Vario_Active, .x = 0, .y = 25},
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 100, .style = Widget_Compass, .type = WidgetContent_CompassSensor, .x = 0, .y = 103},
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 150, .style = Widget_Chart, .type = WidgetContent_AltitudeBaroChart, .x = 98, .y = 103},
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_30_CONTENT, .h = 80, .w = 150, .style = Widget_TextBox, .type = WidgetContent_VertAcc, .x = 148, .y = 25},
    };
    Screen_add_widgets(&screen, widgetData, 4);
}

void updateRender()
{

}