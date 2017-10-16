#pragma once
#include "iLowLevelWindow.h"

#include "GLFW/glfw3.h"

class GLFWLowLevelWindow :
	public iLowLevelWindow
{
public:
	GLFWLowLevelWindow();
	virtual ~GLFWLowLevelWindow();

	virtual void Create(const int32_t& iScreenWidth, const int32_t& iscreenHeight) final;
	virtual void Destroy() final;
	virtual HWND GetWindowHandle() final;
	virtual std::vector<const char*> GetRequiredExtensions() final;

	GLFWwindow* window;


};

