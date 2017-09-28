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

// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

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

using namespace vk;



#include "camera.h"

std::unique_ptr<Camera> cam;
std::unique_ptr<RendererVulkan> renderer;

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

	renderer = std::make_unique<RendererVulkan>();
	renderer->Create();

	cam = std::make_unique<Camera>();

	std::cout << "setup completed" << std::endl;

	static float camX, camY, camZ;
	static float camRotX, camRotY, camRotZ;

	camX = 0.0f;
	camY = 0.0f;
	camZ = 0.0f;

	camRotX = 0.0f;
	camRotY = 0.0f;
	camRotZ = 0.0f;

    // Poll for user input.
    bool stillRunning = true;
    while(stillRunning) {

        SDL_Event event;
        while(SDL_PollEvent(&event)) {

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
				}

				if (event.key.keysym.sym == SDLK_a)
				{
					camX += 1.0f;
				}

				if (event.key.keysym.sym == SDLK_s)
				{
					camY -= 1.0f;
				}

				if (event.key.keysym.sym == SDLK_d)
				{
					camX -= 1.0f;
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

            default:
                // Do nothing.
                break;
            }	
        }
		cam->SetPosition(glm::vec3(camX, camY, camZ));
		cam->SetRotation(glm::vec3(camRotX, camRotY, camRotZ));

		renderer->BeginFrame((*cam.get()));
		renderer->Render();
		
       // SDL_Delay(10);
    }
	

	renderer->Destroy();
	std::cin.get();

    return 0;
}

