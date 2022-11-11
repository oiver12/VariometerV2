#include "Widget_Vario.h"
#include "DeviceContext.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

#define BUFFER_TEXT_SIZE 20

#define COMPASS_1METHOD
#define DEGREE_TO_RADIAN 0.01745329251
#define COMPASS_PLACE_SIDE 15
#define CHART_POINT_COUNT 20

LV_IMG_DECLARE(Kompass);
LV_IMG_DECLARE(Kreis_4px);
LV_IMG_DECLARE(arrow);
LV_IMG_DECLARE(Doppelpfeil_Vario)

static lv_style_t frame_Widget_Base_Style;
static lv_style_t infoText_Widget_Base_Style;
static lv_style_t value_Widget_Base_Style;
static lv_style_t unit_Widget_Base_Style;

lv_font_t* __FontStack[] =
{
    &lv_font_montserrat_8,
    &lv_font_montserrat_12,
    &lv_font_montserrat_16,
    &lv_font_montserrat_20,
    &lv_font_montserrat_22,
    &lv_font_montserrat_24,
    &lv_font_montserrat_26,
    &lv_font_montserrat_28,
    &lv_font_montserrat_30,
    &lv_font_montserrat_32,
};

void init_WidgetHandler()
{
    lv_style_init(&frame_Widget_Base_Style);
    lv_style_set_radius(&frame_Widget_Base_Style, LV_STATE_DEFAULT, 8);
    lv_style_set_border_color(&frame_Widget_Base_Style, LV_STATE_DEFAULT, COLOR_BLACK);

    lv_style_init(&infoText_Widget_Base_Style);
    lv_style_set_text_color(&infoText_Widget_Base_Style, LV_STATE_DEFAULT, COLOR_BLACK);
    lv_style_set_text_font(&infoText_Widget_Base_Style, LV_STATE_DEFAULT, &lv_font_montserrat_12);

    lv_style_init(&value_Widget_Base_Style);
    lv_style_set_text_color(&value_Widget_Base_Style, LV_STATE_DEFAULT, COLOR_MAGENTA);
    lv_style_set_text_font(&value_Widget_Base_Style, LV_STATE_DEFAULT, &lv_font_montserrat_32);

    lv_style_init(&unit_Widget_Base_Style);
    lv_style_set_text_color(&unit_Widget_Base_Style, LV_STATE_DEFAULT, COLOR_BLACK);
    lv_style_set_text_font(&unit_Widget_Base_Style, LV_STATE_DEFAULT, &lv_font_montserrat_16);
}

static const char* get_Info_Text(WidgetContentType contentType)
{
    switch (contentType)
    {
    case WidgetContent_Speed_Ground:
        return "Ground Speed";
    case WidgetContent_Vario_Active:
        return "Instant Vario";
    case WidgetContent_Vario_Lazy:
        return "Vario integrated";
    case WidgetContent_VertAcc:
        return "Vertical Acceleration";
    case WidgetContent_Pressure:
        return "Pressure";
    case WidgetContent_Temperature:
        return "Temperature";
    case WidgetContent_AltitudeBaroChart:
        return "Barometer Altitude";
    default:
        return "";
    }
}

float get_Value_Text(WidgetContentType contentType, char* const buffer, bool shouldRender)
{
    float temp;
    bool returnValue = false;
    switch (contentType)
    {
    case WidgetContent_Speed_Ground:
        returnValue = vario_state_get_groundSpeed(&temp);
        break;
    case WidgetContent_Vario_Active:
        returnValue = vario_state_get_vertVelocity(&temp);
        break;
    case WidgetContent_Vario_Lazy:
        returnValue = vario_state_get_vertLazy(&temp);
        break;
    case WidgetContent_VertAcc:
        returnValue = vario_state_get_vertAcc(&temp);
        break;
    case WidgetContent_Pressure:
        returnValue = vario_state_get_pressure(&temp);
        break;
    case WidgetContent_Temperature:
        returnValue = vario_state_get_temperature(&temp);
        break;
    default:
        return;
    }
    if (returnValue | shouldRender)
        snprintf(buffer, BUFFER_TEXT_SIZE, "%.1f", temp);
    else
        buffer[0] = 0;
    return temp;
}

static const char* get_Unit_Text(WidgetContentType contentType)
{
    switch (contentType)
    {
    case WidgetContent_Speed_Ground:
        return "km/h";
    case WidgetContent_Vario_Active:
        return "m/s";
    case WidgetContent_Vario_Lazy:
        return "m/s";
    case WidgetContent_VertAcc:
        return "m/s2";
    case WidgetContent_Pressure:
        return "hPa";
    case WidgetContent_Temperature:
        return "C°";
    default:
        return "";
    }
}

static void add_TextBox_Widget(WidgetData *widget, lv_obj_t *screen)
{
    lv_obj_t* frame = lv_obj_create(screen, NULL);
    lv_obj_add_style(frame, LV_OBJ_PART_MAIN, &frame_Widget_Base_Style);
    lv_obj_align(frame, NULL, widget->alignement, widget->x, widget->y);
    lv_obj_set_size(frame, widget->w, widget->h);
    widget->frame = frame;

    lv_obj_t* information_text = lv_label_create(frame, NULL);
    lv_obj_add_style(information_text, LV_OBJ_PART_MAIN, &infoText_Widget_Base_Style);
    lv_label_set_text(information_text, get_Info_Text(widget->type));
    lv_obj_align(information_text, NULL, LV_ALIGN_IN_TOP_MID, 0, 2);

    TextBox_Widget_Data* extr_data = malloc(sizeof(TextBox_Widget_Data));
    widget->extr_data = (void*)extr_data;

    char tempBuffer[10];
    get_Value_Text(widget->type, tempBuffer, true);
    lv_obj_t* value_Text = lv_label_create(frame, NULL);
    lv_obj_add_style(value_Text, LV_OBJ_PART_MAIN, &value_Widget_Base_Style);
    lv_obj_set_style_local_text_font(value_Text, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, __FontStack[(widget->extraStyle & WS_FONT_MASK_CONTENT) >> 4]);
    lv_label_set_text(value_Text, tempBuffer);
    lv_obj_align(value_Text, NULL, LV_ALIGN_CENTER, -30, 10);
    ((TextBox_Widget_Data*)widget->extr_data)->labelText = value_Text;

    lv_obj_t* unit_Text = lv_label_create(frame, NULL);
    lv_obj_add_style(unit_Text, LV_OBJ_PART_MAIN, &unit_Widget_Base_Style);
    lv_label_set_text(unit_Text, get_Unit_Text(widget->type));
    lv_obj_align(unit_Text, NULL, LV_ALIGN_IN_RIGHT_MID, -5, 11);
}

static void add_Compass_widget(WidgetData* widget, lv_obj_t* screen)
{
    lv_obj_t* frame = lv_obj_create(screen, NULL);
    lv_obj_add_style(frame, LV_OBJ_PART_MAIN, &frame_Widget_Base_Style);
    lv_obj_align(frame, NULL, widget->alignement, widget->x, widget->y);
    lv_obj_set_size(frame, widget->w, widget->h);
    widget->frame = frame;
#ifdef COMPASS_1METHOD
    lv_obj_t* img = lv_img_create(frame, NULL);
    lv_img_set_src(img, &Kompass);
    float width = lv_obj_get_width(img) + COMPASS_PLACE_SIDE;
    uint16_t zoomFactor = (uint16_t)(((float)((float)widget->w / width)) * 256);
    lv_img_set_zoom(img, zoomFactor);
    lv_obj_t* imgArrow = lv_img_create(frame, NULL);
    lv_img_set_src(imgArrow, &arrow);
    lv_coord_t yOffset = (lv_obj_get_height(img) * ((float)zoomFactor / (float)256));
    lv_obj_align(imgArrow, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -yOffset - COMPASS_PLACE_SIDE * 0.6 + 2);
    yOffset = (lv_obj_get_height(img) * (((float)zoomFactor / (float)256) - 1)) / 2;
    lv_obj_align(img, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -yOffset - COMPASS_PLACE_SIDE * 0.6);

    lv_obj_t* labelText = lv_label_create(frame, NULL);
    lv_obj_add_style(labelText, LV_OBJ_PART_MAIN, &value_Widget_Base_Style);
    lv_obj_set_style_local_text_font(labelText, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, __FontStack[(widget->extraStyle & WS_FONT_MASK_CONTENT) >> 4]);
    lv_obj_align(labelText, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 0);
#else
    lv_obj_t* img = lv_img_create(frame, NULL);
    lv_img_set_src(img, &Kreis_4px);
    uint16_t zoomFactor = (uint16_t)(((float)((float)widget->w / ((float)lv_obj_get_width(img) + 10))) * 256);
    lv_img_set_zoom(img, zoomFactor);
    lv_coord_t yOffset = (lv_obj_get_height(img) * (((float)zoomFactor / (float)256) - 1)) / 2;
    lv_obj_align(img, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -yOffset - 5);

    lv_obj_t* imgArrow = lv_img_create(frame, NULL);
    lv_img_set_src(imgArrow, &arrow);
    yOffset = (lv_obj_get_height(img) * ((float)zoomFactor / (float)256));
    lv_obj_align(imgArrow, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -yOffset - 7);

    lv_style_init(&widget->lvgl_styles[0]);
    lv_style_set_text_font(&widget->lvgl_styles[0], LV_CONT_PART_MAIN, __FontStack[(widget->extraStyle & WS_FONT_MASK_CONTENT) >> 4]);
    lv_obj_t* labelText = lv_label_create(frame, NULL);
    lv_obj_add_style(labelText, LV_OBJ_PART_MAIN, &value_Widget_Base_Style);
    lv_obj_add_style(labelText, LV_OBJ_PART_MAIN, &widget->lvgl_styles[0]);
    lv_obj_align(labelText, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    float angle = 0 * DEGREE_TO_RADIAN;
    float widgetWidth = lv_obj_get_height(img) * ((float)zoomFactor / (float)256) / 2 - 17;
    lv_obj_t* label_N = lv_label_create(img, NULL);
    lv_label_set_text(label_N, "N");
    lv_obj_align(label_N, NULL, LV_ALIGN_CENTER, widgetWidth * sin(angle), widgetWidth * -cos(angle)); //PI im Gegenuhrzeigersinn
    lv_obj_t* label_E = lv_label_create(img, NULL);
    lv_label_set_text(label_E, "E");
    lv_obj_align(label_E, NULL, LV_ALIGN_CENTER, widgetWidth * sin(angle+1.5707963), widgetWidth * cos(angle + 1.5707963)); //PI / 2 im Gegenuhrzeigersinn
    lv_obj_t* label_S = lv_label_create(img, NULL);
    lv_label_set_text(label_S, "S");
    lv_obj_align(label_S, NULL, LV_ALIGN_CENTER, widgetWidth * sin(angle), widgetWidth * cos(angle));
    lv_obj_t* label_W = lv_label_create(img, NULL);
    lv_label_set_text(label_W, "W");
    lv_obj_align(label_W, NULL, LV_ALIGN_CENTER, widgetWidth * sin(angle - 1.5707963), widgetWidth * cos(angle - 1.5707963)); //PI / 2 im Uhrzeigersinn
#endif // COMPASS_1METHOD
}

static void add_Chart_Widget(WidgetData* widget, lv_obj_t* screen)
{
    lv_obj_t* frame = lv_obj_create(screen, NULL);
    lv_obj_add_style(frame, LV_OBJ_PART_MAIN, &frame_Widget_Base_Style);
    lv_obj_align(frame, NULL, widget->alignement, widget->x, widget->y);
    lv_obj_set_size(frame, widget->w, widget->h);
    widget->frame = frame;

    Chart_Widget_Data* extr_data = malloc(sizeof(Chart_Widget_Data));
    widget->extr_data = (void*)extr_data;

    lv_obj_t* information_text = lv_label_create(frame, NULL);
    lv_obj_add_style(information_text, LV_OBJ_PART_MAIN, &infoText_Widget_Base_Style);
    lv_label_set_text(information_text, get_Info_Text(widget->type));
    lv_obj_align(information_text, NULL, LV_ALIGN_IN_TOP_MID, 0, 2);

    lv_obj_t* chart = lv_chart_create(frame, NULL);
    lv_obj_set_size(chart, lv_obj_get_width(frame) - 10, lv_obj_get_height(frame) - lv_obj_get_height(information_text) - 20);
    lv_obj_align(chart, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -15);
    lv_obj_set_style_local_size(chart, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 0);

    //lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_series_t* ser1 = lv_chart_add_series(chart, COLOR_BLACK);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_point_count(chart, CHART_POINT_COUNT);
    extr_data->y_Range = 50;
    lv_chart_set_range(chart, 0, extr_data->y_Range);
    extr_data->chart = chart;
    extr_data->lastMax = 50;
    extr_data->series = ser1;
    extr_data->lastPointTime = 0;
    extr_data->timePeriod = 20;
    extr_data->lastMax = 50;

    lv_obj_t* img_arrow = lv_img_create(frame, NULL);
    lv_img_set_src(img_arrow, &Doppelpfeil_Vario);
    float height_Arrow = lv_obj_get_height(img_arrow) + 5;
    uint16_t zoomFactor = (uint16_t)((float)((float)lv_obj_get_height(chart) / height_Arrow) * 256);
    lv_img_set_zoom(img_arrow, zoomFactor);
    lv_obj_align(img_arrow, NULL, LV_ALIGN_IN_LEFT_MID, 0, 2);

    lv_obj_t* height_Text = lv_label_create(frame, NULL);
    lv_obj_add_style(height_Text, LV_OBJ_PART_MAIN, &infoText_Widget_Base_Style);
    lv_label_set_text(height_Text, "50");
    lv_obj_align(height_Text, NULL, LV_ALIGN_IN_LEFT_MID, 15, 0);
}

static void update_TextBox_Widget(WidgetData* widget)
{
    char tempBuffer[10];
    float tempValue = get_Value_Text(widget->type, tempBuffer, false);
    if (tempBuffer[0] != '\0')
    {
        lv_label_set_text(((TextBox_Widget_Data*)widget->extr_data)->labelText, tempBuffer);
        if (widget->type == WidgetContent_Vario_Active || widget->type == WidgetContent_Vario_Lazy)
        {
            if (tempValue >= 0)
                lv_obj_set_style_local_text_color(((TextBox_Widget_Data*)widget->extr_data)->labelText, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, COLOR_GREEN);
            else
                lv_obj_set_style_local_text_color(((TextBox_Widget_Data*)widget->extr_data)->labelText, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, COLOR_MAGENTA);
        }
    }
}

static void update_Compass_Widget(WidgetData* widget)
{
#ifdef COMPASS_1METHOD
    float heading;
    if (vario_state_get_heading(&heading))
    {
        lv_obj_t* label = lv_obj_get_child(widget->frame, NULL);
        lv_obj_t* temp = lv_obj_get_child(widget->frame, label);
        lv_obj_t* img = lv_obj_get_child(widget->frame, temp);
        char tempBuffer[9];
        if (heading > 337 || heading < 23)
            snprintf(tempBuffer, 9, "%.0f° N", heading);
        else if (heading >= 23 && heading <= 67)
            snprintf(tempBuffer, 9, "%.0f° NE", heading);
        else if (heading > 67 && heading < 113)
            snprintf(tempBuffer, 9, "%.0f° E", heading);
        else if (heading >= 113 && heading < 157)
            snprintf(tempBuffer, 9, "%.0f° SE", heading);
        else if (heading >= 157 && heading <= 201)
            snprintf(tempBuffer, 9, "%.0f° S", heading);
        else if (heading > 201 && heading < 247)
            snprintf(tempBuffer, 9, "%.0f° SW", heading);
        else if (heading >= 247 && heading < 293)
            snprintf(tempBuffer, 9, "%.0f° W", heading);
        else
            snprintf(tempBuffer, 9, "%.0f° NW", heading);
        lv_label_set_text(label, tempBuffer);
        lv_img_set_angle(img, -heading * 10);
    }
#else
    float heading;
    if (vario_state_get_heading(&heading))
    {
        lv_obj_t* label = lv_obj_get_child(widget->obj_to_change, NULL);
        char tempBuffer[6];
        snprintf(tempBuffer, 6, "%.0f°", heading);
        lv_label_set_text(label, tempBuffer);
        lv_obj_t* arrow_img = lv_obj_get_child(widget->obj_to_change, label);
        lv_obj_t* circle_img = lv_obj_get_child(widget->obj_to_change, arrow_img);
        lv_obj_t* label_W = lv_obj_get_child(circle_img, NULL);
        lv_obj_t* label_S = lv_obj_get_child(circle_img, label_W);
        lv_obj_t* label_E = lv_obj_get_child(circle_img, label_S);
        lv_obj_t* label_N = lv_obj_get_child(circle_img, label_E);
        lv_obj_invalidate(circle_img);

        float angle = heading * DEGREE_TO_RADIAN;
        float widgetWidth = lv_obj_get_height(circle_img) * ((float)lv_img_get_zoom(circle_img) / (float)256) / 2 - 17;
        lv_obj_align(label_N, NULL, LV_ALIGN_CENTER, widgetWidth * sin(angle + 3.141592654), widgetWidth * cos(angle + 3.141592654)); //PI im Gegenuhrzeigersinn
        lv_obj_align(label_E, NULL, LV_ALIGN_CENTER, widgetWidth * sin(angle + 1.5707963), widgetWidth * cos(angle + 1.5707963)); //PI / 2 im Gegenuhrzeigersinn
        lv_obj_align(label_S, NULL, LV_ALIGN_CENTER, widgetWidth * sin(angle), widgetWidth * cos(angle));
        lv_obj_align(label_W, NULL, LV_ALIGN_CENTER, widgetWidth * sin(angle - 1.5707963), widgetWidth * cos(angle - 1.5707963)); //PI / 2 im Uhrzeigersinn
    }
#endif // COMPASS_1METHOD
}

static void update_Chart_Widget(WidgetData* widget)
{
    float t1 = (float)clock() / CLOCKS_PER_SEC; //in seconds
    Chart_Widget_Data* chart_data = (Chart_Widget_Data*)widget->extr_data;
    if (chart_data->timePeriod / CHART_POINT_COUNT <= t1 - chart_data->lastPointTime)
    {
        float height;
        vario_state_get_altitudeBaro(&height, false);
        lv_chart_set_next(chart_data->chart, ((Chart_Widget_Data*)widget->extr_data)->series, height);
        chart_data->lastPointTime = t1;
        if (chart_data->lastMax < (lv_coord_t)height)
        {
            lv_chart_set_range(chart_data->chart, (lv_coord_t)height - chart_data->y_Range, (lv_coord_t)height);
            chart_data->lastMax = (lv_coord_t)height;
        }
        else if (chart_data->lastMax - chart_data->y_Range > (lv_coord_t)height)
        {
            lv_chart_set_range(chart_data->chart, (lv_coord_t)height, (lv_coord_t)height + chart_data->y_Range);
            chart_data->lastMax = (lv_coord_t)height + chart_data->y_Range;
        }
    }
}

void vario_add_widget(WidgetData* widget, lv_obj_t* screen)
{
    switch (widget->style)
    {
    case Widget_Empty:
        break;
    case Widget_TextBox:
        add_TextBox_Widget(widget, screen);
        break;
    case Widget_Compass:
        add_Compass_widget(widget, screen);
        break;
    case Widget_Chart:
        add_Chart_Widget(widget, screen);
    default:
        break;
    }
}

void vario_update_widget(WidgetData* widget)
{
    switch (widget->style)
    {
    case Widget_Empty:
        break;
    case Widget_TextBox:
        update_TextBox_Widget(widget);
        break;
    case Widget_Compass:
        update_Compass_Widget(widget);
        break;
    case Widget_Chart:
        update_Chart_Widget(widget);
    default:
        break;
    }
}
