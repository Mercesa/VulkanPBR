
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
using namespace vk;



std::unique_ptr<RendererVulkan> renderer;
std::unique_ptr<Game> CurrentGame;
std::unique_ptr<EngineTimer> engineTimer;
std::unique_ptr<ResourceManager> resourceManager;

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


	CurrentGame = std::make_unique<Game>();
	resourceManager = std::make_unique<ResourceManager>();
	renderer = std::make_unique<RendererVulkan>();

	CurrentGame->resourceManager = resourceManager.get();
	CurrentGame->Init();

	renderer->Create(CurrentGame->gameObjects, resourceManager.get());

	engineTimer = std::make_unique<EngineTimer>();


	LOG(INFO) << "setup completed" << std::endl;

	static float camX, camY, camZ;
	static float camRotX, camRotY, camRotZ;
	static float mouseMoveRelX, mouseMoveRelY;
	camX = 0.0f;
	camY = 0.0f;
	camZ = 0.0f;

	mouseMoveRelX = 0.0f;
	mouseMoveRelY = 0.0f;
    // Poll for user input.
    bool stillRunning = true;

	bool firstFrame = true;

	engineTimer->Start();
    while(stillRunning) {
		engineTimer->Update();

		camRotX = 0.0f;
		camRotY = 0.0f;
		camRotZ = 0.0f;
		CurrentGame->camera->keys.up = false;
		CurrentGame->camera->keys.down = false;
		CurrentGame->camera->keys.left = false;
		CurrentGame->camera->keys.right = false;

		mouseMoveRelX = 0.0f;
		mouseMoveRelY = 0.0f;

        SDL_Event event;
        while(SDL_PollEvent(&event)) 
		{

            switch(event.type) {

            case SDL_QUIT:
                stillRunning = false;
                break;

			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					stillRunning = false;
				}

				if (event.key.keysym.sym == SDLK_w)
				{
					camY += 1.0f;
					CurrentGame->camera->keys.up = true;
				}

				if (event.key.keysym.sym == SDLK_a)
				{
					camX -= 1.0f;
					CurrentGame->camera->keys.left = true;
				}

				if (event.key.keysym.sym == SDLK_s)
				{
					camY -= 1.0f;
					CurrentGame->camera->keys.down = true;
				}

				if (event.key.keysym.sym == SDLK_d)
				{
					camX += 1.0f;
					CurrentGame->camera->keys.right = true;

				}

				if (event.key.keysym.sym == SDLK_q)
				{
					camRotY -= 1.0f;
				}

				if (event.key.keysym.sym == SDLK_e)
				{
					camRotY += 1.0f;
				}

				break;


			case SDL_MOUSEMOTION:
				mouseMoveRelX = event.motion.xrel;
				mouseMoveRelY = event.motion.yrel;
				break;

            default:
                // Do nothing.
                break;
            }	

        }
		
		if (!firstFrame)
		{
			CurrentGame->Update(engineTimer->GetDeltaTime());
			CurrentGame->camera->rotate(glm::vec3(mouseMoveRelY, mouseMoveRelX, 0.0f));
			CurrentGame->camera->update(1.0f);

			renderer->BeginFrame((*CurrentGame->camera.get()), CurrentGame->lights);
			renderer->Render(CurrentGame->gameObjects);	
		}

		firstFrame = false;
	
		
       // SDL_Delay(10);
    }
	

	renderer->Destroy();
	std::cin.get();

    return 0;
}

