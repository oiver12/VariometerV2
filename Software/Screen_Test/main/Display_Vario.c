#include "Display_Vario.h"
#include "esp_timer.h"
#include <inttypes.h>
#include "E_Ink/EPD_3in7.h"
#include "E_Ink/GUI_Paint.h"
#include <esp_log.h>
#include <math.h>
#include <vario_config.h>

#define Disp_BufferSize_Full 16800

uint8_t* buffer;
int area_width = 0;

void EPD_3in7_setBuffer()
{
    buffer = heap_caps_malloc(Disp_BufferSize_Full, MALLOC_CAP_DMA);
    memset(buffer, 0xFF, Disp_BufferSize_Full);
    ESP_LOGI("Display_Vario", "Heap Size: %i", esp_get_free_heap_size());
}

void EPD_3in7_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
    static int RefreshCounter = 0;
    #ifdef EInk_DoubleBuffer
    volatile uint32_t size = lv_area_get_size(area);
    uint8_t* Byte_Buffer = (uint8_t*)color_map;
    int i = 0;
    static bool firstTime = true;
    static uint64_t startTime;
    if(firstTime)
    {
        startTime = esp_timer_get_time();
        firstTime = false;
    }
    for (size_t x = area->x1; x < area->x2 + 1; x++)
    {
        for (size_t y = area->y1; y < area->y2 + 1; y++)
        {   
            uint16_t Xpoint = -y + 280;
            uint16_t Ypoint = x;

            uint32_t addr = Xpoint / 8 + Ypoint * 35;
            int colorMapAddr = (x - area->x1) + ((y - area->y1) * (area->x2-area->x1+1));
            if(Byte_Buffer[colorMapAddr] == 0)
                buffer[addr] = buffer[addr] & ~(0x80 >> (Xpoint % 8));
            else
                buffer[addr] = buffer[addr] | (0x80 >> (Xpoint % 8));
            i++;
        }
    }
    if(lv_disp_flush_is_last(drv))
    {
        printf("Time: %" PRIu64 "ms \n", (esp_timer_get_time() - startTime) / 1000);
        firstTime = true;
        EPD_3IN7_1Gray_Display(buffer, RefreshCounter % 10 == 0 ? FULL_G_T_BW : PART_BW_T_BW);
        printf("Refreshed\n");
        RefreshCounter++;
    }
    lv_disp_flush_ready(drv);
    #else
    EPD_3IN7_1Gray_Display_Part((uint8_t*)color_map, -area->y2 + 280, -area->y1 + 280, area->x1, area->x2, PART_BW_T_BW);
    lv_disp_flush_ready(drv);
    #endif
}

void EPD_3IN7_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area)
{
    // //Xpoint always needs to be at beginning from Byte. Xpoint needs to be at the end from byte
    // //To nearest Multiple down: use y because it will be rotated and use y2 because y2 will be x1 when rotated
    // area->y2 = (area->y2 / 8) * 8;
    // //https://stackoverflow.com/questions/3407012/rounding-up-to-the-nearest-multiple-of-a-number funktioniert weil 8 power Off Two
    // //To nearest Multiple up: will be x2 when rotated
    // area->y1 = ((area->y1 + 8 - 1) & -8) - 1;
    // int newEckX1 = area->x1;
    // int newEckY1 = area->y2;
    // int newEckX2 = area->x2;
    // int newEckY2 = area->y1;
    //beim gedrehten Rechteck muss (x2new - x1new) / 8 teilbar sein. Bei dem ungedrehten ist diese Länge y2 - y1.
    area_width = area->y2 - area->y1 + 1;
    if (area_width % 8 != 0)
    {
        area_width = ((area_width + 8 - 1) & -8);
        if (area->y1 + area_width <= 279)
            area->y2 = area->y1 + area_width - 1;
        else if (area->y2 - area_width >= 0)
            area->y1 = area->y2 - area_width + 1;
        else //TODO: Wenn z.B. noch zwei gebraucht werden, um auf 8 zu kommen, aber auf beiden Seiten noch 1 frei ist, muss auf beide Seiten erweitert werden.
            printf("Komisch schiesse\n");
    }
}

void EPD_3IN7_set_px_cb(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    //(Width % 8 == 0)? (Width / 8 ): (Width / 8 + 1); = 60
    //UDOUBLE Addr = X / 8 + Y * Paint.WidthByte;
    //Rotate 270
    // uint16_t Xpoint = x;
    // uint16_t Ypoint = y;
    uint16_t Xpoint = -y + 279;
    volatile uint16_t Ypoint = x;

    uint32_t addr = Xpoint / 8 + Ypoint * (area_width / 8);
    uint8_t rdata = buf[addr];
    if(color.full == 0)
        buf[addr] = rdata & ~(0x80 >> (Xpoint % 8));
    else
        buf[addr] = rdata | (0x80 >> (Xpoint % 8));
}