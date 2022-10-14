#include "Vario_Screen.h"
#include <stdio.h>

void ScreenManager_init(Vario_Screen_Manager_t* screenManager)
{
    memset(screenManager->screens, 0, sizeof(screenManager->screens));
    screenManager->curr_Screen = 0;
    screenManager->screensAdded = 0;


    lv_obj_t* gps_img = lv_img_create(lv_layer_top(), NULL);
    lv_img_set_src(gps_img, &gps_an);
    lv_obj_align(gps_img, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    lv_obj_t* lautsprecher_img = lv_img_create(lv_layer_top(), NULL);
    lv_img_set_src(lautsprecher_img, &lautsprecher_stumm);
    lv_obj_align(lautsprecher_img, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 0);

    lv_obj_t* sd_card_img = lv_img_create(lv_layer_top(), NULL);
    lv_img_set_src(sd_card_img, &sd_card_good);
    lv_obj_align(sd_card_img, NULL, LV_ALIGN_IN_TOP_LEFT, 40, 0);

    init_WidgetHandler();
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
