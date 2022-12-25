#include "Display_Vario.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "DeviceContext.h"
#include "Vario_Screen.h"
#include <esp_log.h>
#ifdef SCREEN_EINK
#include "E_Ink/EPD_3in7.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define DISP_BUF_SIZE 33600
#endif
#define TAG "DISPLAY_VARIO"
#define LV_TICK_PERIOD_MS 3

#ifdef SCREEN_EINK
void EPD_3in7_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
    //Problem: Was tun wenn Startpixel mitten in einem Byte ist? Dumm gelaufen. Darum lieber einfach set_px_cb benutzen
    uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);
    uint8_t *buffer_1Byte = (uint8_t*)color_map;
    // for (size_t i = 0; i < size; i++)
    // {
    //     if(i > 0) printf(",");
    //     printf("%02X", buffer_1Byte[i]);
    // }
    // printf("\n");
    
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
#endif

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void LVGL_TaskHandle_Task(void *arg)
{
    while (1) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        lv_task_handler();
    }
    vTaskDelete(NULL);
}

void render_Screen_Task(void *arg)
{
    Vario_Screen_Manager_t screenManager;
    ScreenManager_init(&screenManager);

    VarioScreen_t screen1;
    ScreenManager_add_init_Screen(&screenManager, &screen1);
    ScreenManager_show_Screen(&screenManager, 0);

    WidgetData widgetData[4] = {
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_30_CONTENT, .h = 80, .w = 150, .style = Widget_TextBox, .type = WidgetContent_Vario_Active, .x = 0, .y = 25},
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 100, .style = Widget_Compass, .type = WidgetContent_CompassSensor, .x = 0, .y = 103},
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 150, .style = Widget_Chart, .type = WidgetContent_AltitudeBaroChart, .x = 98, .y = 103},
        {.alignement = LV_ALIGN_IN_TOP_LEFT, .extraStyle = WS_FONT_20_CONTENT, .h = 80, .w = 150, .style = Widget_TextBox, .type = WidgetContent_Pressure, .x = 148, .y = 25},
    };
    Screen_add_widgets(&screen1, widgetData, 4);

    while(1)
    {
        Screen_update_widgets(&screen1);
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

esp_err_t init_Display_LVGL()
{
    lv_init();

    #ifdef SCREEN_EINK
    spi_bus_config_t spi2cfg={
        .miso_io_num= -1,
        .mosi_io_num=EPD_MOSI_PIN,
        .sclk_io_num=EPD_CLK_PIN,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4092
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi2cfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "SPI initialized");
    // testFunction_EPD3IN7();
    // return;
    DEV_Module_Init();
    EPD_3IN7_1Gray_Init();       //init 1 Gray mode
    EPD_3IN7_1Gray_Clear();
    ESP_LOGI(TAG, "Screen initialized");
    #endif

    /* Initialize SPI or I2C bus used by the drivers */
    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    #ifdef SCREEM_ILI9488
    //mit ili9488 können wir mit zwei Buffer arbeiten. Mit E Ink nicht, da wir custom
    //set_px_cb benutzen und dann mit lvgl nur ein Buffer benutzen können.
    lvgl_driver_init();
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);
    uint32_t size_in_px = DISP_BUF_SIZE;
    #endif
    #ifdef SCREEN_EINK
    lv_color_t* buf2 = NULL;
    uint32_t size_in_px = 480 * 280;
    #endif
    static lv_disp_buf_t disp_buf;

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    #ifdef SCREEN_ILI9488
    disp_drv.flush_cb = disp_driver_flush;
    #endif
    #ifdef SCREEN_EINK
    disp_drv.flush_cb = EPD_3in7_display_flush;
    disp_drv.set_px_cb = EPD_3IN7_set_px_cb;
    #endif
    disp_drv.buffer = &disp_buf;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    xTaskCreatePinnedToCore(LVGL_TaskHandle_Task, "LVGL TaskHandler", 4 * 1024, NULL, TASK_LV_TASK_HANDLE_PRIORITY, NULL, 1);
    xTaskCreatePinnedToCore(render_Screen_Task, "Screen Render", 4 * 1024, NULL, TASK_SCREEN_RENDER_PRIORITY, NULL, 1);
    return ESP_OK;
}