#ifndef __VarioScreen_H_
#define __VarioScreen_H_
#include "Widget_Vario.h"

#define MAX_SCREENS 5
#define MAX_WIDGETS_SCREEN 10
#define LONGPRESS_TIMEPERIOD 500 //ms

typedef enum {
    ScreenMode_Widget = 0,
    ScreenMode_Settings = 1,
    ScreenMode_PowerAccept = 2
} ScreenMode;

typedef struct {
    WidgetData widgets[MAX_WIDGETS_SCREEN];
    int widgetsAdded;
} VarioScreen_t;

typedef struct {
    VarioScreen_t screens[MAX_SCREENS];
    size_t screensAdded;
    uint8_t curr_Screen;
    ScreenMode screenMode;
    uint8_t isRendering;
} Vario_Screen_Manager_t;

void ScreenManager_init();
void ScreenManager_deinit();
void ScreenManager_show_Screen(size_t screenIndex);
void ScreenManager_set_gps(int enabled);
void ScreenManager_set_volume(int value);
void ScreenManager_set_sdcard(int enabled);
void ScreenManager_set_Battery(int percentage);
void ScreenManager_update();
void ScreenManager_powerButtonPressed(uint32_t pressTimeMS);
void ScreenManager_LeftArrowPressed();
void ScreenManager_RightArrowPressed();
void ScreenManager_EnterKeyPressed();
void Screen_redraw_Screen();
void ScreenManager_add_Screen_add_widgets(WidgetData* widgets, size_t size);

#endif
