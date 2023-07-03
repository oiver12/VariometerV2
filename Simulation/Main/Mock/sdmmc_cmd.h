#ifndef __SDMMC_H_
#define __SDMMC_H_

#include "driver/gpio.h"

#include <stdint.h>
#include <stdio.h>

typedef struct
{
    uint32_t dummy;
} sdmmc_card_t;

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif
#ifndef BIT64
#define BIT64(nr) (1ULL << (nr))
#endif

/**
 * @brief Configuration arguments for esp_vfs_fat_sdmmc_mount and esp_vfs_fat_spiflash_mount_rw_wl functions
 */
typedef struct
{
    /**
     * If FAT partition can not be mounted, and this parameter is true,
     * create partition table and format the filesystem.
     */
    bool format_if_mount_failed;
    int max_files; ///< Max number of open files
    /**
     * If format_if_mount_failed is set, and mount fails, format the card
     * with given allocation unit size. Must be a power of 2, between sector
     * size and 128 * sector size.
     * For SD cards, sector size is always 512 bytes. For wear_levelling,
     * sector size is determined by CONFIG_WL_SECTOR_SIZE option.
     *
     * Using larger allocation unit size will result in higher read/write
     * performance and higher overhead when storing small files.
     *
     * Setting this field to 0 will result in allocation unit set to the
     * sector size.
     */
    size_t allocation_unit_size;
    /**
     * Enables real ff_disk_status function implementation for SD cards
     * (ff_sdmmc_status). Possibly slows down IO performance.
     *
     * Try to enable if you need to handle situations when SD cards
     * are not unmounted properly before physical removal
     * or you are experiencing issues with SD cards.
     *
     * Doesn't do anything for other memory storage media.
     */
    bool disk_status_check_enable;
} esp_vfs_fat_mount_config_t;

// Compatibility definition
typedef esp_vfs_fat_mount_config_t esp_vfs_fat_sdmmc_mount_config_t;

typedef struct
{
    uint32_t flags; /*!< flags defining host properties */
} sdmmc_host_t;

#define SDMMC_HOST_DEFAULT()            \
    {                                   \
        .flags = SDMMC_HOST_FLAG_8BIT | \
                 SDMMC_HOST_FLAG_4BIT | \
                 SDMMC_HOST_FLAG_1BIT | \
                 SDMMC_HOST_FLAG_DDR,   \
    }

typedef struct
{
    gpio_num_t clk; ///< GPIO number of CLK signal.
    gpio_num_t cmd; ///< GPIO number of CMD signal.
    gpio_num_t d0;  ///< GPIO number of D0 signal.
    gpio_num_t d1;  ///< GPIO number of D1 signal.
    gpio_num_t d2;  ///< GPIO number of D2 signal.
    gpio_num_t d3;  ///< GPIO number of D3 signal.
    gpio_num_t d4;  ///< GPIO number of D4 signal. Ignored in 1- or 4- line mode.
    gpio_num_t d5;  ///< GPIO number of D5 signal. Ignored in 1- or 4- line mode.
    gpio_num_t d6;  ///< GPIO number of D6 signal. Ignored in 1- or 4- line mode.
    gpio_num_t d7;  ///< GPIO number of D7 signal. Ignored in 1- or 4- line mode.
    union
    {
        gpio_num_t gpio_cd; ///< GPIO number of card detect signal
        gpio_num_t cd;      ///< GPIO number of card detect signal; shorter name.
    };
    union
    {
        gpio_num_t gpio_wp; ///< GPIO number of write protect signal
        gpio_num_t wp;      ///< GPIO number of write protect signal; shorter name.
    };
    uint8_t width;  ///< Bus width used by the slot (might be less than the max width supported)
    uint32_t flags; ///< Features used by this slot
#define SDMMC_SLOT_FLAG_INTERNAL_PULLUP BIT(0)
    /**< Enable internal pullups on enabled pins. The internal pullups
     are insufficient however, please make sure external pullups are
     connected on the bus. This is for debug / example purpose only.
     */
#define SDMMC_SLOT_FLAG_WP_ACTIVE_HIGH BIT(1)
    /**< GPIO write protect polarity.
     * 0 means "active low", i.e. card is protected when the GPIO is low;
     * 1 means "active high", i.e. card is protected when GPIO is high.
     */
} sdmmc_slot_config_t;

#define SDMMC_SLOT_CONFIG_DEFAULT()        \
    {                                      \
        .clk = GPIO_NUM_14,                \
        .cmd = GPIO_NUM_15,                \
        .d0 = GPIO_NUM_2,                  \
        .d1 = GPIO_NUM_4,                  \
        .d2 = GPIO_NUM_12,                 \
        .d3 = GPIO_NUM_13,                 \
        .d4 = GPIO_NUM_33,                 \
        .d5 = GPIO_NUM_34,                 \
        .d6 = GPIO_NUM_35,                 \
        .d7 = GPIO_NUM_36,                 \
        .cd = SDMMC_SLOT_NO_CD,            \
        .wp = SDMMC_SLOT_NO_WP,            \
        .width = SDMMC_SLOT_WIDTH_DEFAULT, \
        .flags = 0,                        \
    }

esp_err_t esp_vfs_fat_sdmmc_mount(const char *base_path,
                                  const sdmmc_host_t *host_config,
                                  const void *slot_config,
                                  const esp_vfs_fat_mount_config_t *mount_config,
                                  sdmmc_card_t **out_card);

#endif