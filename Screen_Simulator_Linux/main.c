#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "Screen/E_Ink/GUI_Paint.h"
#include "UI_Demo.h"
#include "Screen/DeviceContext.h"
#include "main.h"
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include "Screen/IGC_Parser.h"

#define EPD_3IN7_WIDTH       410
#define EPD_3IN7_HEIGHT      480 

Uint32 lastbufferTime = 0;
Uint32 lastFrameTime = 0;
Uint32 lastRenderTime = 0;
Uint32 startedFKeyTime = 0;
bool startedFKey = false;
int currPosStateBuffer = 0;

UBYTE* ScreenBuffer;

size_t vario_state_Simulation_size;
vario_state_Simulation* bufferSimulation;

void open_ReadFile()
{
    char cwd[5000];
    //Change to directory, where the Textfile is (Screen_Simulator_Linux folder)
    if (getcwd(cwd, sizeof(cwd)) != NULL) 
    {
        //printf("Current working dir: %s\n", cwd);
        char* last_folder = basename(cwd);
        while(strcmp(last_folder, "Screen_Simulator_Linux") != 0)
        {
            if (chdir("..") != 0)
                return;
            if (getcwd(cwd, sizeof(cwd)) == NULL)
                return;
            last_folder = basename(cwd);
        }
    }

	FILE* fileptr;
	fileptr = fopen("IGC/IGCbsLKve.IGC", "r");
    if (fileptr == NULL)
    {
        int error = errno; 
        printf("Error %d \n", error);
        return;
    }
    Track_t track;
    if(ParseIGCFile(&track, 50, fileptr) != 0) {
        printf("Failed to parse IGC File\n");
    }
    fclose(fileptr);
    fileptr = fopen("GPSLOG.txt", "w");
    fprintf(fileptr, "latitude,longitude\n");
    for (size_t i = 0; i < track.sizeTrackPoints; i++)
    {
        fprintf(fileptr ,"%.6f,%.6f\n", track.trackPoints[i].lat, track.trackPoints[i].lon);
    }
    
    fclose(fileptr);
	size_t bufferSize;
	fileptr = fopen("Vario10.txt", "rb");  // Open the file in binary mode
    if (fileptr == NULL)
    {
        int error = errno; 
        printf("Error %d \n", error);
        return;
    }
	fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	bufferSize = ftell(fileptr);             // Get the current byte offset in the file
	rewind(fileptr);                      // Jump back to the beginning of the file
	uint8_t* bufferRead = (uint8_t*)malloc(bufferSize * sizeof(uint8_t)); // Enough memory for the file
	fread(bufferRead, bufferSize, 1, fileptr); // Read in the entire file
	fclose(fileptr); // Close the file
	//fill vario_state with value
	vario_state_Simulation_size = bufferSize / 37;
	bufferSimulation = (vario_state_Simulation*)malloc(vario_state_Simulation_size * sizeof(vario_state_Simulation));
	size_t currPos = 0;
	int currPosVarioSimulation = 0;
	while (currPos != bufferSize)
	{
		vario_state_file_t* vario_state = (vario_state_file_t*)&bufferRead[currPos];;
		currPos += sizeof(vario_state_file_t);
		int64_t* time = (int64_t*)&bufferRead[currPos];
		currPos += sizeof(int64_t);
		currPos += sizeof(char);
		bufferSimulation[currPosVarioSimulation].height = vario_state->height;
		bufferSimulation[currPosVarioSimulation].velocity = vario_state->velocity;
		bufferSimulation[currPosVarioSimulation].accX = vario_state->accX;
		bufferSimulation[currPosVarioSimulation].rotation[0] = vario_state->rotation[0];
		bufferSimulation[currPosVarioSimulation].rotation[1] = vario_state->rotation[1];
		bufferSimulation[currPosVarioSimulation].rotation[2] = vario_state->rotation[2];
		bufferSimulation[currPosVarioSimulation].rotation[3] = vario_state->rotation[3];
		bufferSimulation[currPosVarioSimulation].time = *time;
		currPosVarioSimulation++;
	}
	free(bufferRead);
}

void fastForwardSimulation(float percentage)
{
    int step = vario_state_Simulation_size * percentage;
    currPosStateBuffer += step;
    if(currPosStateBuffer > vario_state_Simulation_size)
    {
        currPosStateBuffer = vario_state_Simulation_size;
    }
    lastbufferTime = bufferSimulation[currPosStateBuffer].time;
}

int main(void) {
    SDL_Renderer *renderer;
    SDL_Window *window;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(EPD_3IN7_HEIGHT, EPD_3IN7_WIDTH, 0, &window, &renderer);
    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // for (i = 0; i < EPD_3IN7_WIDTH; ++i)
    //     SDL_RenderDrawPoint(renderer, i, i);

    UWORD Imagesize = ((EPD_3IN7_WIDTH % 4 == 0)? (EPD_3IN7_WIDTH / 4 ): (EPD_3IN7_WIDTH / 4 + 1)) * EPD_3IN7_HEIGHT;
    if ((ScreenBuffer = (UBYTE*)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for Screen memory...\r\n");
        return 1;
    }
    printf("Paint_NewImage \r\n");
    Paint_NewImage(ScreenBuffer, EPD_3IN7_WIDTH, EPD_3IN7_HEIGHT, 270, WHITE);
    Paint_SelectImage(ScreenBuffer);
    Paint_SetScale(2);
    Paint_Clear(WHITE);
    open_ReadFile();
    startRender();
    startRenderTimeProgress();
    startHeightDiagramm(vario_state_Simulation_size, bufferSimulation);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    int sizeXBuffer = EPD_3IN7_WIDTH % 8 == 0 ? EPD_3IN7_WIDTH / 8 : EPD_3IN7_WIDTH / 8 + 1;
    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_DOWN:
                            startDownArrow();
                            break;
                        case SDLK_UP:
                            startUpArrow();
                            break;
                        case SDLK_m:
                            startMenuKey();
                            break;
                        case SDLK_RETURN:
                            startEnterKey();
                            break;
                        case SDLK_p:
                            PKeyPressed();
                            break;
                        case SDLK_f:
                            if (!startedFKey)
                            {
                                startedFKeyTime = SDL_GetTicks();
                                startedFKey = true;
                            }
                            else
                            {
                                fastForwardSimulation(0.005f);
                            }
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_DOWN:
                            DownArrowReleased();
                            break;
                        case SDLK_UP:
                            UpArrowReleased();
                            break;
                        case SDLK_m:
                            MenuKeyReleased();
                            break;
                        case SDLK_RETURN:
                            EnterKeyReleased();
                            break;
                        case SDLK_f:
                            startedFKey = false;
                            break;
                    }
                    break;
                // Handle other event types as needed
            }
        }

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
                    vario_state_set_member(&bufferSimulation[currPosStateBuffer].velocity, vario_state_vertVel);
                    float accX = bufferSimulation[currPosStateBuffer].accX - 9.81;
                    vario_state_set_member(&bufferSimulation[currPosStateBuffer].height, vario_state_altitudeBaro);
                    vario_state_set_member(&accX, vario_state_vertAcc);
                    currPosStateBuffer++;
                }
            }
        }
        updateRender();
        updateRenderTimeProgress((float)currPosStateBuffer / (float)vario_state_Simulation_size, vario_state_Simulation_size, bufferSimulation);
        for (int y = 0; y < EPD_3IN7_HEIGHT; y++)
	    {
            for (int x = 0; x < EPD_3IN7_WIDTH; x++)
            {
                int addr = x / 8 + y * sizeXBuffer;
                uint8_t rdata = ScreenBuffer[addr];
                //Black
                if ((rdata & (0x80 >> (x % 8))) == 0)
                {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    SDL_RenderDrawPoint(renderer, EPD_3IN7_HEIGHT -1 - y, x);
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
                    SDL_RenderDrawPoint(renderer, EPD_3IN7_HEIGHT - 1 - y, x);
                }
            }
	    }
        if(current_time - lastRenderTime > 300)
        {
            SDL_RenderPresent(renderer);
            lastRenderTime = current_time;
        }
        SDL_Delay(5);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}