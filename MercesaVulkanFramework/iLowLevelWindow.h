#pragma once

#include <windows.h>
#include <cstdint>

#include <vector>

class iLowLevelWindow
{
public:
	virtual void Create(const int32_t& iScreenWidth, const int32_t& iscreenHeight ) = 0;
	virtual void Destroy() = 0;
	virtual HWND GetWindowHandle() = 0;
	virtual std::vector<const char*> GetRequiredExtensions() = 0;

protected:
	int32_t windowWidth;
	int32_t windowHeight;
};