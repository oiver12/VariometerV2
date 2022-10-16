#ifndef __WIDGETVARIO_H
#define __WIDGETVARIO_H

#include <stdint.h>
#include "lvgl/lvgl.h"

#define COLOR_BLACK lv_color_hex(0x000000)
#define COLOR_RED lv_color_hex(0xFF0000)
#define COLOR_GREEN lv_color_hex(0x00FF00)
#define COLOR_YELLOW lv_color_hex(0xFFFF00)
#define COLOR_BLUE lv_color_hex(0x0000FF)
#define COLOR_MAGENTA lv_color_hex(0xFF00FF)
#define COLOR_CYAN lv_color_hex(0x00FFFF)
#define COLOR_WHITE lv_color_hex(0xFFFFFF)

#define WS_FONT_MASK_LABEL		0x0000000F
#define WS_FONT_8_LABEL			0x00000000
#define WS_FONT_12_LABEL		0x00000001
#define WS_FONT_16_LABEL		0x00000002
#define WS_FONT_20_LABEL		0x00000003
#define WS_FONT_24_LABEL		0x00000004

#define WS_FONT_MASK_CONTENT	0x000000F0
#define WS_FONT_8_CONTENT		0x00000000
#define WS_FONT_12_CONTENT		0x00000010
#define WS_FONT_16_CONTENT		0x00000020
#define WS_FONT_20_CONTENT		0x00000030
#define WS_FONT_22_CONTENT		0x00000040
#define WS_FONT_24_CONTENT		0x00000050
#define WS_FONT_26_CONTENT		0x00000060
#define WS_FONT_28_CONTENT		0x00000070
#define WS_FONT_30_CONTENT		0x00000080
#define WS_FONT_32_CONTENT		0x00000090

//Wie sieht das Widget aus (TextBox, Compass, etc.)
typedef enum
{
    Widget_Empty,
    Widget_TextBox,
    Widget_Compass,
    Widget_Chart,
} WidgetStyle;

//was ist im Widget dargestellt (z.B. Vario_Active in TextBox)
typedef enum
{
    /* Empty Widget */
    WidgetContent_Empty,

    /* Text Box Widgets */
    WidgetContent_Speed_Ground,
    WidgetContent_Vario_Active,
    WidgetContent_Vario_Lazy,
    WidgetContent_VertAcc,
    WidgetContent_Pressure,
    WidgetContent_Temperature,
    /*WidgetContent_Altitude_GPS,		// QNH
    WidgetContent_Altitude_Baro,	// QNE
    WidgetContent_Altitude_AGL,		// QFE*/

    /* Compass Widgets */
    WidgetContent_CompassSensor,

    /* Chart Widgets */
    WidgetContent_AltitudeBaroChart,
} WidgetContentType;

typedef struct
{
    lv_align_t          alignement;
    WidgetStyle			style;
    uint32_t			extraStyle;
    WidgetContentType	type;
    uint16_t			x, y, w, h;
    lv_obj_t*           obj_to_change;
    void*               extr_data; //Widget_specific_data
} WidgetData;

typedef struct
{
    lv_chart_series_t* series;
    uint32_t timePeriod; //In Seconds
    uint64_t lastPointTime;
    int16_t y_Range;
    lv_coord_t lastMax;
} Chart_Widget_Data;

void init_WidgetHandler();
void vario_add_widget(WidgetData* widget, lv_obj_t *screen);
void vario_update_widget(WidgetData* widget);

#endif
