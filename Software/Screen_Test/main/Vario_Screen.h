#ifndef __VarioScreen_H_
#define __VarioScreen_H_
#include "Widget_Vario.h"

LV_IMG_DECLARE(gps_an);
LV_IMG_DECLARE(gps_aus);
LV_IMG_DECLARE(lautsprecher_an);
LV_IMG_DECLARE(lautsprecher_stumm);
LV_IMG_DECLARE(sd_card_good);

#define MAX_SCREENS 5
#define MAX_WIDGETS_SCREEN 10

typedef struct {
    WidgetData widgets[MAX_WIDGETS_SCREEN];
    int widgetsAdded;
    lv_obj_t* screen_Obj;
} VarioScreen_t;

typedef struct {
    VarioScreen_t screens[MAX_SCREENS];
    size_t screensAdded;
    uint8_t curr_Screen;
} Vario_Screen_Manager_t;

void ScreenManager_init(Vario_Screen_Manager_t* screenManager);
void ScreenManager_deinit(VarioScreen_t* screenManager);
void ScreenManager_add_init_Screen(Vario_Screen_Manager_t* screenManager, VarioScreen_t* screen);
void ScreenManager_show_Screen(Vario_Screen_Manager_t* screenManager, size_t screenIndex);
void Screen_add_widgets(VarioScreen_t *screen, WidgetData *widgets, size_t size);
void Screen_update_widgets(VarioScreen_t* screen);

#endif
