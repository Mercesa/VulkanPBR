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

#include <glm/gtx/common.hpp>
#include <glm/gtx/transform.hpp>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);
std::vector<const char*> getAvailableWSIExtensions();

static const int screenWidth = 1280;
static const int screenHeight = 720;

using namespace vk;

vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;


vk::Image depthImage;
vk::ImageView depthImageView;
vk::DeviceMemory depthMem;


vk::Format format;
vk::SurfaceKHR surface;
vk::SwapchainKHR swapchain;
std::vector<vk::Image> images;
std::vector<vk::ImageView> views;

vk::Device device = VK_NULL_HANDLE;
vk::Instance instance;

// Extensions and layers
std::vector<const char*> extensions;

vk::Queue graphicsQueue;
vk::Queue presentQueue;

vk::DebugReportCallbackEXT callback;

vk::CommandPool commandPool;
vk::CommandBuffer commandBuffer;

std::vector<PhysicalDevice> physicalDevices;
std::vector<LayerProperties> layers;

vk::PhysicalDeviceMemoryProperties memoryProperties;

std::vector<QueueFamilyProperties> familyProperties;
int32_t familyIndex = 0;

int32_t familyGraphicsIndex = 0;
int32_t familyPresenteIndex = 0;

SDL_Window* window = nullptr;


glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 modelMatrix;
glm::mat4 clipMatrix;
glm::mat4 mvpMatrix;

#define NUM_SAMPLES vk::SampleCountFlagBits::e1


bool memory_type_from_properties(PhysicalDeviceMemoryProperties memProps, uint32_t typeBits, vk::MemoryPropertyFlagBits requirements_mask, uint32_t *typeIndex) {
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memProps.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

void SetupApplication()
{
	// vk::ApplicationInfo allows the programmer to specifiy some basic information about the
	// program, which can be useful for layers and tools to provide more debug information.
	vk::ApplicationInfo appInfo = vk::ApplicationInfo()
		.setPApplicationName("Vulkan C++ Windowed Program Template")
		.setApplicationVersion(1)
		.setPEngineName("LunarG SDK")
		.setEngineVersion(1)
		.setApiVersion(VK_API_VERSION_1_0);

	std::vector<const char*> layerNames;

	for (auto &e : layers)
	{
		layerNames.push_back(e.layerName);
	}

	// vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
	// are needed.
	vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
		.setFlags(vk::InstanceCreateFlags())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data())
		.setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
		.setPpEnabledLayerNames(layerNames.data());


	// Create instance
	try {
		instance = vk::createInstance(instInfo);
	}
	catch (const std::exception& e) {
		std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
	}
}

// Ensure there is at least 1 physical device capable to be used
void EnumerateDevices()
{
	physicalDevices = instance.enumeratePhysicalDevices();

	assert(physicalDevices.size() != 0);
}

void EnumerateLayers()
{
	layers = vk::enumerateInstanceLayerProperties();
}

void SetupDevice()
{
	vk::PhysicalDevice deviceToUse = physicalDevices[0];

	familyProperties = deviceToUse.getQueueFamilyProperties();

	// Find a family that supports graphics operations
	bool found = false;

	for (unsigned int i = 0; i < familyProperties.size(); i++)
	{
		if (familyProperties[i].queueCount & VK_QUEUE_GRAPHICS_BIT)
		{
			familyIndex = 0;
			found = true;
			break;
		}
	}

	assert(found);
	assert(familyProperties.size() >= 1);

	// Cache the memory properties of our physical device
	memoryProperties = physicalDevices[0].getMemoryProperties();


	float queue_priorities[1] = { 0.0 };

	vk::DeviceQueueCreateInfo queueInfo = DeviceQueueCreateInfo()
		.setPNext(nullptr)
		.setQueueCount(1)
		.setPQueuePriorities(queue_priorities);

	vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo()
		.setPNext(nullptr)
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queueInfo)
		.setEnabledExtensionCount(0)
		.setPpEnabledExtensionNames(NULL)
		.setEnabledLayerCount(0)
		.setPpEnabledLayerNames(NULL)
		.setPEnabledFeatures(NULL);

	device = deviceToUse.createDevice(deviceInfo);
}

void SetupSDL()
{
	// Create an SDL window that supports Vulkan and OpenGL rendering.
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "Could not initialize SDL." << std::endl;
		return;
	}
	
	window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		std::cout << "Could not create SDL window." << std::endl;
		return;
	}

	// Create a Vulkan surface for rendering
	try {
		surface = createVulkanSurface(instance, window);
	}
	catch (const std::exception& e) {
		std::cout << "Failed to create Vulkan surface: " << e.what() << std::endl;
		instance.destroy();
		return;
	}
}

void SetupCommandBuffer()
{
	vk::CommandPoolCreateInfo commandPoolInfo = vk::CommandPoolCreateInfo()
		.setPNext(nullptr)
		.setQueueFamilyIndex(familyIndex);

	commandPool = device.createCommandPool(commandPoolInfo);
	

	vk::CommandBufferAllocateInfo commandBuffAllInfo = vk::CommandBufferAllocateInfo()
		.setPNext(NULL)
		.setLevel(CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1)
		.setCommandPool(commandPool);

	commandBuffer = device.allocateCommandBuffers(commandBuffAllInfo)[0];
}

void SetupSwapchain()
{
	vk::Bool32* pSupportsPresent = (vk::Bool32*)malloc(familyProperties.size() * sizeof(vk::Bool32));

	// Iterate over each queue 
	for (uint32_t i = 0; i < familyProperties.size(); ++i)
	{
		physicalDevices[0].getSurfaceSupportKHR(i, surface, &pSupportsPresent[i]);
	}


	familyGraphicsIndex = UINT32_MAX;
	familyPresenteIndex = UINT32_MAX;

	for (uint32_t i = 0; i < familyProperties.size(); ++i)
	{
		if ((familyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
		{
			if (familyGraphicsIndex == UINT32_MAX)
			{
				familyGraphicsIndex = i;
			}

			if (pSupportsPresent[i] == VK_TRUE) {
				familyGraphicsIndex = i;
				familyPresenteIndex = i;
				break;
			}
		}
	}

	// Find a seperate queue for present if there is no family that supports both
	if (familyPresenteIndex == UINT32_MAX)
	{
		for (int i = 0; i < familyProperties.size(); ++i)
		{
			if (pSupportsPresent[i] == VK_TRUE)
			{
				familyPresenteIndex = i;
				break;
			}
		}
	}

	free(pSupportsPresent);

	if (familyGraphicsIndex == UINT32_MAX && familyPresenteIndex == UINT32_MAX)
	{
		std::cout << "Could not find queues for graphics and present\n";
		exit(-1);
	}

	// get the list of of supported formats
	 std::vector<SurfaceFormatKHR> surfaceFormats = physicalDevices[0].getSurfaceFormatsKHR(surface);

	 if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
	 {
		 format = vk::Format::eB8G8R8A8Unorm;
	 }
	 else
	 {
		 assert(surfaceFormats.size() >= 1);
		 format = surfaceFormats[0].format;
	 }

	 // Get the surface capabilities
	 vk::SurfaceCapabilitiesKHR surfCapabilities;
	 std::vector<vk::PresentModeKHR> presentModes;

	 // Get surface capabilities and present modes
	 surfCapabilities = physicalDevices[0].getSurfaceCapabilitiesKHR(surface);
	 presentModes = physicalDevices[0].getSurfacePresentModesKHR(surface);

	 vk::Extent2D swapchainExtent;

	 // Width and height are either both 0xFFFFFFFFF or both not 0xFFFFFFFF
	 if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
	 {
		 swapchainExtent.width = screenWidth;
		 swapchainExtent.height = screenHeight;

		 if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
		 {
			 swapchainExtent.width = surfCapabilities.minImageExtent.width;
		 }
		 else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width)
		 {
			 swapchainExtent.width = surfCapabilities.maxImageExtent.width;
		 }

		 if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
		 {
			 swapchainExtent.height = surfCapabilities.minImageExtent.height;
		 }
		 else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
		 {
			 swapchainExtent.height = surfCapabilities.maxImageExtent.height;
		 }
	 }
	 else
	 {
		 swapchainExtent = surfCapabilities.currentExtent;
	 }

	 vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

	 uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

	 vk::SurfaceTransformFlagBitsKHR preTransform;
	 if (surfCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
	 {
		 preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	 }
	 else
	 {
		 preTransform = surfCapabilities.currentTransform;
	 }

	 vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	 vk::CompositeAlphaFlagBitsKHR compositeAlphaFLags[4] = {
	 vk::CompositeAlphaFlagBitsKHR::eOpaque,
	 vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
	 vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
	 vk::CompositeAlphaFlagBitsKHR::eInherit
	 };

	 for (uint32_t i = 0; i < sizeof(compositeAlphaFLags); i++)
	 {
		 if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFLags[i])
		 {
			 compositeAlpha = compositeAlphaFLags[i];
			 break;
		 }
	 }

	 vk::SwapchainCreateInfoKHR swapchainCI = SwapchainCreateInfoKHR()
		 .setPNext(NULL)
		 .setSurface(surface)
		 .setMinImageCount(desiredNumberOfSwapChainImages)
		 .setImageFormat(format)
		 .setImageExtent(swapchainExtent)
		 .setPreTransform(preTransform)
		 .setCompositeAlpha(compositeAlpha)
		 .setImageArrayLayers(1)
		 .setPresentMode(swapchainPresentMode)
		 .setOldSwapchain(VK_NULL_HANDLE)
		 .setClipped(true)
		 .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
		 .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		 .setImageSharingMode(vk::SharingMode::eExclusive)
		 .setQueueFamilyIndexCount(0)
		 .setPQueueFamilyIndices(NULL);

	 uint32_t queueFamilyIndices[2] = { familyGraphicsIndex, familyPresenteIndex };

	 if (familyGraphicsIndex != familyPresenteIndex)
	 {
		 swapchainCI.setImageSharingMode(vk::SharingMode::eConcurrent);
		 swapchainCI.setQueueFamilyIndexCount(2);
		 swapchainCI.setPQueueFamilyIndices(queueFamilyIndices);
	 }

	 swapchain = device.createSwapchainKHR(swapchainCI);

	 std::vector<vk::Image> swapchainImages = device.getSwapchainImagesKHR(swapchain);

	 for (int i = 0; i < swapchainImages.size(); ++i)
	 {
		 images.push_back(swapchainImages[i]);
	 }


	 for (int i = 0; i < images.size(); ++i)
	 {
		 vk::ImageViewCreateInfo color_image_view = vk::ImageViewCreateInfo()
			 .setPNext(NULL)
			 .setImage(images[i])
			 .setViewType(vk::ImageViewType::e2D)
			 .setFormat(format)
			 .setComponents(vk::ComponentMapping(ComponentSwizzle::eR, ComponentSwizzle::eG, ComponentSwizzle::eB))
			 .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
			
		 views.push_back(device.createImageView(color_image_view));
	 }
}

void SetupDepthbuffer()
{
	vk::ImageCreateInfo image_Info = {};
	const vk::Format depth_format = vk::Format::eD16Unorm;

	// Query if the format we supplied is supported by our device
	vk::FormatProperties props = physicalDevices[0].getFormatProperties(depth_format);

	if (props.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
	{
		image_Info.tiling = vk::ImageTiling::eLinear;
	}
	else if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
	{
		image_Info.tiling = vk::ImageTiling::eOptimal;
	}
	else
	{
		std::cout << "VK_FORMAT_D16_UNORM Unsupported.\n" << std::endl;
		exit(-1);
	}
	
	image_Info.pNext = NULL;
	image_Info.imageType = vk::ImageType::e2D;
	image_Info.format = depth_format;
	image_Info.extent.width = screenWidth;
	image_Info.extent.height = screenHeight;
	image_Info.extent.depth = 1;
	image_Info.mipLevels = 1;
	image_Info.arrayLayers = 1;
	image_Info.samples = NUM_SAMPLES;
	image_Info.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	image_Info.queueFamilyIndexCount = 0;
	image_Info.pQueueFamilyIndices = NULL;
	image_Info.sharingMode = vk::SharingMode::eExclusive;

	vk::MemoryAllocateInfo mem_alloc = vk::MemoryAllocateInfo()
		.setPNext(NULL)
		.setAllocationSize(0)
		.setMemoryTypeIndex(0);

	vk::ImageViewCreateInfo view_info = vk::ImageViewCreateInfo()
		.setPNext(NULL)
		.setImage(VK_NULL_HANDLE)
		.setFormat(depth_format)
		.setComponents(vk::ComponentMapping(ComponentSwizzle::eR, ComponentSwizzle::eG, ComponentSwizzle::eB, ComponentSwizzle::eA))
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1))
		.setViewType(ImageViewType::e2D);

	vk::MemoryRequirements mem_reqs;

	depthImage = device.createImage(image_Info);

	mem_reqs = device.getImageMemoryRequirements(depthImage);

	mem_alloc.allocationSize = mem_reqs.size;
	
	memory_type_from_properties(memoryProperties, mem_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, &mem_alloc.memoryTypeIndex);
	

	/* Allocate memory */
	depthMem = device.allocateMemory(mem_alloc);

	/* Bind memory */
	device.bindImageMemory(depthImage, depthMem, 0);

	/* Create Image view */
	view_info.image = depthImage;
	depthImageView = device.createImageView(view_info);
}

void SetupUniformbuffer()
{
	//vk::bufferCreateInfo create_info
}

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
#endif

	projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	viewMatrix = glm::lookAt(glm::vec3(-5, 3, -10), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	modelMatrix = glm::mat4(1.0f);
	clipMatrix = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	mvpMatrix = clipMatrix * projectionMatrix * viewMatrix * modelMatrix;

	SetupApplication();
	SetupSDL();
	EnumerateDevices();
	EnumerateLayers();
	SetupDevice();
	SetupCommandBuffer();
	SetupSwapchain();
	SetupDepthbuffer();
	SetupUniformbuffer();

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
