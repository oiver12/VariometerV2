#include "ErrorHandling.h"
#include <esp_log.h>

const char *TAG = "ERROR HANDLER";

void error_handle(esp_err_t err, error_location location, char *message)
{
    ESP_LOGE(TAG, "Message, error: %s, place: %i", esp_err_to_name(err), (int)location);
}