#include "Display_Vario.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../vario_config.h"
#include "DeviceContext.h"
#include "Vario_Screen.h"

#define LV_TICK_PERIOD_MS 3

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

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();
    printf("TestSize: %i", DISP_BUF_SIZE);
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

    xTaskCreatePinnedToCore(LVGL_TaskHandle_Task, "LVGL TaskHandler", 4 * 1024, NULL, TASK_LV_TASK_HANDLE_PRIORITY, NULL, 1);
    xTaskCreatePinnedToCore(render_Screen_Task, "Screen Render", 4 * 1024, NULL, TASK_SCREEN_RENDER_PRIORITY, NULL, 1);
    return ESP_OK;
}