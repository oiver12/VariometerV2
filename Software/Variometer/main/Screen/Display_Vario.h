#ifndef __DISPLAYVARIO_H
#define __DISPLAYVARIO_H

#include "lvgl.h"
#include "../vario_config.h"
#ifdef SCREEM_ILI9488
#include "lvgl_helpers.h"
#endif

#include <esp_err.h>

esp_err_t init_Display();

#endif