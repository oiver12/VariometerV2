#include "FlightLogger.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "nmea_parser.h"
#include <string.h>

static const char *TAG = "FlightLogger";
char fileNameVario[40];
char fileNameNMEA[40];
FILE *fileVario;
FILE *fileNMEA;
int countVarioFile;
sdcard_t *sdcard;
const char *directoryLogs = NULL;

static void NMEASentenceLogger(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id) {
    case GPS_NMEA_SENTECE:
        ESP_LOGI(TAG, "NMEA Sentence:%s", (char *)event_data);
        int64_t time = esp_timer_get_time();
        fwrite(&time, sizeof(time), 1, fileNMEA); //write time to file
        fprintf(fileNMEA, "%s\n", (char*)event_data); //write NMEA Sentence and new line to file
        fclose(fileNMEA);
        fileNMEA = openFile_sdcard(sdcard, directoryLogs, fileNameNMEA, false);
        break;
    default:
        break;
    }
}

void startLogging(sdcard_t *card)
{
    if(!(card->cardInit))
    {
        init_sdcard(card, "/sdcard");
    }

    int count = 0;
    do{
        if(directoryLogs == NULL)
            snprintf(fileNameVario, sizeof(fileNameVario), "/Vario%02d.txt", count++);
        else
            snprintf(fileNameVario, sizeof(fileNameVario), "/%s/Vario%02d.txt", directoryLogs, count++);
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
}

void endLogging()
{

}

void logVarioData(vario_state_t state, float rotation[4])
{
    fwrite(&state, sizeof(vario_state_t), 1, fileVario); //write vario_state to file
    fwrite(rotation, sizeof(float), 4, fileVario); //write roation of Vario to file
    int64_t time = esp_timer_get_time(); 
    fwrite(&time, sizeof(int64_t), 1, fileVario); //write Time since startup to file
    fprintf(fileVario, "\n"); //new line
    if(countVarioFile % 10 == 0)
    {
        fclose(fileVario);
        fileVario = openFile_sdcard(sdcard, directoryLogs, fileNameVario, false);
        //printf("State: %.2f, %.2f, %.2f, %lli\n", state.height, state.velocity, state.accX, time);
    }
    countVarioFile++;
}