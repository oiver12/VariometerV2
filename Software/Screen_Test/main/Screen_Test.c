#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "vario_config.h"
#include "math.h"
#include <inttypes.h>
#include <esp_log.h>
#include "Display_Vario.h"
#ifdef E_INK_DISPLAY
#include "E_Ink/EPD_3in7.h"
#include "E_Ink/GUI_Paint.h"
#endif

#define EPD_RST_PIN     45
#define EPD_DC_PIN      42
#define EPD_CS_PIN      39
#define EPD_BUSY_PIN    40
#define EPD_MOSI_PIN    41
#define EPD_CLK_PIN     46

#define DISP_BUF_SIZE 33600

#define TAG "demo"
#define LV_TICK_PERIOD_MS 3

#define POWSW_ENABLE 13

extern void example_lvgl_demo_ui();

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void powerOffScreen()
{
    EPD_3IN7_4Gray_Init();
    EPD_3IN7_4Gray_Clear();
    
    // Sleep & close 5V
    printf("Goto Sleep...\r\n");
    EPD_3IN7_Sleep();

    printf("close 5V, Module enters 0 power consumption ...\r\n");
    DEV_Module_Exit();
}

void LVGL_TaskHandle_Task(void *arg)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_task_handler();
        //printf("Time to run: %i\n", time);
    }
    vTaskDelete(NULL);
}

void TaskScreenRefresher(void *param)
{
    lv_obj_t* information_text = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(information_text, "Test");
    // int i = 0;
    // char buffer[20];
    // while(true)
    // {
    //     i++;
    //     snprintf(buffer, 20, "%i", i);
    //     lv_label_set_text(information_text, buffer);
    //     vTaskDelay(20 / portTICK_PERIOD_MS);
    // }
    vTaskDelete(NULL);
}

void my_monitor_cb(lv_disp_drv_t * disp_drv, uint32_t time, uint32_t px)
{
  printf("%d px refreshed in %d ms\n", px, time);
}

extern void testFunction_EPD3IN7();

void app_main(void)
{
    gpio_config_t enableGPIOOutput = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << POWSW_ENABLE,
        .pull_down_en = 0,
        .pull_up_en = 0
    };

    gpio_config(&enableGPIOOutput);
    gpio_set_level(POWSW_ENABLE, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    #ifdef ILI9488_DISPLAY 
    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();
    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    static lv_disp_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;

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
    example_lvgl_demo_ui();

    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_task_handler();
    }
    #endif
    #ifdef E_INK_DISPLAY

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

    lv_init();
    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    //lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    //assert(buf2 != NULL);

    static lv_disp_buf_t disp_buf;
    //uint32_t size_in_px = DISP_BUF_SIZE;
    uint32_t size_in_px = 480 * 280;

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, NULL, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = EPD_3in7_display_flush;
    disp_drv.set_px_cb = EPD_3IN7_set_px_cb;
    disp_drv.monitor_cb = my_monitor_cb;
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
    TaskHandle_t lvglTaskHandler, RefreshScreen;
    xTaskCreatePinnedToCore(LVGL_TaskHandle_Task, "LVGL TaskHandler", 4 * 1024, NULL, 2, &lvglTaskHandler, 0);
    xTaskCreatePinnedToCore(TaskScreenRefresher, "Refresh Screen", 4 * 1024, NULL, 8, &RefreshScreen, 0);
    vTaskDelay(20000 / portTICK_PERIOD_MS);
    vTaskDelete(lvglTaskHandler);
    //vTaskDelete(RefreshScreen);
    lv_deinit();
    powerOffScreen();
    #endif
}
