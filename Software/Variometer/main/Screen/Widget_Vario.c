#include "Widget_Vario.h"
#include "DeviceContext.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "E_Ink/GUI_Paint.h"

#define BUFFER_TEXT_SIZE 20
#define Compass_Method1

#define COMPASS_1METHOD
#define DEGREE_TO_RADIAN 0.01745329251
#define COMPASS_PLACE_SIDE 15
#define CHART_POINT_COUNT 20

#ifdef Compass_Method1
const unsigned char gImage_Compass[800] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0, 0x0, 0x7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x0, 0x0, 0x0, 0x0, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x0, 0x7F, 0xFF, 0x0, 0xF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x3, 0xFF, 0xFF, 0xE0, 0x3, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x1F, 0xFF, 0xFF, 0xFC, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x7F, 0xFF, 0xFF, 0xFF, 0x0, 0xFF, 0xFF, 0xFF, 0xFE, 0x1, 0xFF, 0xE7, 0x8F, 0xFF, 0xC0, 0x3F, 0xFF, 0xFF, 0xFC, 0x3, 0xFF, 0xF3, 0xDF, 0xFF, 0xE0, 0x1F, 0xFF, 0xFF, 0xF8, 0xF, 0xFF, 0xF5, 0xDF, 0xFF, 0xF8, 0xF, 0xFF, 0xFF, 0xF0, 0x1F, 0xFF, 0xF5, 0xDF, 0xFF, 0xFC, 0x7, 0xFF, 0xFF, 0xE0, 0x3F, 0xFF, 0xF6, 0xDF, 0xFF, 0xFE, 0x3, 0xFF, 0xFF, 0xC0, 0x7F, 0xFF, 0xF7, 0x5F, 0xFF, 0xFF, 0x1, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xF7, 0x9F, 0xFF, 0xFF, 0x81, 0xFF, 0xFF, 0x81, 0xFF, 0xFF, 0xF7, 0x9F, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0x3, 0xFF, 0xFF, 0xE3, 0xDF, 0xFF, 0xFF, 0xE0, 0x7F, 0xFE, 0x7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x3F, 0xFE, 0xF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x3F, 0xFC, 0xF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x1F, 0xFC, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x1F, 0xF8, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF, 0xF8, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF, 0xF0, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7, 0xF0, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x83, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x83, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC1, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC1, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1, 0xC3, 0x8D, 0xC7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x3, 0xE1, 0xC3, 0xDC, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7B, 0xE1, 0xC3, 0xDA, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xE1, 0xC3, 0xEA, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0xE1, 0xC3, 0xEB, 0x5F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7, 0xE1, 0xC3, 0xEB, 0x5F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0xE1, 0xC3, 0xE7, 0x5F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xE1, 0xC3, 0xF7, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7B, 0xE1, 0xC3, 0xF7, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x3, 0xE1, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC1, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC1, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x83, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x83, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xF0, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7, 0xF0, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7, 0xF8, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF, 0xF8, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF, 0xFC, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x1F, 0xFC, 0xF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x1F, 0xFE, 0xF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x3F, 0xFE, 0x7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x3F, 0xFF, 0x3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x7F, 0xFF, 0x81, 0xFF, 0xFF, 0xFC, 0x3F, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xFB, 0xDF, 0xFF, 0xFF, 0x81, 0xFF, 0xFF, 0xC0, 0x7F, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0x1, 0xFF, 0xFF, 0xE0, 0x3F, 0xFF, 0xF8, 0xFF, 0xFF, 0xFE, 0x3, 0xFF, 0xFF, 0xF0, 0x1F, 0xFF, 0xFE, 0x3F, 0xFF, 0xFC, 0x7, 0xFF, 0xFF, 0xF8, 0xF, 0xFF, 0xFF, 0x9F, 0xFF, 0xF8, 0xF, 0xFF, 0xFF, 0xFC, 0x3, 0xFF, 0xFF, 0xDF, 0xFF, 0xE0, 0x1F, 0xFF, 0xFF, 0xFE, 0x1, 0xFF, 0xFB, 0xDF, 0xFF, 0xC0, 0x3F, 0xFF, 0xFF, 0xFF, 0x80, 0x7F, 0xFC, 0x3F, 0xFF, 0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x1F, 0xFF, 0xFF, 0xFC, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x3, 0xFF, 0xFF, 0xE0, 0x3, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x0, 0x7F, 0xFF, 0x0, 0xF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x0, 0x0, 0x0, 0x0, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0, 0x0, 0x7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
#else
const unsigned char gImage_Compass_Zahlen[512] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEB, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xED, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xED, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0x5F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0x5F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0x78, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x3, 0xEF, 0x7D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFB, 0xF6, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xF6, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xF6, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF7, 0xF6, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x7, 0xFA, 0xD7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF7, 0xFA, 0xD7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFA, 0xD7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xF9, 0xD7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFD, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFB, 0xFD, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF3, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
#endif
const unsigned char gImage_Arrow_down[20] = { 0x0, 0x3F, 0x0, 0x3F, 0x0, 0x3F, 0x80, 0x7F, 0x80, 0x7F, 0xC0, 0xFF, 0xC0, 0xFF, 0xE1, 0xFF, 0xE1, 0xFF, 0xF3, 0xFF };

sFONT* __FontStack[] =
{
    &Font12, //8px
    &Font12, //12px
    &Font16,//16px
    &Font20,//20px
    &Font20,//22px
    &Font24,//24px
    &Font24,//26px
    &Font24,//28px
    &Font24,//30px
    &Font24//32px
};

void init_WidgetHandler()
{
   
}

static uint16_t getTextWidth(const char* pString, sFONT* font)
{
    return font->Width * strlen(pString);
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
        return "Vario integrated (2s)";
    case WidgetContent_VertAcc:
        return "Vertical Acceleration";
    case WidgetContent_Pressure:
        return "Pressure";
    case WidgetContent_Temperature:
        return "Temperature";
    case WidgetContent_Altitude_Baro:
        return "Barometer Altitude";
    default:
        return "";
    }
}

float get_Value_Text(WidgetContentType contentType, char* const buffer, size_t bufferSize, bool shouldRender, bool shouldReset)
{
    float temp;
    bool returnValue = false;
    switch (contentType)
    {
    case WidgetContent_Speed_Ground:
        returnValue = vario_state_get_member(&temp, vario_state_groundSpeed, shouldReset);
        break;
    case WidgetContent_Vario_Active:
        returnValue = vario_state_get_member(&temp, vario_state_vertVel, shouldReset);
        break;
    case WidgetContent_Vario_Lazy:
        returnValue = vario_state_get_member(&temp, vario_state_vertVelLazy, shouldReset);
        break;
    case WidgetContent_VertAcc:
        returnValue = vario_state_get_member(&temp, vario_state_vertAcc, shouldReset);
        break;
    case WidgetContent_Pressure:
        returnValue = vario_state_get_member(&temp, vario_state_pressure, shouldReset);
        break;
    case WidgetContent_Temperature:
        returnValue = vario_state_get_member(&temp, vario_state_temperature, shouldReset);
        break;
    case WidgetContent_Altitude_Baro:
        returnValue = vario_state_get_member(&temp, vario_state_altitudeBaro, shouldReset);
        break;
    default:
        return;
    }
    if ((returnValue | shouldRender) && bufferSize != 0)
    {
        //Wenn zwischen -0.1f und 0.0f, setzte auf 0.0f, weil sonst -0.0 angezeigt wird
        if (temp > -0.05f && temp < 0.0f)
            temp = 0.0f;
        snprintf(buffer, bufferSize, "%.1f", temp);
    }
    else if(bufferSize != 0)
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
        return 'C°';
    case WidgetContent_Altitude_Baro:
        return "m";
    default:
        return "";
    }
}

static void add_TextBox_Widget(WidgetData* widget)
{
    //TODO wenn Widget gelöscht wird, muss auch das hiet gelöscht werden
    TextBox_Widget_Data* extr_data = malloc(sizeof(TextBox_Widget_Data));
    widget->extr_data = (void*)extr_data;

    //TODO: Add rounded Corners to Frame
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);

    char* informationString = get_Info_Text(widget->type);
    uint16_t textWidth = getTextWidth(informationString, &Font12);
    //TODO: was tuen, wenn Text grösser ist? (sollte nicht passieren)
    if (widget->w < textWidth)
        printf("Text passt nicht rein!\n");
    //Text ist oben in der Mitte vom Frame
    Paint_DrawString_EN(widget->x + ((widget->w - textWidth) / 2), widget->y + 4, informationString, &Font12, WHITE, BLACK, TRUE);

    char* unitText = get_Unit_Text(widget->type);
    textWidth = getTextWidth(unitText, &Font16);
    //StartX: Abstand rechts vom Fram ist 5 Pixel
    Paint_DrawString_EN(widget->x + (widget->w - textWidth) - 5, widget->y + (widget->h / 2) + 5, unitText, &Font16, WHITE, BLACK, TRUE);

    sFONT* fontValue = __FontStack[(widget->extraStyle & WS_FONT_MASK_CONTENT) >> 4];
    //Draw value Text
    ((TextBox_Widget_Data*)widget->extr_data)->linkerRandUnitText = widget->x + widget->w - textWidth;
    switch (widget->extraStyle & WS_FONT_MASK_CONTENT)
    {
    case WS_FONT_8_CONTENT:
        ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText = widget->y + widget->h / 2 + 7;
        break;
    case WS_FONT_12_CONTENT:
        ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText = widget->y + widget->h / 2 + 7;
        break;
    case WS_FONT_16_CONTENT:
        ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText = widget->y + widget->h / 2 + 5;
        break;
    case WS_FONT_20_CONTENT:
        ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText = widget->y + widget->h / 2 + 3;
        break;
    case WS_FONT_24_CONTENT:
        ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText = widget->y + widget->h / 2;
        break;
    case WS_FONT_30_CONTENT:
        ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText = widget->y + widget->h / 2;
        break;
    case WS_FONT_32_CONTENT:
        ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText = widget->y + widget->h / 2;
        break;
    }
    char tempBuffer[10];
    get_Value_Text(widget->type, tempBuffer, 10, true, true);
    textWidth = getTextWidth(tempBuffer, __FontStack[(widget->extraStyle & WS_FONT_MASK_CONTENT) >> 4]);
    //StartX: Abstand zwischen Unit rechts und Frame links soll gleich gross sein
    Paint_DrawString_EN(widget->x + ((((TextBox_Widget_Data*)widget->extr_data)->linkerRandUnitText - widget->x) / 2) - textWidth / 2, ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText, tempBuffer, fontValue,WHITE, BLACK, TRUE);
}

static void add_Compass_widget(WidgetData* widget)
{
    Compass_Widget_Data* extr_data = malloc(sizeof(Compass_Widget_Data));
    widget->extr_data = (void*)extr_data;
    extr_data->lastAngle = 0;
    extr_data->angleDifferenceToShow = 5;
    //Draw Frame
    //TODO: Add rounded Corners to Frame
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawBitMap_Paste(gImage_Arrow_down, widget->x + widget->w / 2 - 5, widget->y + 28, 10, 10, FALSE);
#ifdef Compass_Method1
    Paint_DrawBitMap_Paste(gImage_Compass, widget->x + widget->w / 2 - 40, widget->y + widget->h - 80 - 10, 80, 80, FALSE);
#else
    Paint_DrawCircle(widget->x + widget->w / 2, widget->y + widget->h - 40 - 8, 40, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
#endif
}

static void add_Chart_Widget(WidgetData* widget)
{
    //Draw frame
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);

    Chart_Widget_Data* extr_data = malloc(sizeof(Chart_Widget_Data));
    widget->extr_data = (void*)extr_data;
    extr_data->data = fifo_create(ChartNumDataPoints, sizeof(Chart_Point));
    extr_data->lastPointTime = 0;
    extr_data->startXPosition = widget->x + widget->w - 10;
    extr_data->timePeriod = 120;
    extr_data->differenzToShow = 200;
    extr_data->heightChart = widget->h - 45;
    extr_data->yStartPosChart = widget->y + 30;
    extr_data->pointsAdded = 0;
    uint32_t chartXWidth = widget->w - 35;
    extr_data->xMoveDelta = chartXWidth / ChartNumDataPoints;
    //Draw Chart
    char* informationString = get_Info_Text(widget->type);
    uint16_t textWidth = getTextWidth(informationString, &Font12);
    //TODO: was tuen, wenn Text grösser ist? (sollte nicht passieren)
    if (widget->w < textWidth)
        printf("Text passt nicht rein!\n");
    //Text ist oben in der Mitte vom Frame
    Paint_DrawString_EN(widget->x + ((widget->w - textWidth) / 2), widget->y + 4, informationString, &Font12, WHITE, BLACK, TRUE);
    Paint_DrawLine(widget->x + 8, widget->y + 25, widget->x + 8, widget->y + widget->h - 10, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    char buffer[6];
    snprintf(buffer, 6, "%i%s", extr_data->differenzToShow, get_Unit_Text(widget->type));
    Paint_DrawString_EN(widget->x + 12, widget->y + widget->h / 2, buffer, &Font12, WHITE, BLACK, TRUE);
    snprintf(buffer, 6, "%is", extr_data->timePeriod);
    Paint_DrawString_EN(widget->x + widget->w - 30, widget->y + widget->h - 14, buffer,&Font12, WHITE, BLACK, TRUE);
}

static void add_sdCardInfo_Widget(WidgetData* widget)
{
    sd_card_state_t sdcard_state;
    device_state_get_member(&sdcard_state, device_state_statusSDCard, TRUE);
    Paint_DrawRectangle(widget->x, widget->y, widget->x + widget->w, widget->y + widget->h, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    char* informationString = "SD Card usage";
    uint16_t textWidth = getTextWidth(informationString, &Font12);
    Paint_DrawString_EN(widget->x + ((widget->w - textWidth) / 2), widget->y + 4, informationString, &Font12, WHITE, BLACK, TRUE);
    if (sdcard_state.status & SD_CARD_STATE_ERROR_INIT)
    {
		char* buffer = "SD Card not mounted";
		textWidth = getTextWidth(buffer, &Font12);
		Paint_DrawString_EN(widget->x + ((widget->w - textWidth) / 2), widget->y + widget->h / 2, buffer, &Font12, WHITE, BLACK, TRUE);
        return;
	}
    uint32_t startPos = widget->x + 10;
    uint32_t widthProgressBar = widget->w - 60;
    Paint_DrawRectangle(startPos, widget->y + 25, startPos + widthProgressBar, widget->y + 45, BLACK, DOT_PIXEL_1X1, FALSE);
    double percentage = 1.0f - ((double)sdcard_state.freeSpace / (double)sdcard_state.maxSpace);
    uint32_t filledPartWidth = percentage * widthProgressBar;
    Paint_DrawRectangle(startPos, widget->y + 25, startPos + filledPartWidth, widget->y + 45, BLACK, DOT_PIXEL_1X1, TRUE);
    char buffer[50];
    snprintf(buffer, 10, "%i%%", (int)(percentage * 100));
    Paint_DrawString_EN(startPos + widthProgressBar + 7, widget->y + 29, buffer, &Font16, WHITE, BLACK, true);
    if (sdcard_state.status & SD_CARD_STATE_FLIGHTLOGGING)
        snprintf(buffer, 50, "Flight Logger started");
    else
        snprintf(buffer, 50, "Flight Logger disabled");
    textWidth = getTextWidth(buffer, &Font12);
    Paint_DrawString_EN(widget->x + widget->w / 2 - textWidth / 2, widget->y + 55, buffer, &Font12, WHITE, BLACK, true);
    if(sdcard_state.status & SD_CARD_STATE_IGC_LOGGING)
        snprintf(buffer, 50, "IGC Logger started");
    else
        snprintf(buffer, 50, "IGC Logger disabled");
    textWidth = getTextWidth(buffer, &Font12);
    Paint_DrawString_EN(widget->x + widget->w / 2 - textWidth / 2, widget->y + 70, buffer, &Font12, WHITE, BLACK, true);
}

static void update_TextBox_Widget(WidgetData* widget)
{
    char tempBuffer[10];
    float tempValue = get_Value_Text(widget->type, tempBuffer, 10, false, true);
    //Update Value Text
    if (tempBuffer[0] != '\0')
    {
        sFONT* font = __FontStack[(widget->extraStyle & WS_FONT_MASK_CONTENT) >> 4];
        uint16_t textWidth = getTextWidth(tempBuffer, font);
        //Clear surface in frame
        Paint_ClearWindows(widget->x + 5, ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText - 2, ((TextBox_Widget_Data*)widget->extr_data)->linkerRandUnitText - 8, widget->y + (widget->h / 2) + 21, WHITE);
        //Paint_DrawRectangle(widget->x + 5, ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText - 2, ((TextBox_Widget_Data*)widget->extr_data)->linkerRandUnitText - 8, widget->y + (widget->h / 2) + 21, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawString_EN(widget->x + ((((TextBox_Widget_Data*)widget->extr_data)->linkerRandUnitText - widget->x) / 2) - textWidth / 2, ((TextBox_Widget_Data*)widget->extr_data)->yStartValueText, tempBuffer, font, WHITE, BLACK, FALSE);
    }
}

static void update_Compass_Widget(WidgetData* widget)
{
    //Compass_Method2: Optimisation: Draw Circle once and rotate only letters in the middle. In the rotated function, only display pixels in circle (corners would "overdraw" circle)
    int heading;
    if (vario_state_get_member(&heading, vario_state_heading, TRUE) && abs(((Compass_Widget_Data*)widget->extr_data)->lastAngle - heading) > ((Compass_Widget_Data*)widget->extr_data)->angleDifferenceToShow)
    {
        ((Compass_Widget_Data*)widget->extr_data)->lastAngle = heading;
#ifdef Compass_Method1
        Paint_DrawBitMap_Paste_Rotated(gImage_Compass, widget->x + widget->w / 2 - 40, widget->y + widget->h - 80 - 10, 80, 80, (int)heading);
#else
        Paint_DrawBitMap_Paste_Rotated_Circle(gImage_Compass_Zahlen, widget->x + widget->w / 2 - 32, widget->y + widget->h - 80, 64, 64, (int)heading, 31);
#endif
        char tempBuffer[9];
        if (heading > 337 || heading < 23)
            snprintf(tempBuffer, 9, "%iN", heading);
        else if (heading >= 23 && heading <= 67)
            snprintf(tempBuffer, 9, "%iNE", heading);
        else if (heading > 67 && heading < 113)
            snprintf(tempBuffer, 9, "%iE", heading);
        else if (heading >= 113 && heading < 157)
            snprintf(tempBuffer, 9, "%iSE", heading);
        else if (heading >= 157 && heading <= 201)
            snprintf(tempBuffer, 9, "%iS", heading);
        else if (heading > 201 && heading < 247)
            snprintf(tempBuffer, 9, "%iSW", heading);
        else if (heading >= 247 && heading < 293)
            snprintf(tempBuffer, 9, "%iW", heading);
        else
            snprintf(tempBuffer, 9, "%iNW", heading);
        uint16_t textWidth = getTextWidth(tempBuffer, &Font16);
        Paint_ClearWindows(widget->x + 10, widget->y + 10, widget->x + (widget->w / 2) + 30, widget->y + 25, WHITE);
        Paint_DrawString_EN(widget->x + (widget->w / 2) - textWidth / 2, widget->y + 10, tempBuffer, &Font16, WHITE, BLACK, FALSE);
    }
}

//TODO: Program Mode, where it auto. zooms to see all Points
static void update_Chart_Widget(WidgetData* widget)
{
    float t1 = (float)clock() / CLOCKS_PER_SEC; //in seconds
    //Update Chart
    Chart_Widget_Data* chart_data = (Chart_Widget_Data*)widget->extr_data;
    //Place a point all x Seconds (e.g. when timePeriod is 20s and Point Count is 20, place all 1s a new point and shift all Point to the left)
    if (chart_data->timePeriod / CHART_POINT_COUNT <= t1 - chart_data->lastPointTime)
    {
        chart_data->lastPointTime = t1;
        if (chart_data->pointsAdded == 0)
        {
            /*float height;
            vario_state_get_member(&height, vario_state_altitudeBaro, FALSE);*/
			float value = get_Value_Text(widget->type, NULL, 0, false, false);
            //Place first Point in the middle
            chart_data->startValue = value - chart_data->differenzToShow / 2;
            float percentageFromHeight = ((float)(value - chart_data->startValue)) / ((float)chart_data->startValue);
            Chart_Point point = {
                .lastXPoint = chart_data->startXPosition,
                .yPoint = (uint16_t)((1 - percentageFromHeight) * (float)chart_data->heightChart) + chart_data->yStartPosChart,
                .heightValue = value,
            };
            fifo_add(chart_data->data, &point);
            Paint_DrawPoint(point.lastXPoint, point.yPoint, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
            chart_data->pointsAdded++;
            return;
        }
        float value = get_Value_Text(widget->type, NULL, 0, false, false);
        bool startValueChanged = false;
        //when new height can't is lower or higher than the range in the chart, shift the range, so the new point is in the chart
        if (value < chart_data->startValue || value > chart_data->startValue + chart_data->differenzToShow)
        {
            chart_data->startValue = value < chart_data->startValue ? value : (value - chart_data->differenzToShow);
            startValueChanged = true;
        }
        float percentageFromHeight = ((float)(value - chart_data->startValue)) / ((float)chart_data->differenzToShow);
        Chart_Point newPoint = {
            .lastXPoint = chart_data->startXPosition,
            .yPoint = (uint16_t)((1-percentageFromHeight) * (float)chart_data->heightChart) + chart_data->yStartPosChart,
            .heightValue = value,
        };
        Chart_Point lastpoint;
        if (chart_data->pointsAdded < CHART_POINT_COUNT)
        {
            chart_data->pointsAdded++;
            if (fifo_is_full(chart_data->data))
                printf("Test scheisse\n");
        }
        else
        {
            fifo_get(chart_data->data, &lastpoint);
            if (lastpoint.yPoint != -1)
                Paint_DrawPoint(lastpoint.lastXPoint, lastpoint.yPoint, WHITE, DOT_PIXEL_2X2, DOT_STYLE_DFT);
        }

        fifo_add(chart_data->data, &newPoint);

        //Paint_ClearWindows(chart_data->startXPosition - chart_data->xMoveDelta * CHART_POINT_COUNT, chart_data->yStartPosChart - 2, chart_data->startXPosition + 2, chart_data->yStartPosChart + chart_data->heightChart + 2, WHITE);
        //Paint_DrawRectangle(chart_data->startXPosition - chart_data->xMoveDelta * CHART_POINT_COUNT , chart_data->yStartPosChart - 2, chart_data->startXPosition + 2, chart_data->yStartPosChart + chart_data->heightChart + 2, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

        //shift all Points left and render (dont shift last one)
		//TODO: there are buggs, where the last point isn't rendered full.
		//TODO: also fifo_peekItem shouldn't make a for loop everytime. State could be remembered in the fifo and just call getNext() on fifo
        for (size_t i = 0; i < chart_data->pointsAdded - 1; i++)
        {
            Chart_Point* itemPoint = fifo_peekItem(chart_data->data, i);
			//Clear old Point
            if(itemPoint->yPoint != -1)
				Paint_DrawPoint(itemPoint->lastXPoint, itemPoint->yPoint, WHITE, DOT_PIXEL_2X2, DOT_STYLE_DFT);
            if (startValueChanged)
            {
                float percentageFromHeight = ((float)(itemPoint->heightValue - chart_data->startValue)) / ((float)chart_data->differenzToShow);
                if (percentageFromHeight > 1 || percentageFromHeight < 0)
                    itemPoint->yPoint = -1;
                else
                    itemPoint->yPoint = (uint16_t)((1 - percentageFromHeight) * (float)chart_data->heightChart) + chart_data->yStartPosChart;
            }
            itemPoint->lastXPoint -= chart_data->xMoveDelta;
            if(itemPoint->yPoint != -1)
                Paint_DrawPoint(itemPoint->lastXPoint, itemPoint->yPoint, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
        }
        Paint_DrawPoint(newPoint.lastXPoint, newPoint.yPoint, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    }
}

void update_sdCardInfo_Widget(WidgetData* widget)
{
    sd_card_state_t sdcard_state;
    if (device_state_get_member(&sdcard_state, device_state_statusSDCard, true))
    {
        if (sdcard_state.status & SD_CARD_STATE_ERROR_INIT)
        {
            char* buffer = "SD Card not mounted";
            uint16_t textWidth = getTextWidth(buffer, &Font12);
            Paint_DrawString_EN(widget->x + ((widget->w - textWidth) / 2), widget->y + widget->h / 2, buffer, &Font12, WHITE, BLACK, TRUE);
            return;
        }
        uint32_t startPos = widget->x + 10;
        uint32_t widthProgressBar = widget->w - 60;
        Paint_ClearWindows(widget->x + 5, widget->y + 25, widget->x + widget->w - 5, widget->y + widget->h - 5, WHITE);
        Paint_DrawRectangle(startPos, widget->y + 25, startPos + widthProgressBar, widget->y + 45, BLACK, DOT_PIXEL_1X1, FALSE);
        double percentage = 1.0f - ((double)sdcard_state.freeSpace / (double)sdcard_state.maxSpace);
        uint32_t filledPartWidth = percentage * widthProgressBar;
        Paint_DrawRectangle(startPos, widget->y + 25, startPos + filledPartWidth, widget->y + 45, BLACK, DOT_PIXEL_1X1, TRUE);
        char buffer[50];
        snprintf(buffer, 10, "%i%%", (int)(percentage * 100));
        Paint_DrawString_EN(startPos + widthProgressBar + 7, widget->y + 29, buffer, &Font16, WHITE, BLACK, true);
        if (sdcard_state.status & SD_CARD_STATE_FLIGHTLOGGING)
            snprintf(buffer, 50, "Flight Logger started");
        else
            snprintf(buffer, 50, "Flight Logger disabled");
        uint16_t textWidth = getTextWidth(buffer, &Font12);
        Paint_DrawString_EN(widget->x + widget->w / 2 - textWidth / 2, widget->y + 55, buffer, &Font12, WHITE, BLACK, true);
        if (sdcard_state.status & SD_CARD_STATE_IGC_LOGGING)
            snprintf(buffer, 50, "IGC Logger started");
        else
            snprintf(buffer, 50, "IGC Logger disabled");
        textWidth = getTextWidth(buffer, &Font12);
        Paint_DrawString_EN(widget->x + widget->w / 2 - textWidth / 2, widget->y + 70, buffer, &Font12, WHITE, BLACK, true);
    }
}

void vario_display_add_widget(WidgetData* widget)
{
    switch (widget->style)
    {
    case Widget_Empty:
        break;
    case Widget_TextBox:
        add_TextBox_Widget(widget);
        break;
    case Widget_Compass:
        add_Compass_widget(widget);
        break;
    case Widget_Chart:
        add_Chart_Widget(widget);
        break;
    case Widget_SdCardInfo:
        add_sdCardInfo_Widget(widget);
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
    case Widget_Chart:
        update_Chart_Widget(widget);
        break;
    case Widget_SdCardInfo:
        update_sdCardInfo_Widget(widget);
        break;
    default:
        break;
    }
}
