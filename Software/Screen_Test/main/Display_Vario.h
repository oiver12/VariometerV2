#include "lvgl.h"

void EPD_3in7_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);

void EPD_3IN7_set_px_cb(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa);