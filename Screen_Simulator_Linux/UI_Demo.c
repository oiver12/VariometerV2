#include <stdio.h>
#include "Screen/DeviceContext.h"
#include "Screen/ScreenManager.h"
#include "Screen/E_Ink/GUI_Paint.h"
#include <time.h>
#include <math.h>
#include "Screen/DeviceContext.h"
#include "UI_Demo.h"
#include <math.h>

float startTimeDown = 0.0f;
uint8_t startedDown = FALSE;
uint8_t startedLeft = FALSE;
uint8_t startedRight = FALSE;
uint8_t startedEnter = FALSE;

static const int secondsPerPx = 1;  //arc seconds per pixel (3 equals cca 90m)
static const int totalPx = 3601;
extern void airspaceParcer();

static void readPixel(FILE* file, int y, int x, int* height)
{
    int row = (totalPx - 1) - y;
    int col = x;
    int pos = (row * totalPx + col) * 2;
    
    uint8_t temp[2];
    fseek(file, pos, SEEK_SET);
    fread(temp, sizeof(uint8_t), 2, file);
	*height = 0 | (temp[0] << 8) | temp[1];
}

void readAGL(double latitude, double longitude)
{
    //float height = srtmGetElevation(latitude, longitude);
    
	int latDec = (int)floor(latitude);
	int lonDec = (int)floor(longitude);
    char fileName[50];
    sprintf(fileName, "%s/%c%02d%c%03d.hgt", "AGL",
        latitude > 0 ? 'N' : 'S', abs(latDec),
        longitude > 0 ? 'E' : 'W', abs(lonDec));
    
    FILE* fileptr;

    fileptr = fopen(fileName, "r"); 


    float secondsLat = (latitude - latDec) * 60 * 60;
    float secondsLon = (longitude - lonDec) * 60 * 60;
    int y = secondsLat / secondsPerPx;
	int x = secondsLon / secondsPerPx;
    
    int height[4];
    readPixel(fileptr, y, x, &height[2]);
    readPixel(fileptr, y + 1, x, &height[0]);
    readPixel(fileptr, y, x + 1, &height[3]);
    readPixel(fileptr, y + 1, x + 1, &height[1]);

    //ratio where X lays
    //float dy = fmod(secondsLat, secondsPerPx) / secondsPerPx;
    //float dx = fmod(secondsLon, secondsPerPx) / secondsPerPx;

    // Bilinear interpolation
    // h0------------h1
    // |
    // |--dx-- .
    // |       |
    // |      dy
    // |       |
    // h2------------h3   
     
    // float rightHeight = height[0] * dy * (1 - dx) +
    //     height[1] * dy * (dx)+
    //     height[2] * (1 - dy) * (1 - dx) +
    //     height[3] * (1 - dy) * dx;
    /*double lat = latitude - (int)latitude;
    double lon = longitude - (int)longitude;
    
    double latitudeSeconds = lat * 3600;
	double longitudeSeconds = lon * 3600;

	double foundLat = ((double)round(latitudeSeconds / 3) * 3.0) / 3600.0;
    double foundLong = ((double)round(longitudeSeconds / 3) * 3.0) / 3600.0;

    int i = 1201 - round(latitudeSeconds / 3);
	int j = round(longitudeSeconds / 3);

    uint8_t temp[2];
    
    fseek(fileptr, ((i - 1) * 1201 + (j - 1)) * 2, SEEK_SET);
    fread(temp, sizeof(uint8_t), 2, fileptr);

    uint16_t alt122 = (temp[0] << 8) + temp[1];*/
}

void startRender()
{

    //airspaceParcer();
    gps_t gsp;
    vario_state_get_member(&gsp, vario_state_gps_data, false);
    gsp.tim.hour = 12;
    gsp.tim.minute = 40;
    vario_state_set_member(&gsp, vario_state_gps_data);
    //readAGL(46.840812059784795, 8.41060546239144);
    static nmea_parser_handle_t parser_hdl;
    init_DeviceContext(parser_hdl);

    sd_card_state_t sdcard_state = {
        .status = SD_CARD_STATE_INIT | SD_CARD_STATE_IGC_LOGGING,
        .maxSpace = 4294967296,
        .freeSpace = 2147483648
    };
    device_state_set_member(&sdcard_state, device_state_statusSDCard);

    float pressure = 101325;
    vario_state_set_member(&pressure, vario_state_pressure);

    uint16_t volume = 10000;
    vario_settings_set_member(&volume, vario_settings_soundVolume);

    static SemaphoreHandle_t semphr;

    ScreenManager_init(semphr);
    
    static WidgetData widgetDataScreen0[8] = {
        {.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Vario_Active, .x = 0, .y = 27},
        {.extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 100, .style = Widget_Compass, .type = WidgetContent_CompassSensor, .x = 0, .y = 87},
        {.extraStyle = WS_FONT_20_CONTENT, .h = 193, .w = 220, .style = Widget_Chart, .type = WidgetContent_Altitude_Baro, .x = 100, .y = 87},
        {.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_VertAcc, .x = 160, .y = 27},
        {.extraStyle = WS_FONT_16_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Pressure, .x = 320, .y = 27},
        {.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Altitude_GPS, .x = 320, .y = 87},
		{.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Speed_Ground, .x = 320, .y = 147},
        {.extraStyle = WS_FONT_24_CONTENT, .h = 73, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Vario_Lazy, .x = 320, .y = 207},
    };
    ScreenManager_add_Screen_add_widgets(widgetDataScreen0, 8);
    static WidgetData widgetDataScreen1[5] = {
        {.extraStyle = WS_FONT_32_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Time_GPS, .x = 0, .y = 27},
        {.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Altitude_AGL, .x = 160, .y = 27},
		{.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Speed_Ground, .x = 320, .y = 27},
        {.extraStyle = WS_FONT_32_CONTENT, .h = 100, .w = 200, .style = Widget_SdCardInfo, .type = WidgetContent_SDCardInfo, .x = 280, .y = 87},
        {.extraStyle = WS_FONT_12_CONTENT, .h = 192, .w = 280, .style = Widget_GPSInfo, .type = WidgetContent_GPSInfo, .x = 0, .y = 87}
    };
    ScreenManager_add_Screen_add_widgets(widgetDataScreen1, 5);
    ScreenManager_show_Screen(1);
    FILE* fileptr;
	fileptr = fopen("IGC/2023-05-06-XCT-TLE-09.IGC", "r");
    if(fileptr == NULL)
    {
        printf("Can't finde File\n");
        return;
    }
    //vario_display_FlightStatistics(fileptr, "IGC_Flight");
    fclose(fileptr);
}

void updateRender()
{
    ScreenManager_update();
}

void startRenderTimeProgress()
{
    Paint_DrawLine(0, 281, 480, 281, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    const int x = 20;
    const int y = 290;
    const int width = 440; 
    const int height = 20;
    Paint_DrawRectangle(x, y, x + width, y + height, BLACK, DOT_PIXEL_1X1, FALSE);
    Paint_DrawRectangle(x + 2, y + 2, x + width - 2, y + height - 2, WHITE, DOT_PIXEL_1X1, WHITE);
}

void updateRenderTimeProgress(float percentage, size_t vario_state_Simulation_size, vario_state_Simulation* bufferSimulation)
{
    static int i = 0;
    if (i % 10 == 0)
    {
        startRenderTimeProgress();
        startHeightDiagramm(vario_state_Simulation_size, bufferSimulation);
    }
    const int x = 20;
    const int y = 290;
    const int width = 440; 
    const int height = 20;
    Paint_DrawRectangle(x + 2, y + 2, x + width - 2, y + height - 2, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(x + 2, y + 2, x + width * percentage - 2, y + height - 2, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
}

void startHeightDiagramm(size_t vario_state_Simulation_size, vario_state_Simulation* bufferSimulation)
{
    const int x = 10;
    const int y = 320;
    const int width = 460;
    const int height = 80;
    Paint_DrawRectangle(x, y, x + width, y + height, BLACK, DOT_PIXEL_1X1, FALSE);
    float ratio = (float)vario_state_Simulation_size / (float)width;
    float i = 0;
    int pixelDraw = 0;
    float maxHeight = 0;
    float minHeight = 0;
    for (size_t i = 0; i < vario_state_Simulation_size; i++)
    {
        float height = bufferSimulation[i].height;
        if (height > maxHeight)
            maxHeight = height;
        else if (height < minHeight)
            minHeight = height;
    }

    maxHeight += 50;
    minHeight -= 50;
    while(i < vario_state_Simulation_size && pixelDraw <= width)
    {
        float percentageFromHeight = ((float)(bufferSimulation[(int)roundf(i)].height - minHeight)) / (float)(maxHeight - minHeight);
        float heightY = (uint16_t)((1 - percentageFromHeight) * (float)height) + y;
        Paint_DrawPoint(x + pixelDraw, heightY, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);
        pixelDraw++;
        i += ratio;
    }
}

void startDownArrow()
{
    if (!startedLeft)
        startedLeft = TRUE;
}

void DownArrowReleased()
{
    if (startedLeft)
    {
        ScreenManager_DownArrowPressed();
        startedLeft = FALSE;
    }
}

void startUpArrow()
{
    if (!startedRight)
        startedRight = TRUE;
}

void UpArrowReleased()
{
    if (startedRight)
    {
        ScreenManager_UpArrowPressed();
        startedRight = FALSE;
    }
}

void startEnterKey()
{
    if (!startedEnter)
        startedEnter = TRUE;
}

void EnterKeyReleased()
{
    if (startedEnter)
    {
        ScreenManager_EnterKeyPressed();
        startedEnter = FALSE;
    }
}

void startMenuKey()
{
    if (!startedDown)
    {
        startTimeDown = (float)clock() / CLOCKS_PER_SEC; //in seconds
        startedDown = TRUE;
    }
}

void MenuKeyReleased()
{
    if (startedDown)
    {
        float difference = ((float)clock() / CLOCKS_PER_SEC) - startTimeDown;
        startedDown = FALSE;
        ScreenManager_powerButtonPressed(difference > 0.5f);
    }
}

void PKeyPressed()
{
	gps_time_t time = {
		.hour = 12,
		.minute = 34,
		.second = 56
	};
	gps_t gps = {
		.altitude = 1000,
		.latitude = 46.840812059784795,
		.longitude = 8.41060546239144,
		.speed = 10,
		.tim = time,
		.valid = 1
	};
	vario_state_set_member(&gps, vario_state_gps_data);
    PopUpConfig_t config = {
        .x = 150,
        .y = 100,
        .w = 100,
        .h = 60,
        .type = PopUpType_TakeOff,
        .timeToDisplay = 3000,
    };
    ScreenManager_showPopUp(config);
}