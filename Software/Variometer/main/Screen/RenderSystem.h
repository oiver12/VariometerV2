#ifndef RenderSystem_h
#define RenderSystem_h

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    RenderMode_Normal,
    RenderMode_Settings,
} RenderModes_t;

void startRendering(uint8_t* screenBuffer);
void stopRendering_disableScreen();
void rendering_Settings(bool varfullRedraw);
void rendering_Normal(bool varfullRedraw);

#endif