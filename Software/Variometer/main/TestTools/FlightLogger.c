#include "FlightLogger.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "nmea_parser.h"
#include <string.h>
#include "DeviceContext.h"
#include "vario_config.h"

bool sdCard_init = false;
static const char *TAG = "FlightLogger";
char fileNameVario[40];
char fileNameNMEA[40];
char fileNameGPSUpdate[40];
FILE *fileVario;
FILE *fileNMEA;
FILE *fileGPSUpdate;
int countVarioFile;
sdcard_t *sdcard;
nmea_parser_handle_t nmea_parser_hndl;
const char *directoryLogs = "/Trackloggs";

void FlightLogger_NMEASentenceLogger(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    int64_t time;
    static int count = 0;
    gps_t* gps;
    switch (event_id) {
    #ifdef NotifyNMEASentence
    case GPS_NMEA_SENTECE:
        //ESP_LOGI(TAG, "NMEA Sentence:%s", (char *)event_data);
        time = esp_timer_get_time();
        fwrite(&time, sizeof(time), 1, fileNMEA); //write time to file
        fprintf(fileNMEA, "%s\n", (char*)event_data); //write NMEA Sentence and new line to file
        if(count % 20 == 0)
            fflush(fileNMEA);
        break;
    #endif
    case GPS_UPDATE:
        gps = (gps_t *)event_data; 
        time = esp_timer_get_time();
        fwrite(&time, sizeof(time), 1, fileGPSUpdate); //write time to file
        fwrite(gps, sizeof(gps_t), 1, fileGPSUpdate); //write gps data to file
        if(count % 20 == 0)
            fflush(fileGPSUpdate);
    default:
        break;
    }
    count++;
}

void startLogging()
{
    if(!(sdcard->cardInit))
    {
        if(init_sdcard(sdcard, "/sdcard") != ESP_OK)
            return;
    }
    int count = 0;
    do{
        snprintf(fileNameVario, sizeof(fileNameVario), "Vario%02d.txt", count++);
    }while(fileExists_sdcard(sdcard, directoryLogs, fileNameVario));
    ESP_LOGI(TAG, "Found file: %s", fileNameVario);
    //snprintf(fileNameVario, sizeof(fileNameVario), "Vario%02d.txt", --count);
    fileVario = openFile_sdcard(sdcard, directoryLogs, fileNameVario, "wb");
    
    #ifdef NotifyNMEASentence
    count = 0;
    do{
        snprintf(fileNameNMEA, sizeof(fileNameNMEA), "NMEA%02d.txt", count++);
    }while(fileExists_sdcard(sdcard, directoryLogs, fileNameNMEA));
    ESP_LOGI(TAG, "Found file: %s", fileNameNMEA);
    fileNMEA = openFile_sdcard(sdcard, directoryLogs, fileNameNMEA, true);
    #endif

    count = 0;
    do{
        snprintf(fileNameGPSUpdate, sizeof(fileNameGPSUpdate), "GPS%02d.txt", count++);
    }while(fileExists_sdcard(sdcard, directoryLogs, fileNameGPSUpdate));
    ESP_LOGI(TAG, "Found file: %s", fileNameGPSUpdate);
    fileGPSUpdate = openFile_sdcard(sdcard, directoryLogs, fileNameGPSUpdate, "wb");

    sd_card_state_t sdstate;
    device_state_get_member(&sdstate, device_state_statusSDCard, false);
    sdstate.status |= SD_CARD_STATE_FLIGHTLOGGING;
    device_state_set_member(&sdstate, device_state_statusSDCard);
    sdCard_init = true;
    #ifdef NotifyNMEASentence
    nmea_parser_add_handler(nmea_parser_hndl, FlightLogger_NMEASentenceLogger, NULL, GPS_NMEA_SENTECE);
    #endif
    nmea_parser_add_handler(nmea_parser_hndl, FlightLogger_NMEASentenceLogger, NULL, GPS_UPDATE);
}

void initLogging(sdcard_t* card, nmea_parser_handle_t nmea_parser_handle)
{
    sdcard = card;
    nmea_parser_hndl = nmea_parser_handle;
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
        // fclose(fileVario);
        // fileVario = openFile_sdcard(sdcard, directoryLogs, fileNameVario, true);
    }
    countVarioFile++;
}