
// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED


#include "RenderingIncludes.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>
#include <memory>
#include <fstream>


#define ELPP_DISABLE_DEFAULT_CRASH_HANDLING

#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP


#include "RendererVulkan.h"
#include "Game.h"
#include "NewCamera.h"
#include "EngineTimer.h"
#include "ResourceManager.h"
#include "GLFWLowLevelWindow.h"
#include "inputGlfw.h"
#include "MainGUI.h"

#include <imgui.h>
#include "Imgui/imgui_impl_glfw_vulkan.h"


std::unique_ptr<RendererVulkan> renderer;
std::unique_ptr<Game> CurrentGame;
std::unique_ptr<EngineTimer> engineTimer;
std::unique_ptr<ResourceManager> resourceManager;
std::unique_ptr<GLFWLowLevelWindow> window;
std::unique_ptr<inputGlfw> input;
std::unique_ptr<MainGUI> gui;
bool mouseFirstFrame = true;

double lastX = 0;
double lastY = 0;
double relX = 0;
double relY = 0;

static bool startMenu = false;

void cursorpos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if(!startMenu)
	input->MouseMoveInput(xpos, ypos);
}

void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
	//ImGui_ImplGlfwVulkan_MouseButtonCallback(window, button, action, mods);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!startMenu)
	{
		input->KeyboardInput(key, scancode, action, mods);
	}

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		startMenu = !startMenu;
	}
	//ImGui_ImplGlfwVulkan_KeyCallback(window, key, scancode, action, mods);
}

void char_callback(GLFWwindow* window, unsigned int c)
{
	ImGui_ImplGlfwVulkan_CharCallback(window, c);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//ImGui_ImplGlfwVulkan_ScrollCallback(window, xoffset, yoffset);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);


}

int main()
{
    // Use validation layers if this is a debug build, and use WSI extensions regardless

#if defined(_DEBUG) 
	// Open and allocate a console window
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
#endif

	resourceManager = std::make_unique<ResourceManager>();
	input = std::make_unique<inputGlfw>();
	window = std::make_unique<GLFWLowLevelWindow>();
	renderer = std::make_unique<RendererVulkan>();
	engineTimer = std::make_unique<EngineTimer>();
	gui = std::make_unique<MainGUI>();

	GLFWLowLevelWindow* glfwWindow = dynamic_cast<GLFWLowLevelWindow*>(window.get());

	input->Initialize();
	CurrentGame = std::make_unique<Game>(input.get(), resourceManager.get());
	window->Create(1280, 720);
	CurrentGame->Init();
	renderer->Initialize(GFXParams(0, 0, 1280, 720, 0, 60, 1), window.get());
	renderer->PrepareResources(
		resourceManager->texturesToPrepare, 
		resourceManager->modelsToPrepare, 
		resourceManager->objsToPrepare, 
		CurrentGame->gameObjects);

	glfwSetInputMode(glfwWindow->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwMakeContextCurrent(glfwWindow->window);
	
	glfwSetCursorPosCallback(glfwWindow->window, cursorpos_callback);
	glfwSetMouseButtonCallback(glfwWindow->window, mousebutton_callback);
	glfwSetKeyCallback(glfwWindow->window, key_callback);
	glfwSetCharCallback(glfwWindow->window, char_callback);
	glfwSetScrollCallback(glfwWindow->window, scroll_callback);

	engineTimer->Start();

    while(!glfwWindowShouldClose(glfwWindow->window)) 
	{
		// Update timer
		engineTimer->Update();

		ImGui_ImplGlfwVulkan_NewFrame();

		// Process input
		input->Update();
		glfwPollEvents();
		processInput(glfwWindow->window);

		// Update game
		CurrentGame->Update(engineTimer->GetDeltaTime());

		if (startMenu)
		{
			gui->Update(CurrentGame.get());
		}
		// Render resources
		renderer->BeginFrame((*CurrentGame->camera.get()), CurrentGame->lights);
		renderer->Render(CurrentGame->gameObjects);	
    }
	
	ImGui_ImplGlfwVulkan_Shutdown();
	window->Destroy();
	renderer->Destroy();
	std::cin.get();

    return 0;
}

