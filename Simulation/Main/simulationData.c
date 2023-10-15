#include "SimulationData.h"
#include "IGC_Parser.h"
#include "FlightLogger.h"
#include "nmea_parser.h"
#include "crc/crc.h"

#include <esp_err.h>
#include <esp_log.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <libgen.h>

const char *TAG = "SIMULATION";

static esp_event_handler_t gpsSubscriptions[10];
static size_t gpsHandlesAdded = 0;

static FlightLogger_t *simulationData;
static size_t simulationData_size;
static size_t currPosSimulationData;

static gps_t *simulationGPSData;
static size_t simulationGPSData_size;
static size_t currPosSimulationGPSData;

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

void GPSUpdateTask(void *arg)
{
    while (true)
    {
        gps_t *currGPS = simulationGPSData + currPosSimulationGPSData;
        for (size_t i = 0; i < gpsHandlesAdded; i++)
        {
            gpsSubscriptions[i](NULL, 0, GPS_UPDATE, currGPS);
        }
        currPosSimulationGPSData++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static esp_err_t startSimulationFromFile(char *varioName, char *gpsName)
{
    if (isInit || simulationData || simulationGPSData)
    {
        ESP_LOGE(TAG, "Already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    FILE *file;
    char filename[255];
    strncpy(filename, "VarioLogs/", 255);
    strncat(filename, varioName, 243);
    file = fopen(filename, "rb");
    if (!file)
    {
        ESP_LOGE(TAG, "Can't open Simulation file: %s", filename);
        return ESP_ERR_NOT_FOUND;
    }
    fseek(file, 0, SEEK_END);
    simulationData_size = ftell(file) / (37 + 1) + 1;
    simulationData = (FlightLogger_t *)calloc(sizeof(FlightLogger_t) * simulationData_size, 1);
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    size_t currDataPos = 0;
    while (ftell(file) < fileSize)
    {
        if (currDataPos >= simulationData_size)
        {
            ESP_LOGE(TAG, "To much data");
            break;
        }
        // handle Vario File
        FlightLogger_t *state = simulationData + currDataPos;
        fread(&state->acc, sizeof(float), 3, file);
        fread(&state->gyr, sizeof(float), 3, file);
        fread(&state->magn, sizeof(float), 3, file);
        fread(&state->pressure, sizeof(float), 1, file);
        state->temperature = 20;
        if (fgetc(file) == '\n')
            currDataPos++;
        else
        {
            char n = fgetc(file);
            if (n == -1)
                break;
            ESP_LOGE(TAG, "Not valid log data");
            while (1)
            {
                n = fgetc(file);
                if (n == '\n')
                {
                    continue;
                }
            }
        }
    }

    strncpy(filename, "VarioLogs/", 255);
    strncat(filename, gpsName, 243);
    file = fopen(filename, "r");
    fseek(file, 0, SEEK_END);
    simulationGPSData_size = ftell(file) / (51 + 1) + 1;
    simulationGPSData = (gps_t *)calloc(sizeof(gps_t) * simulationGPSData_size, 1);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    currDataPos = 0;
    while (ftell(file) < fileSize)
    {
        if (currDataPos >= simulationGPSData_size)
        {
            ESP_LOGE(TAG, "To much data GPS");
            continue;
        }
        gps_t *currGPS = simulationGPSData + currDataPos;
        // handle gps Data
        fread(&currGPS->tim.hour, sizeof(uint8_t), 1, file);
        fread(&currGPS->tim.minute, sizeof(uint8_t), 1, file);
        fread(&currGPS->tim.second, sizeof(uint8_t), 1, file);
        fread(&currGPS->tim.thousand, sizeof(uint16_t), 1, file);
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
        if (fgetc(file) == '\n')
            currDataPos++;
        else
        {
            char n = fgetc(file);
            if (n == -1)
                break;
            ESP_LOGE(TAG, "Not valid log data");
            while (1)
            {
                n = fgetc(file);
                if (n == '\n')
                {
                    continue;
                }
            }
        }
    }
    currPosSimulationData = 0;
    currPosSimulationGPSData = 0;
    isInit = true;
    return ESP_OK;
}

void simulation_start(char *varioname, char *gpsName)
{
    esp_err_t err = changeToProjectRoot();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Can't change to root directory of project");
        return;
    }
    startSimulationFromFile(varioname, gpsName);
    xTaskCreate(GPSUpdateTask, "GPS Eventloop", 1024 * 2, NULL, 10, NULL);
}
esp_err_t simulation_nextPressure(dps310_t *dps)
{
    if (!isInit || currPosSimulationData >= simulationData_size)
        return ESP_ERR_INVALID_STATE;
    FlightLogger_t currData = simulationData[currPosSimulationData];
    dps->Pressure = currData.pressure;
    dps->Temperature = dps->Temperature;
    currPosSimulationData++;
    return ESP_OK;
}
esp_err_t simulation_nextAcc(bmi088_t *bmi)
{
    if (!isInit || currPosSimulationData >= simulationData_size)
        return ESP_ERR_INVALID_STATE;
    FlightLogger_t currData = simulationData[currPosSimulationData];
    memcpy(bmi->acc_mps2, currData.acc, sizeof(float) * 3);
    memcpy(bmi->gyr_rps, currData.gyr, sizeof(float) * 3);
    return ESP_OK;
}
esp_err_t simulation_nextMagn(qmc588l_t *qmc)
{
    if (!isInit || currPosSimulationData >= simulationData_size)
        return ESP_ERR_INVALID_STATE;
    FlightLogger_t currData = simulationData[currPosSimulationData];
    memcpy(qmc->CorrectedMagnValuemG, currData.magn, sizeof(float) * 3);
    return ESP_OK;
}
esp_err_t simulation_addGPSHandle(esp_event_handler_t eventHandler)
{
    gpsSubscriptions[gpsHandlesAdded] = eventHandler;
    gpsHandlesAdded++;
    return ESP_OK;
}