#include "SDCard.h"
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include <esp_log.h>
#include <string.h>
#include "driver/sdmmc_host.h"
#include <errno.h>
#include "DeviceContext.h"

static const char *TAG = "SDCard";

#define SDCARD_DETECT 14

esp_err_t init_sdcard(sdcard_t *sdcard, const char *Mount_Point)
{
    // /* Configure SDCard */
    gpio_config_t sdcard_detect_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << SDCARD_DETECT,
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&sdcard_detect_conf);
    int sdCardLevel = gpio_get_level(SDCARD_DETECT);
    ESP_LOGI(TAG, "SDCard level: %i", sdCardLevel);
    
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4;
    slot_config.clk = 4;
    slot_config.cmd = 5;
    slot_config.d0 = 6;
    slot_config.d1 = 7;
    slot_config.d2 = 15;
    slot_config.d3 = 16;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    sd_card_state_t sdcard_state_vario; //for Vario State
    memset(&sdcard_state_vario, 0, sizeof(sd_card_state_t));

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(Mount_Point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        sdcard_state_vario.status |= SD_CARD_STATE_ERROR_INIT;
        device_state_set_member(&sdcard_state_vario, device_state_statusSDCard);
        return ret;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    sdcard->cardInit = true;
    sdcard->card = card;
    sdcard->Mount_Point = malloc(strlen(Mount_Point) + 1);
    strcpy(sdcard->Mount_Point, Mount_Point);
    /* Get volume information and free clusters of sdcard */
    FATFS *fs;
    uint32_t fre_clust, fre_sect, tot_sect;
	FRESULT res = f_getfree(sdcard->Mount_Point, &fre_clust, &fs);
	if (res) {
		printf("Couldn't find out SdCard size\n");
        sdcard_state_vario.freeSpace = 0;
        sdcard_state_vario.maxSpace = 0;
        sdcard_state_vario.status |= SD_CARD_STATE_INIT;
        device_state_set_member(&sdcard_state_vario, device_state_statusSDCard);
        return ret;
	}

	/* Get total sectors and free sectors */
	tot_sect = (fs->n_fatent - 2) * fs->csize;
	fre_sect = fre_clust * fs->csize;
	
	uint64_t tmp_total_bytes = (uint64_t)tot_sect * FF_SS_SDCARD;
	uint64_t tmp_free_bytes = (uint64_t)fre_sect * FF_SS_SDCARD;

	/* Print the free space in bytes */
	printf("%llu total bytes. %llu free bytes. sector_size=%u", tmp_total_bytes, tmp_free_bytes, FF_SS_SDCARD);

    sdcard_state_vario.status |= SD_CARD_STATE_INIT;
    sdcard_state_vario.maxSpace = tmp_total_bytes;
    sdcard_state_vario.freeSpace = tmp_free_bytes;
    device_state_set_member(&sdcard_state_vario, device_state_statusSDCard);
    return ret;
}

esp_err_t deinit_sdcard(sdcard_t *sdcard)
{
    esp_vfs_fat_sdcard_unmount(sdcard->Mount_Point, sdcard->card);
    free(sdcard->Mount_Point);
    return ESP_OK;
}

//TODO: directory not working
FILE *openFile_sdcard(sdcard_t *sdcard, const char *directory, const char* fileName, bool createNew)
{
    char buffer[100];
    strcpy(buffer, sdcard->Mount_Point);
    struct stat st = {0};   
    if(directory != NULL)
    {
        strcat(buffer, directory);
        ESP_LOGI(TAG, "Directory: %s", buffer);
        if(stat(buffer, &st) == -1)
        {
            if(mkdir(buffer, 0700) != 0)
            {
                volatile char *testValue = strerror(errno);
                ESP_LOGE(TAG, "Could not create Directory: %s", testValue);
            }
            else
                ESP_LOGI(TAG, "Createt Directory");
        }
        else
        {
            ESP_LOGI(TAG, "Direactory already exists");
        }
    }
    strcat(buffer, "/");
    strcat(buffer, fileName);
    //ESP_LOGI(TAG, "File Destination: %s", buffer);
    if(stat(buffer, &st) != -1 && createNew)
    {
        ESP_LOGE(TAG, "File already exists");
    }
    if(createNew)
        return fopen(buffer, "w");
    else
        return fopen(buffer, "a");
}

bool fileExists_sdcard(sdcard_t *sdcard, const char *fileName)
{
    char buffer[strlen(sdcard->Mount_Point) + strlen(fileName) + 1];
    strcpy(buffer, sdcard->Mount_Point);
    strcat(buffer, fileName);
    struct stat st = {0};   
    return stat(buffer, &st) != -1;
}

uint64_t sdcard_remainingSpace_update(sdcard_t *sdcard)
{
    if(!sdcard->cardInit)
        return 0;
    sd_card_state_t state;
    device_state_get_member(&state, device_state_statusSDCard, false);

    FATFS *fs;
    uint32_t fre_clust, fre_sect, tot_sect;
	FRESULT res = f_getfree(sdcard->Mount_Point, &fre_clust, &fs);
	if (res) {
		printf("Couldn't find out SdCard size\n");
        state.freeSpace = 0;
        state.maxSpace = 0;
        state.status |= SD_CARD_STATE_INIT;
        device_state_set_member(&state, device_state_statusSDCard);
        return 0;
	}

	/* Get total sectors and free sectors */
	tot_sect = (fs->n_fatent - 2) * fs->csize;
	fre_sect = fre_clust * fs->csize;
	
	uint64_t tmp_total_bytes = (uint64_t)tot_sect * FF_SS_SDCARD;
	uint64_t tmp_free_bytes = (uint64_t)fre_sect * FF_SS_SDCARD;

    state.freeSpace = tmp_free_bytes;
    state.maxSpace = tmp_total_bytes;
    device_state_set_member(&state, device_state_statusGPS);
    return tmp_free_bytes;
}