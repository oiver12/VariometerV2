#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "Screen/E_Ink/GUI_Paint.h"
#include "UI_Demo.h"
#include "Screen/DeviceContext.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "Screen/IGC_Parser.h"
#include "airspaceParcer.h"
#include "Simulation.h"

#define EPD_3IN7_WIDTH 410
#define EPD_3IN7_HEIGHT 480

Uint32 lastRenderTime = 0;
Uint32 startedFKeyTime = 0;
bool startedFKey = false;

UBYTE *ScreenBuffer;

void Get_duration_diffGPSTime(gps_time_t start_time, gps_time_t end_time, gps_time_t *duration)
{
    uint64_t total_start_seconds = start_time.hour * 60 * 60 + start_time.minute * 60 + start_time.second;
    uint64_t total_end_seconds = end_time.hour * 60 * 60 + end_time.minute * 60 + end_time.second;
    uint64_t total_diff_minutes = total_end_seconds - total_start_seconds;

    duration->hour = total_diff_minutes / 3600;
    duration->minute = (total_diff_minutes / 60) % 60;
    duration->second = (total_diff_minutes % 60);
}

void AnalyzeGPS()
{
    FILE *f = fopen("Trackloggs/GPSPassivThin.txt", "r");
    if (!f)
    {
        printf("Couldn't open GPS Log File\n");
        return;
    }
    fseek(f, 0, SEEK_END);
    size_t bufferSize = ftell(f);
    rewind(f);
    uint8_t bufferRead[bufferSize];
    fread(bufferRead, bufferSize, 1, f);
    fclose(f);
    size_t currPos = 0;
    int currPosGPSObj = 0;
    size_t gpsSize = bufferSize / sizeof(gps_t);
    gps_t gps_Objs[bufferSize];
    bool firstValid = false;
    bool firstGPS = false;
    bool firstDGPS = false;
    bool first2D = false;
    bool first3D = false;
    gps_analyze_t gpsAnalyze;
    memset(&gpsAnalyze, 0, sizeof(gps_analyze_t));
    FILE *pythonArrayF = fopen("pythonArray", "w");
    fprintf(pythonArrayF, "y=[");
    while (currPosGPSObj < gpsSize)
    {
        int64_t *time = (int64_t *)&bufferRead[currPos];
        currPos += sizeof(int64_t);
        gps_t data;
        data.tim.hour = bufferRead[currPos];
        currPos++;
        data.tim.minute = bufferRead[currPos];
        currPos++;
        data.tim.second = bufferRead[currPos];
        currPos++;
        data.tim.thousand = bufferRead[currPos];
        currPos++;
        data.fix = *((gps_fix_t *)&bufferRead[currPos]);
        currPos += sizeof(gps_fix_t);
        data.fix_mode = *((gps_fix_mode_t *)&bufferRead[currPos]);
        currPos += sizeof(gps_fix_mode_t);
        data.valid = *((bool *)&bufferRead[currPos]);
        currPos += sizeof(bool);
        data.dop_h = *((float *)&bufferRead[currPos]);
        currPos += sizeof(float);
        data.dop_p = *((float *)&bufferRead[currPos]);
        currPos += sizeof(float);
        data.dop_v = *((float *)&bufferRead[currPos]);
        currPos += sizeof(float);
        data.sats_in_use = *((uint8_t *)&bufferRead[currPos]);
        currPos++;
        data.sats_in_view = *((uint8_t *)&bufferRead[currPos]);
        currPos++;
        data.latitude = *((float *)&bufferRead[currPos]);
        currPos += sizeof(float);
        data.longitude = *((float *)&bufferRead[currPos]);
        currPos += sizeof(float);
        data.altitude = *((float *)&bufferRead[currPos]);
        currPos += sizeof(float);
        data.speed = *((float *)&bufferRead[currPos]);
        currPos += sizeof(float);
        data.cog = *((float *)&bufferRead[currPos]);
        currPos += sizeof(float);
        data.variation = *((float *)&bufferRead[currPos]);
        currPos += sizeof(float);
        char lastChar = bufferRead[currPos];
        if (currPosGPSObj != 0)
        {
            fprintf(pythonArrayF, ",");
        }
        fprintf(pythonArrayF, "%.2f", data.dop_p);
        if (lastChar != '\n')
        {
            int i = 0;
            while (lastChar != '\n')
            {
                i++;
                currPos++;
                lastChar = bufferRead[currPos];
            }
            printf("Can't parse GPS\n");
            return;
        }
        currPos++;
        if (!firstValid && data.valid)
        {
            Get_duration_diffGPSTime(gps_Objs[0].tim, data.tim, &gpsAnalyze.timeToValid);
            firstValid = true;
        }
        if (!firstGPS && data.fix == GPS_FIX_GPS)
        {
            Get_duration_diffGPSTime(gps_Objs[0].tim, data.tim, &gpsAnalyze.timeToGPS);
            firstGPS = true;
        }
        if (!firstDGPS && data.fix == GPS_FIX_DGPS)
        {
            Get_duration_diffGPSTime(gps_Objs[0].tim, data.tim, &gpsAnalyze.timeToDGPS);
            firstDGPS = true;
        }
        if (!first2D && data.fix_mode == GPS_MODE_2D)
        {
            Get_duration_diffGPSTime(gps_Objs[0].tim, data.tim, &gpsAnalyze.timeTo2D);
            first2D = true;
        }
        if (!first3D && data.fix_mode == GPS_MODE_3D)
        {
            Get_duration_diffGPSTime(gps_Objs[0].tim, data.tim, &gpsAnalyze.timeTo3D);
            first3D = true;
        }
        if (firstValid)
        {
            gpsAnalyze.meanDopH += data.dop_h;
            gpsAnalyze.meanDopP += data.dop_p;
            gpsAnalyze.meanDopV += data.dop_v;
            gpsAnalyze.meanSatsInUseValid += data.sats_in_use;
            gpsAnalyze.meanSatsInViewValid += data.sats_in_view;
        }
        memcpy(&gps_Objs[currPosGPSObj], &data, sizeof(gps_t));
        currPosGPSObj++;
    }
    fprintf(pythonArrayF, "]");
    fprintf(pythonArrayF, "\nx=[");
    for (size_t i = 0; i < currPosGPSObj; i++)
    {
        if (i != 0)
            fprintf(pythonArrayF, ",");
        fprintf(pythonArrayF, "%i", (int)i);
    }
    fprintf(pythonArrayF, "]");
    fclose(pythonArrayF);
    gpsAnalyze.meanDopH += ((double)gpsAnalyze.meanDopH) / (double)(currPosGPSObj);
    gpsAnalyze.meanDopP += ((double)gpsAnalyze.meanDopP) / (double)(currPosGPSObj);
    gpsAnalyze.meanDopV += ((double)gpsAnalyze.meanDopV) / (double)(currPosGPSObj);
    gpsAnalyze.meanSatsInUseValid += ((double)gpsAnalyze.meanSatsInUseValid) / (double)(currPosGPSObj);
    gpsAnalyze.meanSatsInViewValid += ((double)gpsAnalyze.meanSatsInViewValid) / (double)(currPosGPSObj);
}

int main(void)
{
    // AnalyzeGPS();
    SDL_Renderer *renderer;
    SDL_Window *window;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(EPD_3IN7_HEIGHT, EPD_3IN7_WIDTH, 0, &window, &renderer);
    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // for (i = 0; i < EPD_3IN7_WIDTH; ++i)
    //     SDL_RenderDrawPoint(renderer, i, i);

    UWORD Imagesize = ((EPD_3IN7_WIDTH % 4 == 0) ? (EPD_3IN7_WIDTH / 4) : (EPD_3IN7_WIDTH / 4 + 1)) * EPD_3IN7_HEIGHT;
    if ((ScreenBuffer = (UBYTE *)malloc(Imagesize)) == NULL)
    {
        printf("Failed to apply for Screen memory...\r\n");
        return 1;
    }
    printf("Paint_NewImage \r\n");
    Paint_NewImage(ScreenBuffer, EPD_3IN7_WIDTH, EPD_3IN7_HEIGHT, 270, WHITE);
    Paint_SelectImage(ScreenBuffer);
    Paint_SetScale(2);
    Paint_Clear(WHITE);
    startRender();
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    simulation_init(true, "2023-05-06-XCT-TLE-09.igc");

    int sizeXBuffer = EPD_3IN7_WIDTH % 8 == 0 ? EPD_3IN7_WIDTH / 8 : EPD_3IN7_WIDTH / 8 + 1;
    while (1)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_DOWN:
                    startDownArrow();
                    break;
                case SDLK_UP:
                    startUpArrow();
                    break;
                case SDLK_m:
                    startMenuKey();
                    break;
                case SDLK_RETURN:
                    startEnterKey();
                    break;
                case SDLK_p:
                    PKeyPressed();
                    break;
                case SDLK_f:
                    if (!startedFKey)
                    {
                        startedFKeyTime = SDL_GetTicks();
                        startedFKey = true;
                    }
                    else
                    {
                        simulation_fastForward(0.005f);
                    }
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_DOWN:
                    DownArrowReleased();
                    break;
                case SDLK_UP:
                    UpArrowReleased();
                    break;
                case SDLK_m:
                    MenuKeyReleased();
                    break;
                case SDLK_RETURN:
                    EnterKeyReleased();
                    break;
                case SDLK_f:
                    startedFKey = false;
                    break;
                }
                break;
                // Handle other event types as needed
            }
        }

        updateRender();
        simulation_nextStep();
        for (int y = 0; y < EPD_3IN7_HEIGHT; y++)
        {
            for (int x = 0; x < EPD_3IN7_WIDTH; x++)
            {
                int addr = x / 8 + y * sizeXBuffer;
                uint8_t rdata = ScreenBuffer[addr];
                // Black
                if ((rdata & (0x80 >> (x % 8))) == 0)
                {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    SDL_RenderDrawPoint(renderer, EPD_3IN7_HEIGHT - 1 - y, x);
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
                    SDL_RenderDrawPoint(renderer, EPD_3IN7_HEIGHT - 1 - y, x);
                }
            }
        }
        Uint32 current_time = SDL_GetTicks();
        if (current_time - lastRenderTime > 300)
        {
            SDL_RenderPresent(renderer);
            lastRenderTime = current_time;
        }
        SDL_Delay(5);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}