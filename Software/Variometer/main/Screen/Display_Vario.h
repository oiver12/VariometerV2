#ifndef __DISPLAYVARIO_H
#define __DISPLAYVARIO_H

#include "../vario_config.h"
#ifdef SCREEM_ILI9488
#include "lvgl.h"
#include "lvgl_helpers.h"
#endif

#include <esp_err.h>

esp_err_t init_Display();
esp_err_t Display_powerOff_Screen();
esp_err_t Display_SetFullRefresh();

#endif