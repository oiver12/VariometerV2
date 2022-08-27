#include "Widget.h"
#include "fonts.h"
#include "GUI_Paint.h"

static vario_state_t* vario_state; 
SemaphoreHandle_t vario_state_Mutex;
char tempString[100];

sFONT* __FontStack[] = 
{
	&Font8,
    &Font12,
    &Font16,
    &Font20,
    &Font24
};

uint8_t fontHeights[] = 
{
    8, 
    12,
    16,
    20,
    24
};

const char* getLabel(WidgetData* widget)
{
    switch (widget->type)
    {
        case WidgetContent_Vario_Active:
            return "Vario";
        default:
            return "";
    }
}

const char* getContentString(WidgetData* widget)
{
    float temp = 0;
    switch (widget->type)
    {
        case WidgetContent_Vario_Active:
            xSemaphoreTake(vario_state_Mutex, portMAX_DELAY);
            {
                temp = vario_state->velocity;
            }
            xSemaphoreGive(vario_state_Mutex);
            snprintf(tempString, 100, "%.2f", temp);
            return tempString;
        default:
            return "";
    }
}

const char* getUnit(WidgetData* widget)
{
    switch (widget->type)
    {
        case WidgetContent_Vario_Active:
            return "m/s";
        default:
            return "";
    }
}

void draw_Border(WidgetData* widget)
{
    if (widget->extraStyle & WS_BORDER_TOP)
		Paint_DrawNormalHLine(widget->x, widget->y, widget->w);
	if (widget->extraStyle & WS_BORDER_BOTTOM)
		Paint_DrawNormalHLine(widget->x, widget->y + widget->h - 1, widget->w);
	if (widget->extraStyle & WS_BORDER_LEFT)
		Paint_DrawNormalVLine(widget->x, widget->y, widget->h);
	if (widget->extraStyle & WS_BORDER_RIGHT)
		Paint_DrawNormalVLine(widget->x + widget->w - 1, widget->y, widget->h);
}

void draw_Text(const char * str, int16_t x, int16_t y, sFONT* font)
{
    Paint_DrawString_EN(x, y, str, font, WHITE, BLACK);
}

void draw_Empty_Widget(WidgetData* widget)
{
    draw_Border(widget);
}

void draw_TextBox_Widget(WidgetData* widget)
{
    //TODO: nur Content neu zeichenen. Den innen Teil vom Rechteck ausfüllen mit weiss davor.
    draw_Border(widget);
    draw_Text(getLabel(widget), widget->x + 2, widget->y + 2, __FontStack[widget->extraStyle & WS_FONT_MASK_LABEL]);
    draw_Text(getUnit(widget), widget->x + widget->w - 30, widget->y + 2, __FontStack[widget->extraStyle & WS_FONT_MASK_LABEL]);
    draw_Text(getContentString(widget), widget->x + 2, widget->y + widget->h / 2, __FontStack[(widget->extraStyle & WS_FONT_MASK_CONTENT) >> 4]);
}

void draw_Widget(WidgetData* widget)
{
    switch(widget->style)
    {
        case Widget_Empty:
            draw_Empty_Widget(widget);
            break;
        case Widget_TextBox:
            draw_TextBox_Widget(widget);
        default:
            break;
    }
}

void init_Widgets_Handler(vario_state_t* state, SemaphoreHandle_t Mutex)
{
    vario_state = state;
    vario_state_Mutex = Mutex;
}