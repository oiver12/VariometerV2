#include "Vario_Screen.h"
#include <stdio.h>

lv_obj_t* gps_obj;
lv_obj_t* lautsprecher_obj;
lv_obj_t* sdcard_obj;

void ScreenManager_init(Vario_Screen_Manager_t* screenManager)
{
    memset(screenManager->screens, 0, sizeof(screenManager->screens));
    screenManager->curr_Screen = 0;
    screenManager->screensAdded = 0;


    gps_obj = lv_img_create(lv_layer_top(), NULL);
    lv_img_set_src(gps_obj, &gps_an);
    lv_obj_align(gps_obj, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    lautsprecher_obj = lv_img_create(lv_layer_top(), NULL);
    lv_img_set_src(lautsprecher_obj, &lautsprecher_stumm);
    lv_obj_align(lautsprecher_obj, NULL, LV_ALIGN_IN_TOP_LEFT, 30, 0);

    sdcard_obj = lv_img_create(lv_layer_top(), NULL);
    lv_img_set_src(sdcard_obj, &sd_card_good);
    lv_obj_align(sdcard_obj, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 0);

    init_WidgetHandler();
}

void ScreenManager_set_gps(bool enabled)
{
    lv_img_set_src(gps_obj, enabled ? &gps_an : &gps_aus);
}

void ScreenManager_set_volume(float value)
{
    lv_img_set_src(lautsprecher_obj, value == 0 ? &lautsprecher_stumm : &lautsprecher_an);
}

void ScreenManager_set_sdcard(bool enabled)
{
    lv_img_set_src(sdcard_obj, enabled ? &sd_card_good : &sd_card_error);
}

void ScreenManager_deinit(VarioScreen_t* screenManager)
{

}
void ScreenManager_add_init_Screen(Vario_Screen_Manager_t* screenManager, VarioScreen_t* screen)
{
    if (screenManager->screensAdded >= MAX_SCREENS)
    {
        printf("To many Screens added!!");
        return;
    }
    memset(screen->widgets, 0, sizeof(screen->widgets));
    screen->widgetsAdded = 0;
    screen->screen_Obj = lv_obj_create(NULL, NULL);
    screenManager->screens[screenManager->screensAdded] = *screen;
    screenManager->screensAdded++;
}
void ScreenManager_show_Screen(Vario_Screen_Manager_t* screenManager, size_t screenIndex)
{
    if (screenManager->screensAdded <= screenIndex)
    {
        printf("Screen not added yet!!");
        return;
    }
    lv_disp_load_scr(screenManager->screens[screenIndex].screen_Obj);
}
void Screen_add_widgets(VarioScreen_t* screen, WidgetData* widgets, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        screen->widgets[screen->widgetsAdded++] = *(widgets + i);
        vario_add_widget(&screen->widgets[screen->widgetsAdded - 1], screen->screen_Obj);
    }
}

void Screen_update_widgets(VarioScreen_t* screen)
{
    for (size_t i = 0; i < screen->widgetsAdded; i++)
    {
        vario_update_widget(&screen->widgets[i]);
    }
}

void Screen_redraw_Screen()
{
    lv_obj_invalidate(lv_scr_act());
}
