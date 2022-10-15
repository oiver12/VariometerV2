#include "lvgl/lvgl.h"
#include "Vario_Screen.h"
#include "DeviceContext.h"

#define COLOR_BLACK lv_color_hex(0x000000)
#define COLOR_RED lv_color_hex(0xFF0000)
#define COLOR_GREEN lv_color_hex(0x00FF00)
#define COLOR_YELLOW lv_color_hex(0xFFFF00)
#define COLOR_BLUE lv_color_hex(0x0000FF)
#define COLOR_MAGENTA lv_color_hex(0xFF00FF)
#define COLOR_CYAN lv_color_hex(0x00FFFF)
#define COLOR_WHITE lv_color_hex(0xFFFFFF)


LV_IMG_DECLARE(gps_an);
LV_IMG_DECLARE(gps_aus);
LV_IMG_DECLARE(lautsprecher_an);
LV_IMG_DECLARE(lautsprecher_stumm);
LV_IMG_DECLARE(sd_card_good);

float countS = 0;

void my_task(lv_task_t* task)
{
    vario_state_set_heading(countS);
    vario_state_set_vertVelocity(countS);
    countS += 1;
    if (((int)countS) % 20 == 0)
        ScreenManager_set_sdcard(false);
    if (countS >= 360)
        countS = 0;
    Screen_update_widgets((VarioScreen_t*)task->user_data);
}

void example_lvgl_demo_ui(lv_group_t* group)
{
    init_DeviceContext();

    static Vario_Screen_Manager_t screenManager;
    ScreenManager_init(&screenManager);

    static VarioScreen_t screen;
    ScreenManager_add_init_Screen(&screenManager, &screen);

    ScreenManager_show_Screen(&screenManager, 0);

    static WidgetData widgetData[2] = {
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_30_CONTENT, .h = 80, .w = 150, .style = Widget_TextBox, .type = WidgetContent_Vario_Active, .x = 0, .y = 25},
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 100, .style = Widget_Compass, .type = WidgetContent_CompassSensor, .x = 0, .y = 100},
    };
    Screen_add_widgets(&screen, widgetData, 2);
    vario_state_set_heading(90);
    lv_task_t* task = lv_task_create(my_task, 50, LV_TASK_PRIO_MID, &screen);
    //lv_group_add_obj(group, frame);
}
