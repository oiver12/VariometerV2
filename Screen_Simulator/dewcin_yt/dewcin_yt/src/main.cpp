#include "dewcin.h"
#include <time.h>
#include <filesystem>
#include <iostream>

extern "C" {
	#include "Screen/DeviceContext.h"
	#include "Screen/Vario_Screen.h"

	extern void updateRender();
	extern void startUpArrow();
	extern void UpArrowReleased();
	extern void startDownArrow();
	extern void DownArrowReleased();
	extern void startEnterKey();
	extern void EnterKeyReleased();
	extern void startMenuKey();
	extern void MenuKeyReleased();
}

typedef struct
{
	float height;
	float velocity;
	float accX;
	float rotation[4];
}vario_state_file_t;

typedef struct
{
	float height;
	float velocity;
	float accX;
	float rotation[4];
	long long time;
} vario_state_Simulation;

size_t vario_state_Simulation_size;
vario_state_Simulation* bufferSimulation;

void open_ReadFile()
{
	TCHAR path[MAX_PATH + 1] = L"";
	DWORD len = GetCurrentDirectory(MAX_PATH, path);
	
	FILE* fileptr;
	
	size_t bufferSize;
	fileptr = fopen("Vario08.txt", "rb");  // Open the file in binary mode
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
	for (size_t i = 0; i < vario_state_Simulation_size; i++)
	{
		vario_state_Simulation temp = bufferSimulation[i];
		int test1 = 0;
		test1++;
	}
	free(bufferRead);
}

dewcin_app_entry_point
{
	// game init code
	int width = 480;
	int height = 280;
	dewcin::Game::setGameUpdate([&](float delta)
	{
		wchar_t charBuffer[256];
		//swprintf(charBuffer, 256, L"delta: %f\n", delta);
		//OutputDebugString(charBuffer);

		static int frames = 0;
		static float timePassed = 0.0f;
		static float vertVel = 0.0f;

		static int currPos = 0;
		static int64_t lastTime = 0;
		
		if (currPos < vario_state_Simulation_size)
		{
			if (lastTime == 0)
			{
				lastTime = bufferSimulation[currPos].time;
				currPos++;
			}
			else
			{
				lastTime += delta * 1000000;
				if (lastTime >= bufferSimulation[currPos].time)
				{
					vario_state_set_member(&bufferSimulation[currPos].velocity, vario_state_vertVel);
					float accX = bufferSimulation[currPos].accX - 9.81;
					vario_state_set_member(&bufferSimulation[currPos].height, vario_state_altitudeBaro);
					vario_state_set_member(&accX, vario_state_vertAcc);
					currPos++;
				}
			}
		}

		frames++;
		timePassed += delta;

		if (timePassed >= 1.0f)
		{
			swprintf(charBuffer, 256, L"FPS: %d\n", frames);
			OutputDebugString(charBuffer);

			timePassed -= 1.0f;
			frames = 0;
			vertVel++;
			//vario_state_set_member(&vertVel, vario_state_vertVel);
			//float altitude = sin(vertVel / 5) * 20;
			//vario_state_set_member(&altitude, vario_state_altitudeBaro);
			//float percentage = (float)((int)vertVel % 100) / 100.0f;
			//device_state_set_member(&percentage, device_state_batteryPercentage);
			//int heading = (int)vertVel * 20;
			//vario_state_set_member(&heading, vario_state_heading);
			if ((int)vertVel % 5 == 0)
			{
				sd_card_state_t state;
				device_state_get_member(&state, device_state_statusSDCard, true);
				state.freeSpace = 2258000000;
				state.status = (SD_CARD_STATE)(state.status | SD_CARD_STATE_FLIGHTLOGGING);
				device_state_set_member(&state, device_state_statusSDCard);
			}
			updateRender();
		}
		ScreenManager_draw_status_bar();
		uint8_t* ScreenBuffer = (uint8_t*)dewcin::Game::getBuffer();
		int sizeXBuffer = 280 % 8 == 0 ? 280 / 8 : 280 / 8 + 1;
		for (int y = 0; y < 480; y++)
		{
			for (int x = 0; x < 280; x++)
			{
				int addr = x / 8 + y * sizeXBuffer;
				uint8_t rdata = ScreenBuffer[addr];
				//Black
				if ((rdata & (0x80 >> (x % 8))) == 0)
				{
					dewcin::Renderer::SetPixel(479-y, x, { 0, 0, 0 });
				}
				else
				{
					dewcin::Renderer::SetPixel(479-y, x, { 255, 255, 255 });
				}
			}
		}

		if (dewcin::Input::isKeyPressed(DC_DOWN))
			startDownArrow();
		if (dewcin::Input::isKeyPressed(DC_UP))
			startUpArrow();
		if (dewcin::Input::isKeyPressed(DC_ENTER))
			startEnterKey();
		if (dewcin::Input::isKeyPressed(DC_M))
			startMenuKey();

		if (dewcin::Input::isKeyReleased(DC_DOWN))
			DownArrowReleased();
		if (dewcin::Input::isKeyReleased(DC_UP))
			UpArrowReleased();
		if (dewcin::Input::isKeyReleased(DC_ENTER))
			EnterKeyReleased();
		if (dewcin::Input::isKeyReleased(DC_M))
			MenuKeyReleased();

		if(dewcin::Input::isKeyPressed(DC_ESCAPE))
			dewcin::Game::getInstance().running = false;
		//dewcin::Input::Position mousePosition = dewcin::Input::getMousePosition();
	}
	);
	open_ReadFile();
	dewcin::Renderer::SetClearColor({ 255, 255, 255 });
	dewcin::Game::setWindowProperties(L"EINK Screen Simulator", width, height);
	dewcin::Game::start();
	// game destroy code

	return 0;
}