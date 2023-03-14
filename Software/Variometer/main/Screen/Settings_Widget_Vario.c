#include "Widget_Vario.h"
#include "E_Ink/GUI_Paint.h"
#include <stdio.h>
#include "DeviceContext.h"
#include "Sensor/QMC588L.h"

const unsigned char gImage_RightArrow[30] = { 0xE7, 0xFE, 0xE3, 0xFE, 0xF1, 0xFE, 0xF8, 0xFE, 0xFC, 0x7E, 0xFE, 0x3E, 0xFF, 0x1E, 0xFF, 0x8E, 0xFF, 0x1E, 0xFE, 0x3E, 0xFC, 0x7E, 0xF8, 0xFE, 0xF1, 0xFE, 0xE3, 0xFE, 0xE7, 0xFE };
const unsigned char gImage_LeftArrow[30] = { 0xFF, 0xCE, 0xFF, 0x8E, 0xFF, 0x1E, 0xFE, 0x3E, 0xFC, 0x7E, 0xF8, 0xFE, 0xF1, 0xFE, 0xE3, 0xFE, 0xF1, 0xFE, 0xF8, 0xFE, 0xFC, 0x7E, 0xFE, 0x3E, 0xFF, 0x1E, 0xFF, 0x8E, 0xFF, 0xCE }; 
const unsigned char gImage_Speaker[60] = { 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0xFF, 0xDF, 0x70, 0xFF, 0x9F, 0x30, 0xFE, 0x1C, 0xB0, 0xFC, 0x1E, 0x90, 0x80, 0x12, 0xD0, 0x80, 0x1A, 0x50, 0x80, 0x1B, 0x50, 0x80, 0x1B, 0x50, 0x80, 0x1B, 0x50, 0x80, 0x12, 0x50, 0xFC, 0x16, 0xD0, 0xFE, 0x1C, 0x90, 0xFF, 0x1D, 0xB0, 0xFF, 0x9F, 0x30, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0 };
const unsigned char gImage_ConfigRad[60] = { 0xFF, 0x8F, 0xF0, 0xFF, 0xF, 0xF0, 0xE7, 0xE, 0x70, 0xC0, 0x0, 0x30, 0xC0, 0x0, 0x30, 0xE0, 0x0, 0x70, 0xE0, 0x0, 0x70, 0xE0, 0xF0, 0x70, 0x81, 0xF8, 0x10, 0x1, 0xFC, 0x0, 0x1, 0xFC, 0x0, 0x1, 0xF8, 0x0, 0xE0, 0xF0, 0x70, 0xE0, 0x60, 0x70, 0xE0, 0x0, 0x70, 0xC0, 0x0, 0x30, 0xC0, 0x0, 0x30, 0xE7, 0xE, 0x70, 0xFF, 0xF, 0xF0, 0xFF, 0x8F, 0xF0 };

//static void CalibrationPageButton_Callback(WidgetData* widget);

static void Slider_Get_Value(WidgetContentType contentType ,uint16_t* currValue, uint16_t* maxValue)
{
    switch (contentType)
    {
    case WidgetContent_Settings_VolumeSlider:
        vario_settings_get_member(currValue, vario_settings_soundVolume, true);
        *maxValue = Volume_max_Size;
        break;
    default:
        printf("Slider Content Type not implemented\n");
    }
}

static void add_Settings_PageButton_Widget(WidgetData* widget, bool hovered, Settings_Page_t currPage, bool reRenderRectangle)
{
    //clear old rectangle
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h + 1, WHITE, DOT_PIXEL_3X3, reRenderRectangle ? DRAW_FILL_FULL : DRAW_FILL_EMPTY);
    
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, hovered ? DOT_PIXEL_3X3 : DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    if (widget->type == WidgetContent_Settings_PageButton_Audio)
    {
        Paint_DrawBitMap_Paste(gImage_Speaker, widget->x + 10, widget->y + widget->h / 2 - 10, 20, 20, false);
        Paint_DrawString_EN(widget->x + 35, widget->y + 14, "Audio", &Font16, WHITE, BLACK, TRUE);
    }
    else if (widget->type == WidgetContent_Settings_PageButton_Config)
    {
        Paint_DrawBitMap_Paste(gImage_ConfigRad, widget->x + 10, widget->y + widget->h / 2 - 10, 20, 20, false);
        Paint_DrawString_EN(widget->x + 35, widget->y + 14, "Config", &Font16, WHITE, BLACK, TRUE);
    }
    Paint_DrawBitMap_Paste(gImage_RightArrow, widget->x + widget->w - 30, widget->y + widget->h / 2 - 7, 15, 15, false);
}

static void add_Settings_BackButton_Widget(WidgetData* widget, bool hovered, bool reRenderRectangle)
{
	//clear old rectangle
	Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h + 1, WHITE, DOT_PIXEL_3X3, reRenderRectangle ? DRAW_FILL_FULL : DRAW_FILL_EMPTY);

	Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, hovered ? DOT_PIXEL_3X3 : DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawBitMap_Paste(gImage_LeftArrow, widget->x + 10, widget->y + widget->h / 2 - 7, 15, 15, false);
    Paint_DrawString_EN(widget->x + 35, widget->y + 14, "Back", &Font16, WHITE, BLACK, TRUE);
}

//here selected means, that this field is selected. If its hovered, then it should display a thicker border
static void add_Settings_Slider_Widget(WidgetData* widget, bool hovered, bool selected, bool reRenderRectangle)
{
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h + 1, WHITE, DOT_PIXEL_3X3, reRenderRectangle ? DRAW_FILL_FULL : DRAW_FILL_EMPTY);
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, hovered && !selected ? DOT_PIXEL_3X3 : DOT_PIXEL_1X1, selected ? DRAW_FILL_FULL : DRAW_FILL_EMPTY);
    Paint_DrawString_EN(widget->x + 5, widget->y + 5, "Volume", &Font12, selected ? BLACK : WHITE, selected ? WHITE : BLACK, TRUE);
    uint16_t currValue, maxVolume;
	Slider_Get_Value(widget->type, &currValue, &maxVolume);
	float percentage = (float)currValue / (float)maxVolume;
    if (percentage > 1)
    {
        percentage = 1;
    }
	Paint_DrawRectangle(widget->x + 5, widget->y + 20, widget->x + widget->w - 45, widget->y + 30, selected ? WHITE : BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(widget->x + 5, widget->y + 20, widget->x + (widget->w - 45) * percentage, widget->y + 30, selected ? WHITE : BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    char buffer[50];
    snprintf(buffer, 10, "%i%%", (int)(percentage * 100));
    Paint_DrawString_EN(widget->x + widget->w - 40, widget->y + 18, buffer, &Font16, selected ? BLACK : WHITE, selected ? WHITE : BLACK, true);
}

static void add_Settings_SoundThreshold(WidgetData* widget, bool hovered, bool selected, bool reRenderRectangle)
{
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h + 1, WHITE, DOT_PIXEL_3X3, reRenderRectangle ? DRAW_FILL_FULL : DRAW_FILL_EMPTY);
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, hovered && !selected ? DOT_PIXEL_3X3 : DOT_PIXEL_1X1, selected ? DRAW_FILL_FULL : DRAW_FILL_EMPTY);
    Paint_DrawString_EN(widget->x + 5, widget->y + 5, "Sink Threshold", &Font12, selected ? BLACK : WHITE, selected ? WHITE : BLACK, TRUE);
}

static void add_Settings_CalibrationPageButton_Widget(WidgetData* widget, bool hovered, bool selected, bool reRenderRectangle)
{
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h + 1, WHITE, DOT_PIXEL_3X3, reRenderRectangle ? DRAW_FILL_FULL : DRAW_FILL_EMPTY);
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, hovered && !selected ? DOT_PIXEL_3X3 : DOT_PIXEL_1X1, selected ? DRAW_FILL_FULL : DRAW_FILL_EMPTY);
    switch(widget->type)
    {
        case WidgetContent_Settings_MagnCalibrationPageButton:
            Paint_DrawString_EN(widget->x + 35, widget->y + 14, "Calibrate Magnetometer", &Font16, selected ? BLACK : WHITE, selected ? WHITE : BLACK, TRUE);
            if(selected)
            {
                Paint_DrawString_EN(widget->x + 35, widget->y + 25, "Calibrate Started...", &Font12, BLACK, WHITE, TRUE);
                Paint_DrawString_EN(widget->x + 35, widget->y + 35, "Please move your vario in all Directions", &Font12, BLACK, WHITE, TRUE);
                qmc588l_calculateCalibration();
            }
            break;
        default:
            break;
    }

}

void vario_display_add_settings_widget(WidgetData* widget, Settings_Page_t currPage, bool hovered, bool selected, bool reRenderRectangle)
{
    switch (widget->style)
    {
    case Widget_settings_BackButton:
		add_Settings_BackButton_Widget(widget, hovered, reRenderRectangle);
		break;
    case Widget_Settings_PageButton:
        add_Settings_PageButton_Widget(widget, hovered, currPage, reRenderRectangle);
        break;
    case Widget_Settings_Slider:
        add_Settings_Slider_Widget(widget, hovered, selected, reRenderRectangle);
        break;
    case Widget_Settings_SoundThreshold:
        add_Settings_SoundThreshold(widget, hovered, selected, reRenderRectangle);
        break;
    case Widget_Settings_CalibrationPageButton:
        add_Settings_CalibrationPageButton_Widget(widget, hovered, selected, reRenderRectangle);
        break;
    default:
        printf("Setting Widget not implemented!\n");
    }
}

void vario_display_select_settings_widget(WidgetData* widget, Settings_Page_t currPage, bool hovered, bool selected)
{
    if(!hovered)
        Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h + 1, WHITE, DOT_PIXEL_3X3, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, hovered ? DOT_PIXEL_3X3 : DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
}

void vario_handle_Slider(WidgetData* widget, Button_Direction direction)
{
	uint16_t currValue, maxVolume;
	Slider_Get_Value(widget->type, &currValue, &maxVolume);
    uint16_t amount = maxVolume * 0.05;
	currValue += direction == Button_Direction_up ? amount : -amount;
    if (currValue > maxVolume)
        currValue = maxVolume;
    
    switch (widget->type)
    {
	case WidgetContent_Settings_VolumeSlider:
		vario_settings_set_member(&currValue, vario_settings_soundVolume);
		break;
    default:
        printf("Slider not implemented!\n");
        break;
    }
    
    float percentage = (float)currValue / (float)maxVolume;
    //clear old Field
    Paint_DrawRectangle(widget->x + 5, widget->y + 20, widget->x + widget->w, widget->y + 30, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    
    Paint_DrawRectangle(widget->x + 5, widget->y + 20, widget->x + widget->w - 45, widget->y + 30,WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(widget->x + 5, widget->y + 20, widget->x + (widget->w - 45) * percentage, widget->y + 30, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    char buffer[50];
    snprintf(buffer, 10, "%i%%", (int)(percentage * 100));
    Paint_DrawString_EN(widget->x + widget->w - 40, widget->y + 18, buffer, &Font16, BLACK, WHITE, true);
}

void vario_handle_SoundThreshold(WidgetData* widget, Button_Direction direction)
{
    uint16_t soundVolume;
    vario_settings_get_member(&soundVolume, vario_settings_soundVolume, true);
}

void vario_display_settings_widget_handle_input(WidgetData* widget, Button_Direction direction)
{
	switch (widget->style)
	{
	case Widget_Settings_Slider:
        vario_handle_Slider(widget, direction);
		break;
	case Widget_Settings_SoundThreshold:
        vario_handle_SoundThreshold(widget, direction);
		break;
	default:
		printf("Setting Widget not implemented!\n");
	}
}