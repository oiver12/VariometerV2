#include "Widget_Vario.h"
#include "E_Ink/GUI_Paint.h"
#include "DeviceContext.h"
#include <stdio.h>
#include "esp_timer.h"
#include "ScreenManager.h"

extern unsigned char gImage_gps_an[78];

static void popUp_Disable_TimerCallback(void* arg)
{
    ScreenManager_closePopUp();
}

static void popUpGPS_draw(PopUpConfig_t configData)
{
	Paint_DrawBitMap_Paste(gImage_gps_an, configData.x + 5, configData.y + 5, 24, 24, false);
	Paint_DrawString_EN(configData.x + 35, configData.y + 15, "GPS FIX", &Font12, WHITE, BLACK, false);
	gps_t gpsData;
	vario_state_get_member(&gpsData, vario_state_gps_data, false);
	char buffer[25];
	snprintf(buffer, 25, "Lat: %.2f", gpsData.longitude);
	Paint_DrawString_EN(configData.x + 10, configData.y + 35, buffer, &Font12, WHITE, BLACK, false);
	snprintf(buffer, 25, "Lon: %.2f", gpsData.latitude);
	Paint_DrawString_EN(configData.x + 10, configData.y + 55, buffer, &Font12, WHITE, BLACK, false);
}

static void popUpBattery_draw(PopUpConfig_t configData)
{
	
}

static void popUpSettings_draw(PopUpConfig_t configData)
{

}

static void popUpTakeOff_draw(PopUpConfig_t configData)
{
	Paint_DrawString_EN(configData.x + 5, configData.y + 10, "Take Off", &Font16, WHITE, BLACK, false);
	gps_t gpsData;
	vario_state_get_member(&gpsData, vario_state_gps_data, false);
	char buffer[25];
	snprintf(buffer, 25, "%i:%i", gpsData.tim.hour, gpsData.tim.minute);
	Paint_DrawString_EN(configData.x + 30, configData.y + 35, buffer, &Font12, WHITE, BLACK, false);
}

static void popUpLanding_draw(PopUpConfig_t configData)
{
	
}

void vario_display_showPopUp(PopUpConfig_t configData)
{
	Paint_DrawRectangle(configData.x, configData.y, configData.x + configData.w, configData.y + configData.h, WHITE, DOT_PIXEL_2X2, DRAW_FILL_FULL);
	Paint_DrawRectangle(configData.x, configData.y, configData.x + configData.w, configData.y + configData.h, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
	switch (configData.type)
	{
	case PopUpType_GPSFix:
		popUpGPS_draw(configData);
		break;
	case PopUpType_BatteryWarning:
		popUpBattery_draw(configData);
		break;
	case PopUpType_WidgetSettings:
		popUpSettings_draw(configData);
		break;
    case PopUpType_TakeOff:
		popUpTakeOff_draw(configData);
		break;
	case PopUpType_Landing:
		popUpLanding_draw(configData);
		break;
	default:
		break;
	}
	if (configData.timeToDisplay > 0)
	{
        const esp_timer_create_args_t oneshot_timer_args = {
            .callback = &popUp_Disable_TimerCallback,
            /* argument specified here will be passed to timer callback function */
            .name = "popUp Disable"
        };
        esp_timer_handle_t oneshot_timer;
        ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer));
        ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, configData.timeToDisplay * 1000));
	}
}