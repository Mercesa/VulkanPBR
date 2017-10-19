#include "SDLLowLevelWindow.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "easylogging++.h"

void SDLLowLevelWindow::Create(const int32_t& iWidth, const int32_t& iHeight)
{
	// Create an SDL window that supports Vulkan and OpenGL rendering.
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		LOG(WARNING) << "Could not initialize SDL.";
		return;
	}

	window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, iWidth, iHeight, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		std::cout << "Could not create SDL window." << std::endl;
		return;
	}

	windowWidth = iWidth;
	windowHeight = iHeight;
	LOG(INFO) << "initialized SDL window";
}

void SDLLowLevelWindow::Destroy()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

// We assume for now that we will always be using the windows operating system
HWND SDLLowLevelWindow::GetWindowHandle()
{
	LOG(INFO) << "Correct window handle function";
	SDL_SysWMinfo windowInfo;
	SDL_VERSION(&windowInfo.version);
	if (!SDL_GetWindowWMInfo(window, &windowInfo)) {
		throw std::system_error(std::error_code(), "SDK window manager info is not available.");
		return 0;
	}

	else
	{
		return windowInfo.info.win.window;
	}
}

std::vector<const char*> SDLLowLevelWindow::GetRequiredExtensions()
{
	return std::vector<const char*>();
}