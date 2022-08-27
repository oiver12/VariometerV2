#ifndef __VarioScreen_H_
#define __VarioScreen_H_

#include "Widget.h"
#include "EPD_3in7.h"
#include "GUI_Paint.h"
#include "freertos/FreeRTOS.h"
#include "main.h"
#include "freertos/semphr.h"

#define max_Widgets 10

typedef struct{
    UBYTE *Screen_Buffer;
    WidgetData* widgets;
    int widgetsAdded;
} VarioScreen_t;

UBYTE* init_Screen(vario_state_t* state, SemaphoreHandle_t vario_state_Mutex);
void deinit_Screen(VarioScreen_t* screen);
void CreateScreen(VarioScreen_t * screen, UBYTE* bufffer);
void selectScreen(VarioScreen_t* screen);
void VarioScreen_add_Widget(WidgetData* widget, VarioScreen_t* screen);
void VarioScreen_render(VarioScreen_t* screen);

#endif