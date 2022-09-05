#include "SDCard.h"
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include <esp_log.h>
#include <string.h>

static const char *TAG = "SDCard";

#define SDCARD_CS 38
#define SDCARD_DETECT 45

esp_err_t init_sdcard(sdcard_t *sdcard, const char *Mount_Point)
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
    esp_err_t ret = esp_vfs_fat_sdspi_mount(Mount_Point, &host, &slot_config, &sdcard_mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
    }
    else
    {
        ESP_LOGI(TAG, "Filesystem mounted");
        sdcard->cardInit = true;
    }
    sdcard->card = card;
    sdcard->Mount_Point = malloc(strlen(Mount_Point) + 1);
    strcpy(sdcard->Mount_Point, Mount_Point);
    return ESP_OK;
}

esp_err_t deinit_sdcard(sdcard_t *sdcard)
{
    esp_vfs_fat_sdcard_unmount(sdcard->Mount_Point, sdcard->card);
    spi_bus_free(SPI2_HOST);
    free(sdcard->Mount_Point);
    return ESP_OK;
}

FILE *openFile_sdcard(sdcard_t *sdcard, const char *directory, const char* fileName)
{
    char buffer[strlen(sdcard->Mount_Point) + strlen(directory) + strlen(fileName) + 2];
    strcpy(buffer, sdcard->Mount_Point);
    struct stat st = {0};   
    if(directory != NULL)
    {
        strcat(buffer, directory);
        ESP_LOGI(TAG, "Directory: %s", buffer);
        if(stat(buffer, &st) == -1)
        {
            mkdir(buffer, 0700);
            ESP_LOGI(TAG, "Createt Directory");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Direactory already exists");
    }
    strcat(buffer, "/");
    strcat(buffer, fileName);
    ESP_LOGI(TAG, "File Destination: %s", buffer);
    if(stat(buffer, &st) != -1)
    {
        ESP_LOGE(TAG, "File already exists");
    }
    return fopen(buffer, "w");
}

bool fileExists_sdcard(sdcard_t *sdcard, const char *fileName)
{
    char buffer[strlen(sdcard->Mount_Point) + strlen(fileName) + 1];
    strcpy(buffer, sdcard->Mount_Point);
    strcat(buffer, fileName);
    struct stat st = {0};   
    return stat(buffer, &st) != -1;
}