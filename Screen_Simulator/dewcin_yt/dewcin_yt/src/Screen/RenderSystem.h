#ifndef RenderSystem_h
#define RenderSystem_h

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    RenderMode_Normal,
    RenderMode_Settings,
    RenderMode_PopUp,
} RenderModes_t;

void rendering_Settings(bool varfullRedraw);
void rendering_Normal(bool varfullRedraw);
void rendering_PopUp(bool varfullRedraw);

#endif