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

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>
#include <set>

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);
std::vector<const char*> getAvailableWSIExtensions();

static const int screenWidth = 1280;
static const int screenHeight = 720;

using namespace vk;

vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
vk::SurfaceKHR surface;
vk::Device device = VK_NULL_HANDLE;
vk::Instance instance;

// Extensions and layers
std::vector<const char*> extensions;
std::vector<const char*> layers;

vk::Queue graphicsQueue;
vk::Queue presentQueue;

vk::DebugReportCallbackEXT callback;


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData) {

	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;
	bool isComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices;


	std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = i;
		}

		vk::Bool32 presentSupport = device.getSurfaceSupportKHR(i, surface);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
			std::cout << "Present support detected in family" << std::endl;
		}

		if (indices.isComplete())
		{
			break;
		}
		i++;
	}

	return indices;
}


bool CheckValidationLayerSupport(const std::vector<const char*>& aLayersToCheck)
{
	uint32_t layerCount;
	std::vector<vk::LayerProperties> availableLayers =  vk::enumerateInstanceLayerProperties();

	
	for (const char* layerName : aLayersToCheck)
	{
		bool layerFound = false;

		for (const auto& layerProps : availableLayers)
		{
			layerFound = false;
			if (strcmp(layerName, layerProps.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

void CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;

	// For every unique queue Family, create queue info
	for (int queueFamily : uniqueQueueFamilies)
	{
		vk::DeviceQueueCreateInfo qCreateInfo = vk::DeviceQueueCreateInfo()
			.setQueueFamilyIndex(queueFamily)
			.setQueueCount(1)
			.setPQueuePriorities(&queuePriority);

		queueCreateInfos.push_back(qCreateInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures = {};

	vk::DeviceCreateInfo createInfo = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
		.setPQueueCreateInfos(queueCreateInfos.data())
		.setPEnabledFeatures(&deviceFeatures)
		.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data());


	// Only enable layers in debug mode
#if defined(_DEBUG)
	createInfo.setEnabledLayerCount(static_cast<uint32_t>(layers.size()));
	createInfo.setPpEnabledLayerNames(layers.data());
#else
	createInfo.setEnabledLayerCount(0);
#endif

	device = physicalDevice.createDevice(createInfo);

	if (device == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to create logical device");
	}

	graphicsQueue = device.getQueue(indices.graphicsFamily, 0);
	presentQueue = device.getQueue(indices.presentFamily, 0);
	
}

void SetupDebugCallBack()
{

	VkInstance tempInstance = VkInstance(instance);
	#ifdef MY_DEBUG_BUILD_MACRO
    /* Load VK_EXT_debug_report entry points in debug builds */
  
#endif

#if defined(_DEBUG)
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(tempInstance, "vkCreateDebugReportCallbackEXT"));
	PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT =
		reinterpret_cast<PFN_vkDebugReportMessageEXT>
		(vkGetInstanceProcAddr(tempInstance, "vkDebugReportMessageEXT"));
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(tempInstance, "vkDestroyDebugReportCallbackEXT"));

	//vk::DebugReportCallbackCreateInfoEXT createinfo = vk::DebugReportCallbackCreateInfoEXT()
	//	.setFlags(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning)
	//	.setPfnCallback(debugCallback);
	//
	//callback = instance.createDebugReportCallbackEXT(createinfo);
#else
	return;
#endif;
}

bool isDeviceSuitable(vk::PhysicalDevice device)
{
	vk::PhysicalDeviceProperties deviceProperties;
	//device.getProperties(deviceProperties, device);
	deviceProperties = device.getProperties();

	PhysicalDeviceFeatures deviceFeatures;
	deviceFeatures = device.getFeatures();

	QueueFamilyIndices indices = FindQueueFamilies(device);

	//return true;
	return deviceProperties.deviceType == PhysicalDeviceType::eDiscreteGpu && deviceFeatures.geometryShader && indices.isComplete();
}

void PickPhysicalDevice()
{
	// Create device
	std::vector<vk::PhysicalDevice> physDevices = instance.enumeratePhysicalDevices();

	if (physDevices.size() == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	for (const auto& device : physDevices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU");
	}}


int main()
{
    // Use validation layers if this is a debug build, and use WSI extensions regardless
    extensions = getAvailableWSIExtensions();

#if defined(_DEBUG) 
	// Open and allocate a console window
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);

	
	std::vector<vk::ExtensionProperties> props = vk::enumerateInstanceExtensionProperties(nullptr);
	
	for (auto &e : props)
	{
		extensions.push_back(e.extensionName);
	}
	
	for (auto& e : props)
	{
		std::cout << "Extension found!" << std::endl;
		std::cout << "Name: " << e.extensionName << std::endl;
		std::cout << "SpecVersion: " << e.specVersion << std::endl;
	}
	
	layers.push_back("VK_LAYER_LUNARG_standard_validation");


#endif

	if (!CheckValidationLayerSupport(layers))
	{
		throw std::runtime_error("Validation layers requested, but not available!");
	}

    // vk::ApplicationInfo allows the programmer to specifiy some basic information about the
    // program, which can be useful for layers and tools to provide more debug information.
	vk::ApplicationInfo appInfo = vk::ApplicationInfo()
		.setPApplicationName("Vulkan C++ Windowed Program Template")
		.setApplicationVersion(1)
		.setPEngineName("LunarG SDK")
		.setEngineVersion(1)
		.setApiVersion(VK_API_VERSION_1_0);

    // vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
    // are needed.
	vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
		.setFlags(vk::InstanceCreateFlags())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data())
		.setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
		.setPpEnabledLayerNames(layers.data());

    // Create the Vulkan instance.



    try {
        instance = vk::createInstance(instInfo);
    } catch(const std::exception& e) {
        std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
        return 1;
    }

	SetupDebugCallBack();
	
    // Create an SDL window that supports Vulkan and OpenGL rendering.
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "Could not initialize SDL." << std::endl;
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    if(window == NULL) {
        std::cout << "Could not create SDL window." << std::endl;
        return 1;
    }

    // Create a Vulkan surface for rendering
    try {
        surface = createVulkanSurface(instance, window);
    } catch(const std::exception& e) {
        std::cout << "Failed to create Vulkan surface: " << e.what() << std::endl;
        instance.destroy();
        return 1;
    }
	

	PickPhysicalDevice();
	//CreateLogicalDevice();

	



    // Poll for user input.
    bool stillRunning = true;
    while(stillRunning) {

        SDL_Event event;
        while(SDL_PollEvent(&event)) {

            switch(event.type) {

            case SDL_QUIT:
                stillRunning = false;
                break;

            default:
                // Do nothing.
                break;
            }
        }
		
        SDL_Delay(10);
    }

    // Clean up.
	
	device.destroy();
	instance.destroySurfaceKHR(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
    instance.destroy();
	
	
    return 0;
}

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window)
{
    SDL_SysWMinfo windowInfo;
    SDL_VERSION(&windowInfo.version);
    if(!SDL_GetWindowWMInfo(window, &windowInfo)) {
        throw std::system_error(std::error_code(), "SDK window manager info is not available.");
    }

    switch(windowInfo.subsystem) {

#if defined(SDL_VIDEO_DRIVER_ANDROID) && defined(VK_USE_PLATFORM_ANDROID_KHR)
    case SDL_SYSWM_ANDROID: {
        vk::AndroidSurfaceCreateInfoKHR surfaceInfo = vk::AndroidSurfaceCreateInfoKHR()
            .setWindow(windowInfo.info.android.window);
        return instance.createAndroidSurfaceKHR(surfaceInfo);
    }
#endif

#if defined(SDL_VIDEO_DRIVER_MIR) && defined(VK_USE_PLATFORM_MIR_KHR)
    case SDL_SYSWM_MIR: {
        vk::MirSurfaceCreateInfoKHR surfaceInfo = vk::MirSurfaceCreateInfoKHR()
            .setConnection(windowInfo.info.mir.connection)
            .setMirSurface(windowInfo.info.mir.surface);
        return instance.createMirSurfaceKHR(surfaceInfo);
    }
#endif

#if defined(SDL_VIDEO_DRIVER_WAYLAND) && defined(VK_USE_PLATFORM_WAYLAND_KHR)
    case SDL_SYSWM_WAYLAND: {
        vk::WaylandSurfaceCreateInfoKHR surfaceInfo = vk::WaylandSurfaceCreateInfoKHR()
            .setDisplay(windowInfo.info.wl.display)
            .setSurface(windowInfo.info.wl.surface);
        return instance.createWaylandSurfaceKHR(surfaceInfo);
    }
#endif

#if defined(SDL_VIDEO_DRIVER_WINDOWS) && defined(VK_USE_PLATFORM_WIN32_KHR)
    case SDL_SYSWM_WINDOWS: {
        vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
            .setHinstance(GetModuleHandle(NULL))
            .setHwnd(windowInfo.info.win.window);
        return instance.createWin32SurfaceKHR(surfaceInfo);
    }
#endif

#if defined(SDL_VIDEO_DRIVER_X11) && defined(VK_USE_PLATFORM_XLIB_KHR)
    case SDL_SYSWM_X11: {
        vk::XlibSurfaceCreateInfoKHR surfaceInfo = vk::XlibSurfaceCreateInfoKHR()
            .setDpy(windowInfo.info.x11.display)
            .setWindow(windowInfo.info.x11.window);
        return instance.createXlibSurfaceKHR(surfaceInfo);
    }
#endif

    default:
        throw std::system_error(std::error_code(), "Unsupported window manager is in use.");
    }
}

std::vector<const char*> getAvailableWSIExtensions()
{
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
    extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

    return extensions;
}
