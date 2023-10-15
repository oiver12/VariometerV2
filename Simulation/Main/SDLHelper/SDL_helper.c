#include "SDLHelper/SDL_helper.h"
#include "ScreenManager.h"

#include <stddef.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdbool.h>
#include <esp_timer.h>
#include <esp_log.h>

SDL_Renderer *renderer;
SDL_Window *window;
int windowWidth;
int windowHeight;

uint64_t startTimeDown = 0.0f;
uint8_t startedDown = false;
uint8_t startedLeft = false;
uint8_t startedRight = false;
uint8_t startedEnter = false;

void startDownArrow()
{
    if (!startedLeft)
        startedLeft = true;
}

void DownArrowReleased()
{
    if (startedLeft)
    {
        ScreenManager_DownArrowPressed();
        startedLeft = false;
    }
}

void startUpArrow()
{
    if (!startedRight)
        startedRight = true;
}

void UpArrowReleased()
{
    if (startedRight)
    {
        ScreenManager_UpArrowPressed();
        startedRight = false;
    }
}

void startEnterKey()
{
    if (!startedEnter)
        startedEnter = true;
}

void EnterKeyReleased()
{
    if (startedEnter)
    {
        ScreenManager_EnterKeyPressed();
        startedEnter = false;
    }
}

void startMenuKey()
{
    if (!startedDown)
    {
        startTimeDown = esp_timer_get_time(); // in microseconds
        startedDown = true;
    }
}

void MenuKeyReleased()
{
    if (startedDown)
    {
        uint64_t difference = ((esp_timer_get_time()) - startTimeDown) / 1000;
        startedDown = false;
        ScreenManager_powerButtonPressed(difference > 500);
    }
}

void SDLH_initializeScreen(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        ESP_LOGE("SDL", "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer) < 0)
    {
        ESP_LOGE("SDL", "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    windowWidth = height;
    windowHeight = width;
    // Clear the screen
    SDL_RenderClear(renderer);
    // Set the color to draw
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Red color (R, G, B, Alpha)

    SDL_Rect rect = {300, 200, 200, 200};
    SDL_RenderFillRect(renderer, &rect);
    // Update the screen
    SDL_RenderPresent(renderer);
}

void SDLH_update(uint8_t *buffer)
{
    int sizeXBuffer = windowWidth % 8 == 0 ? windowWidth / 8 : windowWidth / 8 + 1;
    for (int y = 0; y < windowHeight; y++)
    {
        for (int x = 0; x < windowWidth; x++)
        {
            int addr = x / 8 + y * sizeXBuffer;
            uint8_t rdata = buffer[addr];
            // Black
            if ((rdata & (0x80 >> (x % 8))) == 0)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderDrawPoint(renderer, windowHeight - 1 - y, x);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
                SDL_RenderDrawPoint(renderer, windowHeight - 1 - y, x);
            }
        }
    }
    // Update the screen
    SDL_RenderPresent(renderer);
}

void SDLH_HandleKey()
{
    bool startedFKey = false;
    Uint32 startedFKeyTime = 0;
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
                // case SDLK_p:
                //     PKeyPressed();
                //     break;
                // case SDLK_f:
                //     if (!startedFKey)
                //     {
                //         startedFKeyTime = SDL_GetTicks();
                //         startedFKey = true;
                //     }
                //     else
                //     {
                //         simulation_fastForward(0.005f);
                //     }
                //     break;
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
}