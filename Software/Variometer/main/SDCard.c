#include "SDCard.h"
#include <sys/stat.h>
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include <esp_log.h>

static const char *TAG = "SDCard";

#define SDCARD_CS 38
#define SDCARD_DETECT 45
#define SDCARD_MOUNT_POINT "/sdcard"

void TaskSDCard(void *pvParameters)
{
    sdmmc_card_t *card = (sdmmc_card_t *)pvParameters;
    sdmmc_card_print_info(stdout, card);
    vTaskDelete(NULL);
}

esp_err_t start_sdcard()
{
    // /* Configure SDCard */
    // //TODO: Keine SPI Kommunikation, bevor die SD Karte init ist. https://github.com/espressif/esp-idf/issues/1597
    gpio_config_t sdcard_detect_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (uint64_t) 0x1 << SDCARD_DETECT,
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&sdcard_detect_conf);
    int sdCardLevel = gpio_get_level(SDCARD_DETECT);
    ESP_LOGI(TAG, "%i", sdCardLevel);
    if(sdCardLevel != 1)
    {
        //TODO: Was nun machen?
        ESP_LOGI(TAG, "No SD Card detected!");
    }
    else if(sdCardLevel == 1)
    {
        ESP_LOGI(TAG, "SD Card detected!");
    }
    esp_vfs_fat_sdmmc_mount_config_t sdcard_mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    ESP_LOGI(TAG, "Initialized SD Card");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    sdspi_device_config_t slot_config ={ 
        .host_id   = SPI2_HOST, 
        .gpio_cs   = SDCARD_CS, 
        .gpio_cd   = SDSPI_SLOT_NO_CD, 
        .gpio_wp   = SDSPI_SLOT_NO_WP, 
        .gpio_int  = GPIO_NUM_NC};
    ESP_LOGI(TAG, "Mounting filesystem");
    //TODO: schlechte Funktion oder so da kein Error Handling
    esp_err_t ret = esp_vfs_fat_sdspi_mount(SDCARD_MOUNT_POINT, &host, &slot_config, &sdcard_mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    xTaskCreate(TaskSDCard, "TaskSDCard", 4 * 1024, card, 1, NULL);
    return ESP_OK;
}