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
#include "lvgl_helpers.h"
#include "vario_config.h"
#ifdef E_INK_DISPLAY
#include "E_Ink/EPD_3in7.h"
#include "E_Ink/GUI_Paint.h"
#endif

#define EPD_RST_PIN     45
#define EPD_DC_PIN      42
#define EPD_CS_PIN      39
#define EPD_BUSY_PIN    40
#define EPD_MOSI_PIN    41

#define TAG "demo"
#define LV_TICK_PERIOD_MS 3

#define POWSW_ENABLE 13

extern void example_lvgl_demo_ui();

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

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

    // gpio_config_t io_conf_output = {
    //     .intr_type = GPIO_INTR_DISABLE,
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pin_bit_mask = (1ULL << EPD_DC_PIN) | (1ULL << EPD_RST_PIN) | (1ULL << EPD_CS_PIN) | (1ULL << EPD_BUSY_PIN) | (1ULL << EPD_MOSI_PIN),
    //     .pull_down_en = 0,
    //     .pull_up_en = 0
    // };
    // gpio_config(&io_conf_output);

    // gpio_set_level(EPD_RST_PIN, 1);
    // while(true)
    // {
    //     vTaskDelay(pdMS_TO_TICKS(10));
    // }
    // return;
    spi_bus_config_t spi2cfg={
        .miso_io_num= -1,
        .mosi_io_num=41,
        .sclk_io_num=46,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4000
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi2cfg, SPI_DMA_CH_AUTO));

    //Create a new image cache
    UBYTE *BlackImage;
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    UWORD Imagesize = ((EPD_3IN7_WIDTH % 4 == 0)? (EPD_3IN7_WIDTH / 4 ): (EPD_3IN7_WIDTH / 4 + 1)) * EPD_3IN7_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }

    DEV_Module_Init();

    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_3IN7_WIDTH, EPD_3IN7_HEIGHT, 270, WHITE);
    Paint_SetScale(4);
    Paint_Clear(WHITE);

    printf("SelectImage:BlackImage\r\n");
    Paint_SelectImage(BlackImage);
    Paint_SetScale(4);
    Paint_Clear(BLACK);

    EPD_3IN7_1Gray_Init();
    printf("Clear...\r\n");
    Paint_DrawPoint(10, 10, WHITE, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    EPD_3IN7_1Gray_Display(BlackImage);
    DEV_Delay_ms(1000);
    EPD_3IN7_4Gray_Init();
    EPD_3IN7_4Gray_Clear();
    
    // Sleep & close 5V
    printf("Goto Sleep...\r\n");
    EPD_3IN7_Sleep();

    free(BlackImage);
    BlackImage = NULL;

    printf("close 5V, Module enters 0 power consumption ...\r\n");
    DEV_Module_Exit();
    #endif
}
