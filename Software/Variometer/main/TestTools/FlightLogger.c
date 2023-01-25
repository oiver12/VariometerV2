#include "FlightLogger.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "nmea_parser.h"
#include <string.h>
#include "DeviceContext.h"

bool sdCard_init = false;
static const char *TAG = "FlightLogger";
char fileNameVario[40];
char fileNameNMEA[40];
FILE *fileVario;
FILE *fileNMEA;
int countVarioFile;
sdcard_t *sdcard;
const char *directoryLogs = "/Trackloggs";

void FlightLogger_NMEASentenceLogger(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    int64_t time;
    int count = 0;
    switch (event_id) {
    case GPS_NMEA_SENTECE:
        //ESP_LOGI(TAG, "NMEA Sentence:%s", (char *)event_data);
        time = esp_timer_get_time();
        fwrite(&time, sizeof(time), 1, fileNMEA); //write time to file
        fprintf(fileNMEA, "%s\n", (char*)event_data); //write NMEA Sentence and new line to file
        if(count % 20 == 0)
            fflush(fileNMEA);
        count++;
        break;
    default:
        break;
    }
}

void startLogging(sdcard_t *card, nmea_parser_handle_t nmea_parser_handle)
{
    if(!(card->cardInit))
    {
        if(init_sdcard(card, "/sdcard") != ESP_OK)
            return;
    }
    sdCard_init = true;
    int count = 0;
    do{
        if(directoryLogs == NULL)
            snprintf(fileNameVario, sizeof(fileNameVario), "/Vario%02d.txt", count++);
        else
            snprintf(fileNameVario, sizeof(fileNameVario), "%s/Vario%02d.txt", directoryLogs, count++);
    }while(fileExists_sdcard(card, fileNameVario));
    ESP_LOGI(TAG, "Found file: %s", fileNameVario);
    snprintf(fileNameVario, sizeof(fileNameVario), "Vario%02d.txt", --count);
    fileVario = openFile_sdcard(card, directoryLogs, fileNameVario, true);
    
    count = 0;
    do{
        if(directoryLogs == NULL)
            snprintf(fileNameNMEA, sizeof(fileNameNMEA), "/NMEA%02d.txt", count++);
        else
            snprintf(fileNameNMEA, sizeof(fileNameNMEA), "/%s/NMEA%02d.txt", directoryLogs, count++);
    }while(fileExists_sdcard(card, fileNameNMEA));
    ESP_LOGI(TAG, "Found file: %s", fileNameNMEA);
    snprintf(fileNameNMEA, sizeof(fileNameNMEA), "NMEA%02d.txt", --count);
    fileNMEA = openFile_sdcard(card, directoryLogs, fileNameNMEA, true);
    sdcard = card;
    sd_card_state_t sdstate;
    device_state_get_member(&sdstate, device_state_statusSDCard, true);
    sdstate.status |= SD_CARD_STATE_FLIGHTLOGGING;
    device_state_set_member(&sdstate, device_state_statusSDCard);
    nmea_parser_add_handler(nmea_parser_handle, FlightLogger_NMEASentenceLogger, NULL, GPS_NMEA_SENTECE);
}

void endLogging()
{

}

void logVarioData(FlightLogger_t state)
{
    if(!sdCard_init)
        return;
    fwrite(&state, sizeof(FlightLogger_t), 1, fileVario); //write vario_state to file
    int64_t time = esp_timer_get_time(); 
    fwrite(&time, sizeof(int64_t), 1, fileVario); //write Time since startup to file
    fprintf(fileVario, "\n"); //new line
    if(countVarioFile % 20 == 0) //save File all 20th time ==> all 600 ms
    {
        fflush(fileVario);
    }
    countVarioFile++;
}