#pragma once

#include <windows.h>

#include <string>
#include <functional>
extern "C" {
#include "Screen/E_Ink/GUI_Paint.h"
}

extern "C" {
	extern void startRender();
}

#define EPD_3IN7_WIDTH       280
#define EPD_3IN7_HEIGHT      480 

namespace dewcin
{
	class Game
	{
		friend LRESULT CALLBACK WindowCallback(
			HWND windowHandle,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);
	
	private:
		HINSTANCE hInstance;
		HWND windowHandle = 0;

		std::wstring windowTitle;
		int windowWidth, windowHeight;
		UBYTE* ScreenBuffer;

		std::function<void(float delta)> update;

	public:
		bool running = false;

		Game();


		Game(const Game&) = delete;
		Game& operator= (const Game&) = delete;

		~Game() {}

		inline static Game& getInstance()
		{
			static Game game;
			return game;
		}

		inline static void start()
		{
			UBYTE* ScreenBuffer = getInstance().ScreenBuffer;

			UWORD Imagesize = 16800;
			if ((ScreenBuffer = (UBYTE*)malloc(Imagesize)) == NULL) {
				printf("Failed to apply for black memory...\r\n");
				return;
			}

			printf("Paint_NewImage \r\n");
			Paint_NewImage(ScreenBuffer, EPD_3IN7_WIDTH, EPD_3IN7_HEIGHT, 270, WHITE);

			Paint_SelectImage(ScreenBuffer);
			Paint_SetScale(2);

			Paint_Clear(WHITE);

			getInstance().ScreenBuffer = ScreenBuffer;
			startRender();
			getInstance().startWindow();
		}

		inline static void setWindowProperties(const std::wstring& title, const int& width, const int& height)
		{
			getInstance().windowTitle = title;
			getInstance().windowWidth = width;
			getInstance().windowHeight = height;
		}

		inline static void setGameUpdate(const std::function<void(float delta)>& update) { getInstance().update = update; }

		inline static std::wstring getWindowTitle() { return getInstance().windowTitle; }
		inline static int getWindowWidth() { return getInstance().windowWidth; }
		inline static int getWindowHeight() { return getInstance().windowHeight; }
		inline static void* getBuffer() { return getInstance().ScreenBuffer; }

	private:
		void startWindow();
	};
}