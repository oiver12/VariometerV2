#include "FlightLogger.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "nmea_parser.h"
#include <string.h>

static const char *TAG = "FlightLogger";
char fileNameVario[40];
char fileNameNMEA[40];

static void NMEASentenceLogger(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id) {
    case GPS_NMEA_SENTECE:
        ESP_LOGI(TAG, "NMEA Sentence:%s", (char *)event_data);
        break;
    default:
        break;
    }

    ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
}

static void LoggingTask(void *arg)
{

}

void startLogging(sdcard_t *card)
{
    if(!(card->cardInit))
    {
        init_sdcard(card, "/sdcard");
    }
    int count = 0;
    do{
        snprintf(fileNameVario, sizeof(fileNameVario), "/FlightLoggs/Vario%02d.bytes", count++);
    }while(fileExists_sdcard(card, fileNameVario));
    ESP_LOGI(TAG, "Found file: %s", fileNameVario);
    snprintf(fileNameVario, sizeof(fileNameVario), "Vario%02d.bytes", --count);
    FILE *fileVario = openFile_sdcard(card, "/FlightLoggs", fileNameVario);
    fclose(fileVario);
    
    count = 0;
    do{
        snprintf(fileNameNMEA, sizeof(fileNameNMEA), "/FlightLoggs/NMEA%02d.bytes", count++);
    }while(fileExists_sdcard(card, fileNameNMEA));
    ESP_LOGI(TAG, "Found file: %s", fileNameNMEA);
    snprintf(fileNameNMEA, sizeof(fileNameNMEA), "NMEA%02d.bytes", --count);
    FILE *fileNMEA = openFile_sdcard(card, "/FlightLoggs", fileNameNMEA);
}
void endLogging()
{

}