#ifndef __FLIGHTLOGGER_H_
#define __FLIGHTLOGGER_H_

#include <esp_err.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    const char *base_path;       /*!< File path prefix associated with the filesystem. */
    const char *partition_label; /*!< Optional, label of SPIFFS partition to use. If set to NULL, first partition with subtype=spiffs will be used. */
    size_t max_files;            /*!< Maximum files that could be open at the same time. */
    bool format_if_mount_failed; /*!< If true, it will format the file system if it fails to mount. */
} esp_vfs_spiffs_conf_t;

esp_err_t esp_vfs_spiffs_register(const esp_vfs_spiffs_conf_t *conf);
esp_err_t esp_spiffs_info(const char *partition_label, size_t *total_bytes, size_t *used_bytes);
esp_err_t esp_spiffs_format(const char *partition_label);
esp_err_t esp_spiffs_check(const char *partition_label);

#endif