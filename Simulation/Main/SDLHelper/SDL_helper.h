#ifndef __SDL_HELPER_H__
#define __SDL_HELPER_H__

#include <stdint.h>

void SDLH_initializeScreen(int width, int height);
void SDLH_update(uint8_t *buffer);
void SDLH_HandleKey();

#endif