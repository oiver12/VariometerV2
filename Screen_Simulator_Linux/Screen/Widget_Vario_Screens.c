#include "Widget_Vario.h"
#include "E_Ink/GUI_Paint.h"
#include "assets/assets.h"

void vario_display_PowerOffScreen()
{
    Paint_Clear(WHITE);
    Paint_DrawBitMap_Paste(gImage_PowerOff, Paint.Width / 2 - 50, 50, 100, 100, FALSE);
    const char* stringToShow = "Confirm Power Off";
    uint32_t stringLength = strlen(stringToShow) * Font24.Width;
    Paint_DrawString_EN(Paint.Width / 2 - stringLength / 2, 175, stringToShow, &Font24, WHITE, BLACK, FALSE);
    const char* nextString = "Press the power button again to turn off the device";
    const char* nextString1 = "Press any other button to cancel";
    stringLength = strlen(nextString) * Font12.Width;
    Paint_DrawString_EN(Paint.Width / 2 - stringLength / 2, 220, nextString, &Font12, WHITE, BLACK, FALSE);
    stringLength = strlen(nextString1) * Font12.Width;
    Paint_DrawString_EN(Paint.Width / 2 - stringLength / 2, 240, nextString1, &Font12, WHITE, BLACK, FALSE);
}