#include "flashMemory.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

nvs_handle_t nvsHandle;

void flashMemoryInit(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
    ESP_ERROR_CHECK(err);
}

void flashMemoryWriteInt32(const char* name, int32_t value)
{
    if(nvsHandle == 0)
        return;
    esp_err_t err = nvs_set_i32(nvsHandle, name, value);
    ESP_ERROR_CHECK(err);
    err = nvs_commit(nvsHandle);
    ESP_ERROR_CHECK(err);
}

bool flashMemoryReadInt32(const char* name, int32_t *value)
{
    if(nvsHandle == 0)
        return false;
    printf("Read flash Memory\n");
    esp_err_t err = nvs_get_i32(nvsHandle, name, value);
    //ESP_ERR_NVS_NOT_FOUND means that the value is not initialized yet in NVS
    if(err == ESP_OK)
        return true;
    else
        return false;
}