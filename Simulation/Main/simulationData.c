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
#include <stdlib.h>
#include <string.h>

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
            if (strcmp(cwd, "/") == 0 || chdir("..") != 0)
                return ESP_ERR_INVALID_STATE;
            if (getcwd(cwd, sizeof(cwd)) == NULL)
                return ESP_ERR_INVALID_STATE;
            last_folder = basename(cwd);
        }
        return ESP_OK;
    }
    return ESP_ERR_INVALID_STATE;
}

void GPSUpdateTask(void *arg)
{
    while (currPosSimulationGPSData < simulationGPSData_size)
    {
        gps_t *currGPS = simulationGPSData + currPosSimulationGPSData;
        for (size_t i = 0; i < gpsHandlesAdded; i++)
        {
            gpsSubscriptions[i](NULL, 0, GPS_UPDATE, currGPS);
        }
        currPosSimulationGPSData++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static esp_err_t startSimulationFromFile(char *varioName, char *gpsName)
{
    if (isInit)
        return ESP_ERR_INVALID_STATE;
    FILE *file = fopen(varioName, "rb");
    if (!file)
        return ESP_ERR_NOT_FOUND;
    fseek(file, 0, SEEK_END);
    long bytes = ftell(file);
    rewind(file);
    /* Ten little-endian float32 values followed by LF. Never read a partial tail. */
    simulationData_size = bytes / 41;
    if (bytes % 41)
        ESP_LOGW(TAG, "Ignoring incomplete final sensor record (%ld bytes)", bytes % 41);
    simulationData = calloc(simulationData_size, sizeof(*simulationData));
    if (!simulationData || simulationData_size == 0) {
        fclose(file);
        return ESP_ERR_INVALID_STATE;
    }
    for (size_t i = 0; i < simulationData_size; ++i) {
        FlightLogger_t *state = &simulationData[i];
        if (fread(state->acc, sizeof(float), 3, file) != 3 ||
            fread(state->gyr, sizeof(float), 3, file) != 3 ||
            fread(state->magn, sizeof(float), 3, file) != 3 ||
            fread(&state->pressure, sizeof(float), 1, file) != 1 || fgetc(file) != '\n') {
            fclose(file);
            return ESP_ERR_INVALID_STATE;
        }
        state->temperature = 20;
    }
    fclose(file);
    file = fopen(gpsName, "rb");
    if (!file)
        return ESP_ERR_NOT_FOUND;
    fseek(file, 0, SEEK_END);
    bytes = ftell(file);
    rewind(file);
    /* Serialized GPS record: 52 bytes of fields plus LF (not sizeof(gps_t)). */
    simulationGPSData_size = bytes / 53;
    if (bytes % 53)
        ESP_LOGW(TAG, "Ignoring incomplete final GPS record (%ld bytes)", bytes % 53);
    simulationGPSData = calloc(simulationGPSData_size, sizeof(*simulationGPSData));
    if (!simulationGPSData || simulationGPSData_size == 0) {
        fclose(file);
        return ESP_ERR_INVALID_STATE;
    }
    for (size_t i = 0; i < simulationGPSData_size; ++i) {
        gps_t *gps = &simulationGPSData[i];
#define READ_FIELD(field) do { if (fread(&gps->field, sizeof(gps->field), 1, file) != 1) { fclose(file); return ESP_ERR_INVALID_STATE; } } while (0)
        READ_FIELD(tim.hour); READ_FIELD(tim.minute); READ_FIELD(tim.second);
        READ_FIELD(tim.thousand); READ_FIELD(fix); READ_FIELD(fix_mode); READ_FIELD(valid);
        READ_FIELD(dop_h); READ_FIELD(dop_p); READ_FIELD(dop_v);
        READ_FIELD(sats_in_use); READ_FIELD(sats_in_view);
        READ_FIELD(latitude); READ_FIELD(longitude); READ_FIELD(altitude);
        READ_FIELD(speed); READ_FIELD(cog); READ_FIELD(variation);
#undef READ_FIELD
        if (fgetc(file) != '\n') {
            fclose(file);
            return ESP_ERR_INVALID_STATE;
        }
    }
    fclose(file);
    currPosSimulationData = currPosSimulationGPSData = 0;
    isInit = true;
    return ESP_OK;
}

void simulation_start(char *varioname, char *gpsName)
{
    esp_err_t err = changeToProjectRoot();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Can't change to root directory of project");
        exit(EXIT_FAILURE);
    }
    if (startSimulationFromFile(varioname, gpsName) != ESP_OK) {
        ESP_LOGE(TAG, "Invalid or missing replay recording");
        exit(EXIT_FAILURE);
    }
    xTaskCreate(GPSUpdateTask, "GPS Eventloop", 1024 * 2, NULL, 10, NULL);
}
esp_err_t simulation_nextPressure(dps310_t *dps)
{
    if (!isInit)
        return ESP_ERR_INVALID_STATE;
    if (currPosSimulationData >= simulationData_size) {
        ESP_LOGI(TAG, "Replay complete");
        exit(EXIT_SUCCESS);
    }
    FlightLogger_t currData = simulationData[currPosSimulationData];
    dps->Pressure = currData.pressure;
    dps->Temperature = currData.temperature;
    currPosSimulationData++;
    return ESP_OK;
}
esp_err_t simulation_nextAcc(bmi088_t *bmi)
{
    if (!isInit || currPosSimulationData > simulationData_size)
        return ESP_ERR_INVALID_STATE;
    FlightLogger_t currData = simulationData[currPosSimulationData ? currPosSimulationData - 1 : 0];
    memcpy(bmi->acc_mps2, currData.acc, sizeof(float) * 3);
    memcpy(bmi->gyr_rps, currData.gyr, sizeof(float) * 3);
    return ESP_OK;
}
esp_err_t simulation_nextMagn(qmc588l_t *qmc)
{
    if (!isInit || currPosSimulationData > simulationData_size)
        return ESP_ERR_INVALID_STATE;
    FlightLogger_t currData = simulationData[currPosSimulationData ? currPosSimulationData - 1 : 0];
    memcpy(qmc->CorrectedMagnValuemG, currData.magn, sizeof(float) * 3);
    return ESP_OK;
}
esp_err_t simulation_addGPSHandle(esp_event_handler_t eventHandler)
{
    if (gpsHandlesAdded >= sizeof(gpsSubscriptions) / sizeof(gpsSubscriptions[0]))
        return ESP_ERR_INVALID_STATE;
    gpsSubscriptions[gpsHandlesAdded] = eventHandler;
    gpsHandlesAdded++;
    return ESP_OK;
}
