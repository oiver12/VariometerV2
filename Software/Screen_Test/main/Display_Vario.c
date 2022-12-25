#include "Display_Vario.h"
#include "esp_timer.h"
#include <inttypes.h>
#include "E_Ink/EPD_3in7.h"

void EPD_3in7_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
    uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);
    uint8_t *buffer_1Byte = (uint8_t*)color_map;
    // for (size_t i = 0; i < size; i++)
    // {
    //     if(i > 0) printf(",");
    //     printf("%02X", buffer_1Byte[i]);
    // }
    // printf("\n");
    static int i = 0;
    if(i > 0)
    {
        for (size_t BufferIndex = 0; BufferIndex < size; BufferIndex++)
        {
            printf("%i, ", buffer_1Byte[BufferIndex]);
        }
        printf("\n");
    }
    i++;
    EPD_3IN7_1Gray_Display_Part(buffer_1Byte, area->x1, area->y1, area->x2, area->y2, PART_BW_T_BW);
    // static uint64_t time = 0;
    // printf("Time: %" PRId64 "\n",(esp_timer_get_time()-time) / 1000);
    // time = esp_timer_get_time();
    lv_disp_flush_ready(drv);
}

void EPD_3IN7_set_px_cb(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    //(Width % 8 == 0)? (Width / 8 ): (Width / 8 + 1); = 60
    //UDOUBLE Addr = X / 8 + Y * Paint.WidthByte;
    uint32_t addr = x / 8 + y * 60;
    uint8_t rdata = buf[addr];
    if(color.full == 0)
        buf[addr] = rdata & ~(0x80 >> (x % 8));
    else
        buf[addr] = rdata | (0x80 >> (x % 8));
}