#include "SimulationData.h"
#include "IGC_Parser.h"
#include "FlightLogger.h"
#include "nmea_parser.h"

#include <esp_err.h>
#include <esp_log.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <libgen.h>

const char *TAG = "SIMULATION";

simulation_type_t currSimulationType;

FlightLogger_t *simulationData;
size_t simulationData_size;

gps_t *simulationGPSData;
size_t simulationGPSData_size;

bool isInit = false;

static esp_err_t changeToProjectRoot()
{
    char cwd[500];
    // Change to directory, where the Textfile is (Main folder)
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        char *last_folder = basename(cwd);
        while (strcmp(last_folder, "Main") != 0)
        {
            if (chdir("..") != 0)
                return ESP_ERR_INVALID_STATE;
            if (getcwd(cwd, sizeof(cwd)) == NULL)
                return ESP_ERR_INVALID_STATE;
            last_folder = basename(cwd);
        }
        return ESP_OK;
    }
}

static esp_err_t startSerialLogSimulation(char *name)
{
    if (isInit || simulationData || simulationGPSData)
    {
        ESP_LOGE(TAG, "Already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    FILE *file;
    char filename[255];
    strncpy(filename, "VarioLogs/", 255);
    strncat(filename, name, 243);
    file = fopen(filename, "r");
    if (!file)
    {
        ESP_LOGE(TAG, "Can't open Simulation file: %s", filename);
        return ESP_ERR_NOT_FOUND;
    }
    char *line = NULL;
    size_t len = 0;
    size_t readFile;
    simulationData_size = 0;
    simulationGPSData_size = 0;
    while ((readFile = getline(&line, &len, file)) != -1)
    {
        if (line[0] == 'd')
            simulationData_size++;
        else if (line[0] == 'g')
            simulationGPSData_size++;
        // else
        //     ESP_LOGE(TAG, "This line is not a valid log");
    }
    ESP_LOGI(TAG, "Simulation started with Datasize: %i, GPSSize: %i", simulationData_size, simulationGPSData_size);
    simulationData = (FlightLogger_t *)malloc(sizeof(FlightLogger_t) * simulationData_size);
    simulationGPSData = (gps_t *)malloc(sizeof(gps_t) * simulationGPSData_size);
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    size_t currDataPos = 0;
    size_t currGPSPos = 0;
    while (ftell(file) < fileSize)
    {
        char startChar = fgetc(file);
        char endChar = '0';
        if (startChar == 'd')
        {
            // handle Vario File
            fread(simulationData + currDataPos, sizeof(FlightLogger_t), 1, file);
            endChar = fgetc(file);
            FlightLogger_t *local = simulationData + currDataPos;
            currDataPos++;
        }
        else if (startChar == 'g')
        {
            gps_t *currGPS = simulationGPSData + currGPSPos;
            fread(&currGPS->tim.hour, sizeof(uint8_t), 1, file);
            // handle gps Data
            fread(&currGPS->tim.hour, sizeof(uint8_t), 1, file);
            fread(&currGPS->tim.minute, sizeof(uint8_t), 1, file);
            fread(&currGPS->tim.second, sizeof(uint8_t), 1, file);
            fread(&currGPS->tim.thousand, sizeof(uint8_t), 1, file);
            fread(&currGPS->fix, sizeof(gps_fix_t), 1, file);
            fread(&currGPS->fix_mode, sizeof(gps_fix_mode_t), 1, file);
            fread(&currGPS->valid, sizeof(bool), 1, file);
            fread(&currGPS->dop_h, sizeof(float), 1, file);
            fread(&currGPS->dop_p, sizeof(float), 1, file);
            fread(&currGPS->dop_v, sizeof(float), 1, file);
            fread(&currGPS->sats_in_use, sizeof(uint8_t), 1, file);
            fread(&currGPS->sats_in_view, sizeof(uint8_t), 1, file);
            fread(&currGPS->latitude, sizeof(float), 1, file);
            fread(&currGPS->longitude, sizeof(float), 1, file);
            fread(&currGPS->altitude, sizeof(float), 1, file);
            fread(&currGPS->speed, sizeof(float), 1, file);
            fread(&currGPS->cog, sizeof(float), 1, file);
            fread(&currGPS->variation, sizeof(float), 1, file);
            endChar = fgetc(file);
            currGPSPos++;
        }
        else
        {
            ESP_LOGE(TAG, "This line is not a valid log line");
        }
        if (endChar == '\r')
            endChar = fgetc(file);
        if (endChar != '\n')
            ESP_LOGE(TAG, "Not valied line ending!!");
    }
    return ESP_OK;
}

void simulation_start(char *name, simulation_type_t simulationType)
{
    currSimulationType = simulationType;
    esp_err_t err = changeToProjectRoot();
    if (err != ESP_OK)
    {
        ESP_LOGE("SIMULATION", "Can't change to root directory of project");
        return;
    }
    switch (currSimulationType)
    {
    case SIMULATION_SERIAL:
        startSerialLogSimulation(name);
        break;
    default:
        break;
    }
}
esp_err_t simulation_nextPressure(float *pressure)
{
}
esp_err_t simulation_nextAcc(float *acc)
{
}
esp_err_t simulation_addGPSHandle(esp_event_handler_t eventHandler)
{
}