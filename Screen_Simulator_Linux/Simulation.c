#include "Simulation.h"
#include "main.h"
#include "Screen/E_Ink/GUI_Paint.h"
#include "Screen/IGC_Parser.h"
#include <errno.h>
#include "string.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <unistd.h>
#include <libgen.h>
#include "Screen/Util.h"
#include "Screen/movingAverage.h"

size_t vario_state_Simulation_size;
vario_state_Simulation *bufferSimulation;
Uint32 lastbufferTime = 0;
Uint32 lastFrameTime = 0;
static size_t currPosStateBuffer = 0;
bool isIGCSim = FALSE;
movingAverage_t avg_VertVel;

extern void device_state_gps_handle(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

static void changeToProjectRoot()
{
    char cwd[500];
    // Change to directory, where the Textfile is (Screen_Simulator_Linux folder)
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        // printf("Current working dir: %s\n", cwd);
        char *last_folder = basename(cwd);
        while (strcmp(last_folder, "Screen_Simulator_Linux") != 0)
        {
            if (chdir("..") != 0)
                return;
            if (getcwd(cwd, sizeof(cwd)) == NULL)
                return;
            last_folder = basename(cwd);
        }
    }
}

static void startIGCSimulation(char *name)
{
    char Filename[1000];
    strncpy(Filename, "IGCFlights/", 100);
    strcat(Filename, name);
    FILE *fileptr;
    fileptr = fopen(Filename, "r");
    if (fileptr == NULL)
    {
        int error = errno;
        printf("Error %d \n", error);
        return;
    }
    Track_t track;
    memset(&track, 0, sizeof(Track_t));
    if (ParseIGCFile(&track, 500000000, fileptr, false, false, name) != 0)
    {
        printf("Failed to parse IGC File\n");
        fclose(fileptr);
        return;
    }
    fclose(fileptr);
    vario_state_Simulation_size = track.sizeTrackPoints;
    bufferSimulation = (vario_state_Simulation *)malloc(track.sizeTrackPoints * sizeof(vario_state_Simulation));
    for (size_t i = 0; i < track.sizeTrackPoints; i++)
    {
        TrackPointIGC_t point = track.trackPoints[i];
        bufferSimulation[i].presHeight = point.pressureAlt;
        bufferSimulation[i].gpsHeight = point.gnssAlt;
        // Update all second -> to microseconds
        bufferSimulation[i].time = i * 1000000;
        bufferSimulation[i].lat = point.lat;
        bufferSimulation[i].lon = point.lon;
        bufferSimulation[i].velocity = 0;
        bufferSimulation[i].accX = 9.81;
    }
    FreeTrackFile(&track);
}

static void startFlightLoggerSimulation(char *name)
{
    FILE *fileptr;
    size_t bufferSize;
    fileptr = fopen("Trackloggs/Vario46.txt", "rb"); // Open the file in binary mode
    if (fileptr == NULL)
    {
        int error = errno;
        printf("Error %d \n", error);
        return;
    }
    fseek(fileptr, 0, SEEK_END); // Jump to the end of the file
    bufferSize = ftell(fileptr); // Get the current byte offset in the file
    rewind(fileptr);             // Jump back to the beginning of the file
    // fill vario_state with value
    vario_state_Simulation_size = bufferSize / sizeof(vario_state_file_t);
    bufferSimulation = (vario_state_Simulation *)malloc(vario_state_Simulation_size * sizeof(vario_state_Simulation));
    // size_t currPos = 0;
    int currPosVarioSimulation = 0;
    char newLine = '\n';
    while (newLine == '\n' && currPosVarioSimulation < vario_state_Simulation_size)
    {
        vario_state_file_t vario_state;
        fread((void *)&vario_state, sizeof(vario_state_file_t), 1, fileptr);
        // currPos += sizeof(vario_state_file_t);
        int64_t time;
        fread((void *)&time, sizeof(int64_t), 1, fileptr);
        fread((void *)&newLine, sizeof(char), 1, fileptr);
        // currPos += sizeof(int64_t);
        // currPos += sizeof(char);
        bufferSimulation[currPosVarioSimulation].presHeight = vario_state.height;
        bufferSimulation[currPosVarioSimulation].velocity = vario_state.velocity;
        bufferSimulation[currPosVarioSimulation].accX = vario_state.accX;
        bufferSimulation[currPosVarioSimulation].rotation[0] = vario_state.rotation[0];
        bufferSimulation[currPosVarioSimulation].rotation[1] = vario_state.rotation[1];
        bufferSimulation[currPosVarioSimulation].rotation[2] = vario_state.rotation[2];
        bufferSimulation[currPosVarioSimulation].rotation[3] = vario_state.rotation[3];
        bufferSimulation[currPosVarioSimulation].time = time;
        bufferSimulation[currPosVarioSimulation].lat = 0;
        bufferSimulation[currPosVarioSimulation].lon = 0;
        currPosVarioSimulation++;
    }
    printf("Has found %i vario states", (int)currPosVarioSimulation);
}

static void printToPythonArray()
{
    FILE *pythonArrayF = fopen("pythonArray", "w");
    fprintf(pythonArrayF, "y=[");
    double sum;
    double lastAcc;
    double maxAccD = 0;
    for (size_t i = 0; i < vario_state_Simulation_size; i++)
    {
        float val = bufferSimulation[i].presHeight;
        if (i != 0)
            fprintf(pythonArrayF, ",");
        fprintf(pythonArrayF, "%.2f", val);
        if (i > 0)
        {
            sum += fabs(lastAcc - val);
            if (fabsf(lastAcc - val) > maxAccD)
            {
                maxAccD = fabsf(lastAcc - val);
            }
        }
        lastAcc = bufferSimulation[i].accX;
    }
    fprintf(pythonArrayF, "]\n\n\n");
    fprintf(pythonArrayF, "x=[");
    for (size_t i = 0; i < vario_state_Simulation_size; i++)
    {
        if (i != 0)
            fprintf(pythonArrayF, ",");
        fprintf(pythonArrayF, "%i", (int)i);
    }
    fprintf(pythonArrayF, "]");
    fclose(pythonArrayF);
}

static void startHeightDiagramm()
{
    const int x = 10;
    const int y = 320;
    const int width = 460;
    const int height = 80;
    Paint_DrawRectangle(x, y, x + width, y + height, BLACK, DOT_PIXEL_1X1, FALSE);
    float ratio = (float)vario_state_Simulation_size / (float)width;
    int pixelDraw = 0;
    float maxHeight = 0;
    float minHeight = 0;
    for (size_t i = 0; i < vario_state_Simulation_size; i++)
    {
        float heightV = bufferSimulation[i].presHeight;
        if (heightV > maxHeight && (fabsf(heightV - maxHeight) < 10 || i == 0))
            maxHeight = heightV;
        else if (heightV > maxHeight)
            printf("NOW\n");
        else if (heightV < minHeight)
            minHeight = heightV;
    }

    maxHeight += 50;
    minHeight -= 50;
    size_t i = 0;
    while (i < vario_state_Simulation_size && pixelDraw <= width)
    {
        float percentageFromHeight = ((float)(bufferSimulation[(int)roundf(i)].presHeight - minHeight)) / (float)(maxHeight - minHeight);
        float heightY = (uint16_t)((1 - percentageFromHeight) * (float)height) + y;
        Paint_DrawPoint(x + pixelDraw, heightY, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);
        pixelDraw++;
        i += ratio;
    }
}

static void startRenderTimeProgress()
{
    Paint_DrawLine(0, 281, 480, 281, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    const int x = 20;
    const int y = 290;
    const int width = 440;
    const int height = 20;
    Paint_DrawRectangle(x, y, x + width, y + height, BLACK, DOT_PIXEL_1X1, FALSE);
    Paint_DrawRectangle(x + 2, y + 2, x + width - 2, y + height - 2, WHITE, DOT_PIXEL_1X1, WHITE);
}

void simulation_init(bool igc, char *fileName)
{
    isIGCSim = igc;
    changeToProjectRoot();
    if (igc)
        startIGCSimulation(fileName);
    else
        startFlightLoggerSimulation(fileName);
    startRenderTimeProgress();
    startHeightDiagramm();
    movingAverage_init(&avg_VertVel, 20);
}

static void updateRenderTimeProgress(float percentage)
{
    static int i = 0;
    if (i % 10 == 0)
    {
        startRenderTimeProgress();
        startHeightDiagramm(vario_state_Simulation_size, bufferSimulation);
    }
    const int x = 20;
    const int y = 290;
    const int width = 440;
    const int height = 20;
    Paint_DrawRectangle(x + 2, y + 2, x + width - 2, y + height - 2, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(x + 2, y + 2, x + width * percentage - 2, y + height - 2, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
}

static void UpdateIGCSim()
{
    vario_state_Simulation currPoint = bufferSimulation[currPosStateBuffer + 1];
    gps_t gps = {
        .altitude = currPoint.gpsHeight,
        .latitude = currPoint.lat,
        .longitude = currPoint.lon,
        .fix = GPS_FIX_DGPS,
        .fix_mode = GPS_MODE_3D,
        .valid = true,
    };
    float speed;
    int bearing;
    if (currPosStateBuffer < vario_state_Simulation_size - 1)
    {
        vario_state_Simulation nextPoint = bufferSimulation[currPosStateBuffer];
        bearing = GET_BEARING(currPoint.lat, currPoint.lon, nextPoint.lat, nextPoint.lon);
        speed = GET_DISTANCEFAST(currPoint.lat, currPoint.lon, nextPoint.lat, nextPoint.lon);
        float vario = currPoint.presHeight - nextPoint.presHeight;
        vario_state_set_member(&vario, vario_state_vertVel);
        vario_state_set_member(&bearing, vario_state_heading);
        gps.speed = speed;
    }
    device_state_gps_handle(NULL, 0, GPS_UPDATE, &gps);
}

void simulation_nextStep()
{
    Uint32 current_time = SDL_GetTicks();
    Uint32 time_since_last_frame = current_time - lastFrameTime;
    lastFrameTime = current_time;
    if (currPosStateBuffer < vario_state_Simulation_size)
    {
        if (lastbufferTime == 0)
        {
            lastbufferTime = bufferSimulation[currPosStateBuffer].time;
            currPosStateBuffer++;
        }
        else
        {
            lastbufferTime += time_since_last_frame * 1000;
            if (lastbufferTime >= bufferSimulation[currPosStateBuffer].time)
            {
                vario_state_Simulation currPoint = bufferSimulation[currPosStateBuffer];
                vario_state_set_member(&currPoint.velocity, vario_state_vertVel);
                float accX = currPoint.accX - 9.81;
                vario_state_set_member(&currPoint.presHeight, vario_state_altitudeBaro);
                vario_state_set_member(&accX, vario_state_vertAcc);
                vario_state_set_member(&currPoint.gpsHeight, vario_state_altitudeGPS);
                if (isIGCSim)
                    UpdateIGCSim();
                currPosStateBuffer++;
            }
        }
    }
    updateRenderTimeProgress((float)currPosStateBuffer / (float)vario_state_Simulation_size);
}

void simulation_fastForward(float percentage)
{
    int step = vario_state_Simulation_size * percentage;
    currPosStateBuffer += step;
    if (currPosStateBuffer > vario_state_Simulation_size)
    {
        currPosStateBuffer = vario_state_Simulation_size;
    }
    lastbufferTime = bufferSimulation[currPosStateBuffer].time;
    updateRenderTimeProgress((float)currPosStateBuffer / (float)vario_state_Simulation_size);
}
