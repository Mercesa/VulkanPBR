#include "GLFWLowLevelWindow.h"

#include "GLFW/glfw3.h"


GLFWLowLevelWindow::GLFWLowLevelWindow()
{
}


GLFWLowLevelWindow::~GLFWLowLevelWindow()
{
}


void GLFWLowLevelWindow::Create(const int32_t& iScreenWidth, const int32_t& iscreenHeight)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(iScreenWidth, iscreenHeight, "Vulkan", nullptr, nullptr);
}

void GLFWLowLevelWindow::Destroy()
{
	glfwDestroyWindow(window);

	glfwTerminate();
}

HWND GLFWLowLevelWindow::GetWindowHandle()
{
	return 0;
}

std::vector<const char*> GLFWLowLevelWindow::GetRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++) {
		extensions.push_back(glfwExtensions[i]);
	}

	return extensions;
}