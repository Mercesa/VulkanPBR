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
#include <vulkan/vulkan.h>
#include <glm/gtx/common.hpp>
#include <glm/gtx/transform.hpp>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>

#include <fstream>

#include "cube_data.h"
//#include "SPIRV/GlslangToSpv.h"


#define NUM_SAMPLES vk::SampleCountFlagBits::e1
#define NUM_DESCRIPTOR_SETS 1
#define FENCE_TIMEOUT 100000000

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);
std::vector<const char*> getAvailableWSIExtensions();

static const int screenWidth = 1280;
static const int screenHeight = 720;

using namespace vk;

vk::PhysicalDevice physicalDevice = vk::PhysicalDevice(nullptr);

uint32_t currentBuffer = 0;

std::vector<vk::Framebuffer> framebuffers;

// Descriptor set layout
std::vector<vk::DescriptorSetLayout> desc_layout;
std::vector<vk::DescriptorSet> desc_set;

vk::PipelineLayout pipelineLayout;


// Depth buffer information
vk::Image depthImage;
vk::ImageView depthImageView;
vk::DeviceMemory depthMem;

// Information for the uniform buffer
vk::Buffer uniformBufferBuff;
vk::DeviceMemory uniformBufferMemory;
vk::DescriptorBufferInfo uniformBufferInfo;


vk::DescriptorPool descriptorPool;

vk::RenderPass renderPass;

// Surface and swapchain information
vk::Format format;
vk::SurfaceKHR surface;
vk::SwapchainKHR swapchain;
std::vector<vk::Image> images;
std::vector<vk::ImageView> views;

// Device and instance
vk::Device device = vk::Device(nullptr);
vk::Instance instance;

// Queues 
vk::Queue graphicsQueue;
vk::Queue presentQueue;

// Command pool and buffer
vk::CommandPool commandPool;
vk::CommandBuffer commandBuffer;

// Physical devices, layers and information
std::vector<PhysicalDevice> physicalDevices;
std::vector<LayerProperties> layers;
std::vector<const char*> extensions;

// Information about our device its memory and family properties
vk::PhysicalDeviceMemoryProperties memoryProperties;
std::vector<QueueFamilyProperties> familyProperties;


// Vertex buffer
vk::Buffer vertexBuffer;
vk::DeviceMemory vertexBufferMemory;
vk::VertexInputBindingDescription vertexBindingDescription;
std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescription;

// Family indices
int32_t familyIndex = 0;

int32_t familyGraphicsIndex = 0;
int32_t familyPresenteIndex = 0;

SDL_Window* window = nullptr;

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 modelMatrix;
glm::mat4 clipMatrix;
glm::mat4 mvpMatrix;

std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

vk::Pipeline pipeline;

vk::ShaderModule vertexShaderModule;
vk::ShaderModule fragmentShaderModule;

VkDebugReportCallbackEXT callback;

vk::Viewport viewPort;
vk::Rect2D scissor;


VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
//
PFN_vkDebugReportMessageEXT DebugReportMessageCallback = VK_NULL_HANDLE;
//
PFN_vkDestroyDebugReportCallbackEXT dbgReportCallBack = VK_NULL_HANDLE;


static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}


bool memory_type_from_properties(PhysicalDeviceMemoryProperties memProps, uint32_t typeBits, vk::MemoryPropertyFlags requirements_mask, uint32_t *typeIndex) {
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

VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
	VkDebugReportFlagsEXT       flags,
	VkDebugReportObjectTypeEXT  objectType,
	uint64_t                    object,
	size_t                      location,
	int32_t                     messageCode,
	const char*                 pLayerPrefix,
	const char*                 pMessage,
	void*                       pUserData)
{
	std::cerr << pMessage << std::endl;
	return VK_FALSE;
}


//PFN_vkCreateDebugReportCallbackEXT reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT> (instance.getProcAddr("createDebugReportCallbackEXT"));

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
		std::cout << e.layerName << std::endl;
		//layerNames.push_back(e.layerName);
	}
	
	layerNames.push_back("VK_LAYER_LUNARG_standard_validation");

	std::vector<vk::ExtensionProperties> extProps = vk::enumerateInstanceExtensionProperties();

	for (auto& e : extProps)
	{
		extensions.push_back(e.extensionName);
	}

	for (auto &e : extensions)
	{
		std::cout << "Extension name: " << e << std::endl;
	}
	
	// vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
	// are needed.
	vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
		.setFlags(vk::InstanceCreateFlags())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data())
		.setEnabledLayerCount(static_cast<uint32_t>(layerNames.size()))
		.setPpEnabledLayerNames(layerNames.data());


	// Create instance
	try {
		instance = vk::createInstance(instInfo);
	}
	catch (const std::exception& e) {
		std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
	}

	
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackCreateInfo.pNext = nullptr;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
	callbackCreateInfo.pUserData = nullptr;

	/* Register the callback */
	
	VkInstance tempInst = VkInstance(instance);
	//callback = instance.createDebugReportCallbackEXT(callbackCreateInfo);
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(tempInst, "vkCreateDebugReportCallbackEXT"));

	PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT =
		reinterpret_cast<PFN_vkDebugReportMessageEXT>
		(vkGetInstanceProcAddr(tempInst, "vkDebugReportMessageEXT"));
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(tempInst, "vkDestroyDebugReportCallbackEXT"));

	VkResult result = vkCreateDebugReportCallbackEXT(tempInst, &callbackCreateInfo, nullptr, &callback);

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
#include <iterator>

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

	std::vector<const char*> deviceExtensions;
	std::vector<ExtensionProperties> deviceExtProps = physicalDevices[0].enumerateDeviceExtensionProperties();

	std::vector<const char*> deviceLayers;
	std::vector<LayerProperties> deviceLayerProps = physicalDevices[0].enumerateDeviceLayerProperties();

	//Unsupported extensions
	std::vector<const char*> removeExtensions = { "VK_KHX_external_memory", "VK_KHX_external_memory_win32", "VK_KHX_external_semaphore",  "VK_KHX_external_semaphore_win32", "VK_KHX_win32_keyed_mutex" };

	
	// Fill our extensions and names in
	for (auto& e : deviceExtProps)
	{
		deviceExtensions.push_back(e.extensionName);
		std::cout << e.extensionName << std::endl;
	}

	for (auto& e : deviceLayerProps)
	{
		deviceLayers.push_back(e.layerName);
		std::cout << e.layerName << std::endl;
	}

	// Find the unsupported extensions and remove them
	auto it = deviceExtensions.begin();
	while (it != deviceExtensions.end())
	{
		bool matched = false;
		// Check for a match
		for (int i = 0; i < removeExtensions.size(); ++i)
		{
			if ((*it) == removeExtensions[i])
			{
				it = deviceExtensions.erase(it);
				matched = true;
			}

			if (matched)
			{
				++it;
				break;
			}
		}
	}


	vk::DeviceQueueCreateInfo queueInfo = DeviceQueueCreateInfo()
		.setPNext(nullptr)
		.setQueueCount(1)
		.setPQueuePriorities(queue_priorities);

	vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo()
		.setPNext(nullptr)
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queueInfo)
		.setEnabledExtensionCount(deviceExtensions.size())
		.setPpEnabledExtensionNames(deviceExtensions.data())
		.setEnabledLayerCount(deviceLayers.size())
		.setPpEnabledLayerNames(deviceLayers.data())
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
		 .setOldSwapchain(vk::SwapchainKHR(nullptr))
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
		.setImage(vk::Image(nullptr))
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
	vk::BufferCreateInfo create_info = vk::BufferCreateInfo()
		.setPNext(NULL)
		.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.setSize(sizeof(mvpMatrix))
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(NULL)
		.setSharingMode(vk::SharingMode::eExclusive);

	uniformBufferBuff = device.createBuffer(create_info);

	vk::MemoryRequirements mem_reqs;
	mem_reqs = device.getBufferMemoryRequirements(uniformBufferBuff);

	vk::MemoryAllocateInfo alloc_info = vk::MemoryAllocateInfo()
		.setPNext(NULL)
		.setMemoryTypeIndex(0)
		.setAllocationSize(mem_reqs.size);

	memory_type_from_properties(memoryProperties, mem_reqs.memoryTypeBits, MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent, &alloc_info.memoryTypeIndex);

	uniformBufferMemory = device.allocateMemory(alloc_info);

	uint8_t* pData;
	device.mapMemory(vk::DeviceMemory(uniformBufferMemory), vk::DeviceSize(0), vk::DeviceSize(mem_reqs.size), vk::MemoryMapFlagBits(0), (void**)&pData);

	memcpy(pData, &mvpMatrix, sizeof(mvpMatrix));

	device.unmapMemory(uniformBufferMemory);

	device.bindBufferMemory(uniformBufferBuff, uniformBufferMemory, 0);

	uniformBufferInfo.buffer = uniformBufferBuff;
	uniformBufferInfo.offset = 0;
	uniformBufferInfo.range = sizeof(mvpMatrix);
}

void SetupPipelineLayout()
{
	vk::DescriptorSetLayoutBinding layout_binding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setPImmutableSamplers(NULL);

	vk::DescriptorSetLayoutCreateInfo descriptor_layout = vk::DescriptorSetLayoutCreateInfo()
		.setPNext(NULL)
		.setBindingCount(1)
		.setPBindings(&layout_binding);

	//desc_layout.resize(NUM_DESCRIPTOR_SETS);

	desc_layout.push_back(device.createDescriptorSetLayout(descriptor_layout));

	vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setPNext(NULL)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(NULL)
		.setSetLayoutCount(NUM_DESCRIPTOR_SETS)
		.setPSetLayouts(desc_layout.data());

	pipelineLayout = device.createPipelineLayout(pPipelineLayoutCreateInfo);
}

void SetupDescriptorSet()
{
	vk::DescriptorPoolSize type_count[1];
	type_count[0].type = vk::DescriptorType::eUniformBuffer;
	type_count[0].descriptorCount = 1;

	vk::DescriptorPoolCreateInfo descriptor_pool = vk::DescriptorPoolCreateInfo()
		.setPNext(NULL)
		.setMaxSets(1)
		.setPoolSizeCount(1)
		.setPPoolSizes(type_count);

	descriptorPool = device.createDescriptorPool(descriptor_pool);

	vk::DescriptorSetAllocateInfo alloc_info[1];
	alloc_info[0].pNext = NULL;
	alloc_info[0].setDescriptorPool(descriptorPool);
	alloc_info[0].setDescriptorSetCount(NUM_DESCRIPTOR_SETS);
	alloc_info[0].setPSetLayouts(desc_layout.data());

	desc_set.resize(1);
	device.allocateDescriptorSets(alloc_info, desc_set.data());

	vk::WriteDescriptorSet writes[1];

	writes[0] = {};
	writes[0].pNext = NULL;
	writes[0].dstSet = desc_set[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	writes[0].pBufferInfo = &uniformBufferInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;
	
	device.updateDescriptorSets(1, writes, 0, NULL);
}

void SetupRenderPass()
{

	vk::AttachmentDescription attachments[2] = {};
	attachments[0].format = format;
	attachments[0].samples = NUM_SAMPLES;
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	attachments[0].flags = AttachmentDescriptionFlagBits(0);

	attachments[1].format = format;
	attachments[1].samples = NUM_SAMPLES;
	attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].initialLayout = vk::ImageLayout::eUndefined;
	attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	attachments[1].flags = AttachmentDescriptionFlagBits(0);

	vk::AttachmentReference color_reference = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depth_reference = vk::AttachmentReference()
		.setAttachment(1)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass = vk::SubpassDescription()
		.setPipelineBindPoint(PipelineBindPoint::eGraphics)
		.setFlags(vk::SubpassDescriptionFlagBits(0))
		.setInputAttachmentCount(0)
		.setPInputAttachments(0)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_reference)
		.setPResolveAttachments(NULL)
		.setPDepthStencilAttachment(&depth_reference)
		.setPreserveAttachmentCount(0)
		.setPResolveAttachments(NULL);

	vk::RenderPassCreateInfo rp_info = RenderPassCreateInfo()
		.setPNext(NULL)
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(0)
		.setPDependencies(NULL);


	renderPass = device.createRenderPass(rp_info);
}


void SetupShaders()
{
	auto codeVertex = readFile("Shaders/vert.spv");

	vk::ShaderModuleCreateInfo vertexInfo = vk::ShaderModuleCreateInfo()
		.setCodeSize(codeVertex.size())
		.setPCode(reinterpret_cast<const uint32_t*>(codeVertex.data()));

	vertexShaderModule = device.createShaderModule(vertexInfo);


	auto codeFragment = readFile("Shaders/frag.spv");

	vk::ShaderModuleCreateInfo fragmentInfo = vk::ShaderModuleCreateInfo()
		.setCodeSize(codeFragment.size())
		.setPCode(reinterpret_cast<const uint32_t*>(codeFragment.data()));

	fragmentShaderModule = device.createShaderModule(fragmentInfo);


	vk::PipelineShaderStageCreateInfo shaderStageCInfoVertex = vk::PipelineShaderStageCreateInfo()
		.setStage(vk::ShaderStageFlagBits::eVertex)
		.setPName("main")
		.setModule(vertexShaderModule);


	shaderStages.push_back(shaderStageCInfoVertex);


	vk::PipelineShaderStageCreateInfo shaderStageCInfoFragment = vk::PipelineShaderStageCreateInfo()
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setPName("main")
		.setModule(fragmentShaderModule);
	shaderStages.push_back(shaderStageCInfoFragment);


	//glslang::InitializeProcess();
	//
	//GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vtx_spv);
	//
	//glslang::FinalizeProcess()	
}


//void Execute_queue_command_buffer()
//{
//	const vk::CommandBuffer cmd_bufs[] = { commandBuffer };
//	vk::FenceCreateInfo
//}

void SetupFramebuffers()
{
	vk::ImageView attachments[2];
	attachments[1] = depthImageView;

	vk::FramebufferCreateInfo fb_info = vk::FramebufferCreateInfo()
		.setRenderPass(renderPass)
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setWidth(screenWidth)
		.setHeight(screenHeight)
		.setLayers(1);

	for (int i = 0; i < images.size(); ++i)
	{
		framebuffers.push_back(device.createFramebuffer(fb_info));
	}
}

void SetupVertexBuffer()
{
	vk::BufferCreateInfo buf_info = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
		.setSize(sizeof(g_vb_solid_face_colors_Data))
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(NULL)
		.setSharingMode(SharingMode::eExclusive)
		.setFlags(vk::BufferCreateFlagBits(0));

	vertexBuffer = device.createBuffer(buf_info);

	vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(vertexBuffer);

	vk::MemoryAllocateInfo alloc_info = vk::MemoryAllocateInfo()
		.setMemoryTypeIndex(0)
		.setAllocationSize(memReqs.size);

	memory_type_from_properties(memoryProperties, memReqs.memoryTypeBits, MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &alloc_info.memoryTypeIndex);

	vk::Result res;

	res = device.allocateMemory(&alloc_info, nullptr, &vertexBufferMemory);
	assert(res == vk::Result::eSuccess);

	uint8_t *pData;

	res = device.mapMemory(vertexBufferMemory, 0, vk::DeviceSize(memReqs.size), vk::MemoryMapFlagBits(0), (void**)&pData);
	assert(res == vk::Result::eSuccess);

	memcpy(pData, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data));

	device.unmapMemory(vertexBufferMemory);

	device.bindBufferMemory(vertexBuffer, vertexBufferMemory, 0);

	vertexBindingDescription.binding = 0;
	vertexBindingDescription.inputRate = vk::VertexInputRate::eVertex;
	vertexBindingDescription.stride = sizeof(g_vb_solid_face_colors_Data[0]);



	vk::VertexInputAttributeDescription att1;
	att1.binding = 0;
	att1.location = 0;
	att1.format = vk::Format::eR32G32B32A32Sfloat;

	vk::VertexInputAttributeDescription att2;
	att2.binding = 0;
	att2.location = 1;
	att2.format = vk::Format::eR32G32B32A32Sfloat;
	att2.offset = 16;

	vertexAttributeDescription.push_back(att1);
	vertexAttributeDescription.push_back(att2);

}

#include <ostream>
#include <sstream>
#include <string.h>
VKAPI_ATTR VkBool32 VKAPI_CALL dbgFunc(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
	size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg,
	void *pUserData) {
	std::ostringstream message;

	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		message << "ERROR: ";
	}
	else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		message << "WARNING: ";
	}
	else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		message << "PERFORMANCE WARNING: ";
	}
	else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		message << "INFO: ";
	}
	else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		message << "DEBUG: ";
	}
	message << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;

#ifdef _WIN32
	//MessageBox(NULL, message.str().c_str(), "Alert", MB_OK);
#else
	std::cout << message.str() << std::endl;
#endif

	/*
	* false indicates that layer should not bail-out of an
	* API call that had validation failures. This may mean that the
	* app dies inside the driver due to invalid parameter(s).
	* That's what would happen without validation layers, so we'll
	* keep that behavior here.
	*/
	return false;
}

void SetupPipeline()
{
	std::vector<vk::DynamicState> dynamicStateEnables;
	dynamicStateEnables.resize(VK_DYNAMIC_STATE_RANGE_SIZE); //[VK_DYNAMIC_STATE_RANGE_SIZE];
	
	vk::PipelineDynamicStateCreateInfo dynamicState = PipelineDynamicStateCreateInfo()
		.setPDynamicStates(dynamicStateEnables.data())
		.setDynamicStateCount(0);
	
	vk::PipelineVertexInputStateCreateInfo vi = vk::PipelineVertexInputStateCreateInfo()
		.setFlags(PipelineVertexInputStateCreateFlagBits(0))
		.setPVertexBindingDescriptions(&vertexBindingDescription)
		.setPVertexAttributeDescriptions(vertexAttributeDescription.data())
		.setVertexAttributeDescriptionCount(2)
		.setVertexBindingDescriptionCount(1);

	vk::PipelineInputAssemblyStateCreateInfo ia = vk::PipelineInputAssemblyStateCreateInfo()
		.setPrimitiveRestartEnable(VK_FALSE)
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	vk::PipelineRasterizationStateCreateInfo rs = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setDepthBiasEnable(VK_FALSE)
		.setDepthBiasConstantFactor(0)
		.setDepthBiasClamp(0)
		.setDepthBiasSlopeFactor(0)
		.setLineWidth(1.0f);


	vk::PipelineColorBlendStateCreateInfo cb = {};

	vk::PipelineColorBlendAttachmentState att_state[1] = {};
	att_state[0].colorWriteMask = vk::ColorComponentFlagBits(0xF);
	att_state[0].blendEnable = VK_FALSE;
	att_state[0].alphaBlendOp = vk::BlendOp::eAdd;
	att_state[0].colorBlendOp = vk::BlendOp::eAdd;
	att_state[0].srcColorBlendFactor = vk::BlendFactor::eZero;
	att_state[0].dstColorBlendFactor = vk::BlendFactor::eZero;
	att_state[0].srcAlphaBlendFactor = vk::BlendFactor::eZero;
	att_state[0].dstAlphaBlendFactor = vk::BlendFactor::eZero;

	cb.attachmentCount = 1;
	cb.pAttachments = att_state;
	cb.logicOpEnable = VK_FALSE;
	cb.logicOp = vk::LogicOp::eNoOp;
	cb.blendConstants[0] = 1.0f;
	cb.blendConstants[1] = 1.0f;
	cb.blendConstants[2] = 1.0f;
	cb.blendConstants[3] = 1.0f;


	vk::PipelineViewportStateCreateInfo vp = vk::PipelineViewportStateCreateInfo()
		.setViewportCount(1)
		.setScissorCount(1)
		.setPScissors(NULL)
		.setPViewports(NULL);

	dynamicStateEnables[dynamicState.dynamicStateCount++] = vk::DynamicState::eViewport;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = vk::DynamicState::eScissor;

	vk::PipelineDepthStencilStateCreateInfo ds = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(VK_TRUE)
		.setDepthWriteEnable(VK_TRUE)
		.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setMinDepthBounds(0)
		.setMaxDepthBounds(0)
		.setStencilTestEnable(VK_FALSE)
		.setBack(vk::StencilOpState(StencilOp::eKeep, StencilOp::eKeep, StencilOp::eKeep, CompareOp::eAlways, 0));

	ds.setFront(ds.back);

	vk::PipelineMultisampleStateCreateInfo ms = PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(NUM_SAMPLES)
		.setSampleShadingEnable(VK_FALSE)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE)
		.setMinSampleShading(0.0f)
		.setPSampleMask(VK_NULL_HANDLE);


	vk::GraphicsPipelineCreateInfo gfxPipe = GraphicsPipelineCreateInfo()
		.setLayout(pipelineLayout)
		.setBasePipelineHandle(nullptr)
		.setBasePipelineIndex(0)
		.setPVertexInputState(&vi)
		.setPInputAssemblyState(&ia)
		.setPRasterizationState(&rs)
		.setPColorBlendState(&cb)
		.setPTessellationState(VK_NULL_HANDLE)
		.setPMultisampleState(&ms)
		.setPDynamicState(&dynamicState)
		.setPViewportState(&vp)
		.setPDepthStencilState(&ds)
		.setPStages(shaderStages.data())
		.setStageCount(2)
		.setRenderPass(renderPass)
		.setSubpass(0);

 	pipeline = device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipe);
	
	//vk::PipelineDynamicStateCreateInfo dynamicState = vk
}

void InitViewports()
{
	viewPort.height = (float)screenHeight;
	viewPort.width = (float)screenWidth;
	viewPort.minDepth = (float)0.0f;
	viewPort.maxDepth = (float)1.0f;
	viewPort.x = 0;
	viewPort.y = 0;

	commandBuffer.setViewport(0, 1, &viewPort);
}

void InitScissors()
{
	scissor.extent = vk::Extent2D(screenWidth, screenHeight);
	scissor.offset = vk::Offset2D(0, 0);

	commandBuffer.setScissor(0, 1, &scissor);
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

	projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	viewMatrix = glm::lookAt(glm::vec3(-5, 3, -10), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	modelMatrix = glm::mat4(1.0f);
	clipMatrix = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	mvpMatrix = clipMatrix * projectionMatrix * viewMatrix * modelMatrix;

	extensions = getAvailableWSIExtensions();
	EnumerateLayers();

	SetupApplication();
	SetupSDL();
	EnumerateDevices();
	SetupDevice();
	SetupCommandBuffer();
	SetupSwapchain();
	SetupDepthbuffer();
	SetupUniformbuffer();
	SetupPipelineLayout();
	SetupDescriptorSet();
	SetupRenderPass();
	SetupShaders();
	SetupFramebuffers();
	SetupVertexBuffer();
	SetupPipeline();



	vk::ClearValue clear_values[2] = {};
	clear_values[0].color.float32[0] = 0.2f;
	clear_values[0].color.float32[1] = 0.2f;
	clear_values[0].color.float32[2] = 0.2f;
	clear_values[0].color.float32[3] = 0.2f;
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0;


	vk::Semaphore imageAcquiredSemaphore;
	vk::SemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo = vk::SemaphoreCreateInfo();


	imageAcquiredSemaphore = device.createSemaphore(imageAcquiredSemaphoreCreateInfo);

	device.acquireNextImageKHR(swapchain, UINT64_MAX, imageAcquiredSemaphore, vk::Fence(currentBuffer));


	vk::RenderPassBeginInfo rp_begin = vk::RenderPassBeginInfo()
		.setRenderPass(renderPass)
		.setFramebuffer(framebuffers[currentBuffer])
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(screenWidth, screenHeight)))
		.setClearValueCount(2)
		.setPClearValues(clear_values);
	
		
	// start the pipeline
	commandBuffer.beginRenderPass(&rp_begin, SubpassContents::eInline);
	commandBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, NUM_DESCRIPTOR_SETS, desc_set.data(), 0, NULL);

	const vk::DeviceSize offsets[1] = { 0 };

	commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, offsets);
	InitViewports();
	InitScissors();

	commandBuffer.draw(12 * 3, 1, 0, 0);
	commandBuffer.endRenderPass();
	// End the pipeline


	const vk::CommandBuffer cmd_bufs[] = { commandBuffer };
	vk::FenceCreateInfo fenceInfo;
	vk::Fence drawFence;

	drawFence = device.createFence(fenceInfo, nullptr);

	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::SubmitInfo submit_info[1] = {};
	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = cmd_bufs;
	submit_info[0].signalSemaphoreCount = 0;
	submit_info[0].pSignalSemaphores = NULL;

	graphicsQueue.submit(1, submit_info, drawFence);

	vk::PresentInfoKHR present;
	present.swapchainCount = 1;
	present.pSwapchains = &swapchain;
	present.pImageIndices = &currentBuffer;
	present.pWaitSemaphores = NULL;
	present.waitSemaphoreCount = 0;
	present.pResults = NULL;

	vk::Result res;
	do {
		res = device.waitForFences(1, &drawFence,VK_TRUE, FENCE_TIMEOUT);
	} while (res == vk::Result::eSuccess);

	presentQueue.presentKHR(&present);
	

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
