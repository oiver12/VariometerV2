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
#include <inttypes.h>
#include "E_Ink/GUI_Paint.h"

#ifdef EInk_DoubleBuffer
#define DISP_BUF_SIZE 19200
#define Disp_BufferSize_Full 16800
#else
#define DISP_BUF_SIZE 16800
#endif
#endif
#define TAG "DISPLAY_VARIO"
#define LV_TICK_PERIOD_MS 3

#ifdef SCREEN_EINK
uint8_t* ScreenBuffer;

#ifdef EInk_DoubleBuffer
void EPD_3in7_setBuffer()
{
    buffer = heap_caps_malloc(Disp_BufferSize_Full, MALLOC_CAP_DMA);
    memset(buffer, 0xFF, Disp_BufferSize_Full);
}
#endif

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
        //printf("Time: %" PRIu64 "ms \n", (esp_timer_get_time() - startTime) / 1000);
        firstTime = true;
        EPD_3IN7_1Gray_Display(buffer,  RefreshCounter % 20 == 0 ? FULL_G_T_BW : PART_BW_T_BW);
        //printf("Refreshed\n");
        RefreshCounter++;
    }
    lv_disp_flush_ready(drv);
    #else
    EPD_3IN7_1Gray_Display_Part((uint8_t*)color_map, -area->y2 + 280, -area->y1 + 280, area->x1, area->x2, PART_BW_T_BW);
    lv_disp_flush_ready(drv);
    #endif
}

void EPD_3IN7_set_px_cb(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    uint16_t Xpoint = -y + 280;
    volatile uint16_t Ypoint = x;

    uint32_t addr = Xpoint / 8 + Ypoint * 35;
    uint8_t rdata = buf[addr];
    if(color.full == 0)
        buf[addr] = rdata & ~(0x80 >> (Xpoint % 8));
    else
        buf[addr] = rdata | (0x80 >> (Xpoint % 8)); 
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
    EPD_3IN7_4Gray_Init();
    EPD_3IN7_4Gray_Clear();
    
    EPD_3IN7_Sleep();

    DEV_Module_Exit();

    vTaskDelay(10000 / portTICK_PERIOD_MS); //temporär

    DEV_Module_Init();
    EPD_3IN7_1Gray_Init();       //init 1 Gray mode
    EPD_3IN7_1Gray_Clear();

    ScreenManager_init();
    
    static WidgetData widgetDataScreen0[7] = {
        {.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Vario_Active, .x = 0, .y = 27},
        {.extraStyle = WS_FONT_20_CONTENT, .h = 130, .w = 100, .style = Widget_Compass, .type = WidgetContent_CompassSensor, .x = 0, .y = 87},
        {.extraStyle = WS_FONT_20_CONTENT, .h = 193, .w = 180, .style = Widget_Chart, .type = WidgetContent_Altitude_Baro, .x = 100, .y = 87},
        {.extraStyle = WS_FONT_24_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_VertAcc, .x = 160, .y = 27},
        {.extraStyle = WS_FONT_16_CONTENT, .h = 60, .w = 160, .style = Widget_TextBox, .type = WidgetContent_Pressure, .x = 320, .y = 27},
        {.extraStyle = WS_FONT_32_CONTENT, .h = 100, .w = 200, .style = Widget_SdCardInfo, .type = WidgetContent_SDCardInfo, .x = 280, .y = 87},
		{.extraStyle = WS_FONT_24_CONTENT, .h = 63, .w = 200, .style = Widget_TextBox, .type = WidgetContent_Vario_Lazy, .x = 280, .y = 187},
    };
    ScreenManager_add_Screen_add_widgets(widgetDataScreen0, 7);
    ScreenManager_show_Screen(0);

    uint64_t startTime = esp_timer_get_time();
    int counter = 0;
    while(1)
    {
        //TODO: Render while Display is refreshing
        uint64_t startTimeRender = esp_timer_get_time();
        if(counter % 10 == 0)
            ScreenManager_draw_status_bar();
        counter++;
        ScreenManager_update();
        //printf("RenderTime: %i ms \n", (int)((esp_timer_get_time() - startTimeRender) / 1000));
        EPD_3IN7_1Gray_Display(ScreenBuffer, counter % 150 == 0 ? FULL_G_T_BW : PART_BW_T_BW);
        // if(esp_timer_get_time() - startTime > 40 * 1000 * 1000)
        //     break;
    }
    vTaskDelete(NULL);
}

esp_err_t init_Display()
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

    DEV_Module_Init();
    EPD_3IN7_1Gray_Init();       //init 1 Gray mode
    EPD_3IN7_1Gray_Clear();
    ESP_LOGI(TAG, "Screen initialized");
    #endif
    #ifdef SCREEM_ILI9488
    //mit ili9488 können wir mit zwei Buffer arbeiten. Mit E Ink nicht, da wir custom
    //set_px_cb benutzen und dann mit lvgl nur ein Buffer benutzen können.
    lvgl_driver_init();
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);
    uint32_t size_in_px = DISP_BUF_SIZE;
    #endif
    #ifdef SCREEN_ILI9488
    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

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
    #endif
    if ((ScreenBuffer = (uint8_t*)heap_caps_malloc(DISP_BUF_SIZE, MALLOC_CAP_DMA)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return;
    }

    printf("Paint_NewImage \r\n");
    Paint_NewImage(ScreenBuffer, EPD_3IN7_WIDTH, EPD_3IN7_HEIGHT, 270, WHITE);

    Paint_SelectImage(ScreenBuffer);
    Paint_SetScale(2);

    Paint_Clear(WHITE);

    xTaskCreate(render_Screen_Task, "Screen Render", 4 * 1024, NULL, TASK_SCREEN_RENDER_PRIORITY, NULL);
    return ESP_OK;
}