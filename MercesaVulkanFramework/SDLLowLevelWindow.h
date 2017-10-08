#pragma once

#include "iLowLevelWindow.h"

struct SDL_Window;

class SDLLowLevelWindow : public iLowLevelWindow
{
public:
	SDLLowLevelWindow() = default;

	virtual void Create(const int32_t& iWidth, const int32_t& iHeight) final;
	virtual void Destroy() final;
	virtual HWND GetWindowHandle() final;

private:
	SDL_Window* window = nullptr;
};

