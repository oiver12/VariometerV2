#include "dewcin.h"
#include <time.h>

extern "C" {
	#include "Screen/DeviceContext.h"
	#include "Screen/Vario_Screen.h"

	extern void updateRender();
	extern void startLeftArrow();
	extern void LeftArrowReleased();
	extern void startRightArrow();
	extern void RightArrowReleased();
	extern void startEnterKey();
	extern void EnterKeyReleased();
	extern void startMenuKey();
	extern void MenuKeyReleased();
}

dewcin_app_entry_point
{
	// game init code
	int width = 480;
	int height = 280;
	dewcin::Game::setGameUpdate([&](float delta)
	{
		uint8_t zahl1 = 0xEA;
		uint8_t zahl2 = 0xDF;
		uint8_t xs = 2;
		int32_t oneorzero = (zahl1 & (0x80 >> (xs % 8))) >> (7 - (xs % 8));
		uint32_t n = 7 - (xs % 8);
		zahl2 = (zahl2 & ~(1UL << n)) | (oneorzero << n);

		wchar_t charBuffer[256];
		//swprintf(charBuffer, 256, L"delta: %f\n", delta);
		//OutputDebugString(charBuffer);

		static int frames = 0;
		static float timePassed = 0.0f;
		static float vertVel = 0.0f;

		frames++;
		timePassed += delta;

		if (timePassed >= 1.0f)
		{
			swprintf(charBuffer, 256, L"FPS: %d\n", frames);
			OutputDebugString(charBuffer);

			timePassed -= 1.0f;
			frames = 0;
			vertVel++;
			vario_state_set_member(&vertVel, vario_state_vertVel);
			float altitude = sin(vertVel / 5) * 20;
			vario_state_set_member(&altitude, vario_state_altitudeBaro);
			ScreenManager_set_Battery(vertVel);
			int heading = (int)vertVel * 20;
			vario_state_set_member(&heading, vario_state_heading);
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

		if (dewcin::Input::isKeyPressed(DC_LEFT))
			startLeftArrow();
		if (dewcin::Input::isKeyPressed(DC_RIGHT))
			startRightArrow();
		if (dewcin::Input::isKeyPressed(DC_ENTER))
			startEnterKey();
		if (dewcin::Input::isKeyPressed(DC_DOWN))
			startMenuKey();

		if (dewcin::Input::isKeyReleased(DC_LEFT))
			LeftArrowReleased();
		if (dewcin::Input::isKeyReleased(DC_RIGHT))
			RightArrowReleased();
		if (dewcin::Input::isKeyReleased(DC_ENTER))
			EnterKeyReleased();
		if (dewcin::Input::isKeyReleased(DC_DOWN))
			MenuKeyReleased();
		//dewcin::Input::Position mousePosition = dewcin::Input::getMousePosition();
	}
	);
	dewcin::Renderer::SetClearColor({ 255, 255, 255 });
	dewcin::Game::setWindowProperties(L"EINK Screen Simulator", width, height);
	dewcin::Game::start();
	// game destroy code

	return 0;
}