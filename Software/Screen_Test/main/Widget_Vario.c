#include "Widget_Vario.h"
#include "DeviceContext.h"
#include <stdio.h>

#define BUFFER_TEXT_SIZE 20

LV_IMG_DECLARE(Kompass);
LV_IMG_DECLARE(arrow);

static lv_style_t frame_Widget_Base_Style;
static lv_style_t infoText_Widget_Base_Style;
static lv_style_t value_Widget_Base_Style;
static lv_style_t unit_Widget_Base_Style;

void init_WidgetHandler()
{
    lv_style_init(&frame_Widget_Base_Style);
    lv_style_set_radius(&frame_Widget_Base_Style, LV_STATE_DEFAULT, 8);
    lv_style_set_border_color(&frame_Widget_Base_Style, LV_STATE_DEFAULT, COLOR_BLACK);

    lv_style_init(&infoText_Widget_Base_Style);
    lv_style_set_text_color(&infoText_Widget_Base_Style, LV_STATE_DEFAULT, COLOR_BLACK);
    lv_style_set_text_font(&infoText_Widget_Base_Style, LV_STATE_DEFAULT, &lv_font_montserrat_16);

    lv_style_init(&value_Widget_Base_Style);
    lv_style_set_text_color(&value_Widget_Base_Style, LV_STATE_DEFAULT, COLOR_MAGENTA);
    lv_style_set_text_font(&value_Widget_Base_Style, LV_STATE_DEFAULT, &lv_font_montserrat_16);

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
        return "Vertikal Speed";
    default:
        return "";
    }
}

void get_Value_Text(WidgetContentType contentType, char* const buffer, bool shouldRender)
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

    lv_obj_t* information_text = lv_label_create(frame, NULL);
    lv_obj_add_style(information_text, LV_OBJ_PART_MAIN, &infoText_Widget_Base_Style);
    lv_label_set_text(information_text, get_Info_Text(widget->type));
    lv_obj_align(information_text, NULL, LV_ALIGN_IN_TOP_MID, 0, 2);

    char tempBuffer[10];
    get_Value_Text(widget->type, tempBuffer, true);
    lv_obj_t* value_Text = lv_label_create(frame, NULL);
    lv_obj_add_style(value_Text, LV_OBJ_PART_MAIN, &value_Widget_Base_Style);
    lv_label_set_text(value_Text, tempBuffer);
    lv_obj_align(value_Text, NULL, LV_ALIGN_IN_LEFT_MID, 10, 5);
    widget->obj_to_change = value_Text;

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
    widget->obj_to_change = frame;

    lv_obj_t* img = lv_img_create(frame, NULL);
    lv_img_set_src(img, &Kompass);
    uint16_t zoomFactor = (uint16_t)(((float)((float)widget->w / (float)lv_obj_get_width(img))) * 256);
    lv_img_set_zoom(img, zoomFactor);
    lv_obj_t* imgArrow = lv_img_create(frame, NULL);
    lv_img_set_src(imgArrow, &arrow);
    lv_coord_t yOffset = (lv_obj_get_height(img) * ((float)zoomFactor / (float)256));
    lv_obj_align(imgArrow, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -yOffset + 2);
    yOffset = (lv_obj_get_height(img) * (((float)zoomFactor / (float)256) - 1)) / 2;
    lv_obj_align(img, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -yOffset);

    lv_obj_t* labelText = lv_label_create(frame, NULL);
    lv_obj_add_style(labelText, LV_OBJ_PART_MAIN, &value_Widget_Base_Style);
    lv_obj_align(labelText, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    //lv_img_set_angle(img, 450);
}

static void update_TextBox_Widget(WidgetData* widget)
{
    char tempBuffer[10];
    get_Value_Text(widget->type, tempBuffer, false);
    if (tempBuffer[0] != '\0')
    {
        lv_label_set_text(widget->obj_to_change, tempBuffer);
    }
}

static void update_Compass_Widget(WidgetData* widget)
{
    float heading;
    if (vario_state_get_heading(&heading))
    {
        lv_obj_t* label = lv_obj_get_child(widget->obj_to_change, NULL);
        lv_obj_t* temp = lv_obj_get_child(widget->obj_to_change, label);
        lv_obj_t* img = lv_obj_get_child(widget->obj_to_change, temp);
        char tempBuffer[6];
        snprintf(tempBuffer, 6, "%.0f°", heading);
        lv_label_set_text(label, tempBuffer);
        lv_img_set_angle(img, -heading * 10);
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
    default:
        break;
    }
}
