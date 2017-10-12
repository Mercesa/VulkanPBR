/*
 * Vulkan Windowed Program
 *
 * Copyright (C) 2016 Valve Corporation
 * Copyright (C) 2016 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
Vulkan C++ Windowed Project Template
Create and destroy a Vulkan surface on an SDL window.
*/



// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED


#include "RenderingIncludes.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>

#include <fstream>


#define ELPP_DISABLE_DEFAULT_CRASH_HANDLING

#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP


#include "RendererVulkan.h"
#include "Game.h"
#include "NewCamera.h"
using namespace vk;



#include "camera.h"
#include <memory>
std::unique_ptr<Camera> cam;
std::unique_ptr<NewCamera> newCam;
std::unique_ptr<RendererVulkan> renderer;
std::unique_ptr<Game> CurrentGame;

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
	CurrentGame->Init();

	renderer = std::make_unique<RendererVulkan>();
	renderer->Create(CurrentGame->gameObjects);

	newCam = std::make_unique<NewCamera>();
	newCam->setPerspective(45, (float)((float)1280.0f / (float)720.0f), 0.01f, 100.0f);
	newCam->setPosition(glm::vec3(0.0f, 0.0f, -10.0));
	

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
    while(stillRunning) {

		camRotX = 0.0f;
		camRotY = 0.0f;
		camRotZ = 0.0f;
		newCam->keys.up = false;
		newCam->keys.down = false;
		newCam->keys.left = false;
		newCam->keys.right = false;

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
					newCam->keys.up = true;
				}

				if (event.key.keysym.sym == SDLK_a)
				{
					camX -= 1.0f;
					newCam->keys.left = true;
				}

				if (event.key.keysym.sym == SDLK_s)
				{
					camY -= 1.0f;
					newCam->keys.down = true;
				}

				if (event.key.keysym.sym == SDLK_d)
				{
					camX += 1.0f;
					newCam->keys.right = true;

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
			CurrentGame->Update();
			newCam->rotate(glm::vec3(mouseMoveRelY, mouseMoveRelX, 0.0f));
			newCam->update(1.0f);

			renderer->BeginFrame((*newCam.get()), CurrentGame->lights);
			renderer->Render(CurrentGame->gameObjects);
			
		}

		firstFrame = false;
	
		
       // SDL_Delay(10);
    }
	

	renderer->Destroy();
	std::cin.get();

    return 0;
}

