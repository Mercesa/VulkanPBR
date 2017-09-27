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
#define NOMINMAX

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"


#include "RenderingIncludes.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>

#include <fstream>

#define ELPP_DISABLE_DEFAULT_CRASH_HANDLING

#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

#include "VulkanDataObjects.h"
#include "VulkanHelper.h"
#include "Helper.h"
#include "ModelLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <ostream>
#include <sstream>
#include <string.h>
#include <iterator>
#include "ConstantBuffers.h"
#include "DescriptorPoolVulkan.h"

CBMatrix matrixConstantBufferData;

#define NUM_SAMPLES vk::SampleCountFlagBits::e1
#define NUM_DESCRIPTOR_SETS 2
#define FENCE_TIMEOUT 100000000

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);

static const int screenWidth = 1280;
static const int screenHeight = 720;

using namespace vk;

vk::PhysicalDevice physicalDevice = vk::PhysicalDevice(nullptr);

uint32_t currentBuffer = 0;

std::vector<vk::Framebuffer> framebuffers;

// Descriptor set layout
std::vector<vk::DescriptorSetLayout> desc_layout;
std::vector<vk::DescriptorSet> descriptor_set;

vk::PipelineLayout pipelineLayout;


// Depth buffer information


TextureVulkan depthBuffer;

UniformBufferVulkan uniformBufferMVP;


vk::RenderPass renderPass;

// Device and instance
vk::Device device = vk::Device(nullptr);
vk::Instance instance;

// Queues 
vk::Queue graphicsQueue;
vk::Queue presentQueue;

// Command pool and buffer
vk::CommandPool commandPool;
//vk::CommandBuffer commandBuffer;

std::vector<vk::CommandBuffer> commandBuffers;

// Physical devices, layers and information
std::vector<PhysicalDevice> physicalDevices;
std::vector<LayerProperties> layers;
std::vector<const char*> extensions;

// Information about our device its memory and family properties
vk::PhysicalDeviceMemoryProperties memoryProperties;
std::vector<QueueFamilyProperties> familyProperties;

std::unique_ptr<DescriptorPoolVulkan> descriptorPool;

SwapchainVulkan swapchain;

//BufferVulkan vertexBufferStaging;
//VertexBufferVulkan vertexBuffer;


//std::vector<VertexBufferVulkan> vertexBuffers;

std::vector<ModelVulkan> models;

TextureVulkan testTexture;
TextureVulkan testTexture2;

VmaAllocator allocator;

// Family indices
int32_t familyGraphicsIndex = 0;
int32_t familyPresenteIndex = 0;

SDL_Window* window = nullptr;


std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

vk::Pipeline pipeline;

vk::ShaderModule vertexShaderModule;
vk::ShaderModule fragmentShaderModule;

VkDebugReportCallbackEXT callback;

vk::Viewport viewPort;
vk::Rect2D scissor;

std::vector<RawMeshData> rawMeshData;

vk::Sampler testImageSampler;
vk::Sampler testSampler;

vk::Semaphore imageAcquiredSemaphore;
vk::Semaphore rendererFinishedSemaphore;

#include "camera.h"

std::unique_ptr<Camera> cam;


PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
PFN_vkDebugReportMessageEXT DebugReportMessageCallback = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT dbgReportCallBack = VK_NULL_HANDLE;

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
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
	//layerNames.push_back("VK_LAYER_LUNARG_core_validation");
	//layerNames.push_back("VK_LAYER_LUNARG_api_dump");

	std::vector<const char*> removeExtensions = { "VK_KHR_get_surface_capabilities2", "VK_KHX_device_group_creation", "VK_KHR_external_fence_capabilities" };


	std::vector<vk::ExtensionProperties> extProps = vk::enumerateInstanceExtensionProperties();

	bool foundUnwantedExtension = false;
	for (auto& e : extProps)
	{
		foundUnwantedExtension = false;
		for (auto& eTwo : removeExtensions)
		{
			if (strcmp(e.extensionName, eTwo) == 0)
			{
				foundUnwantedExtension = true;
			}
		}
		
		if (foundUnwantedExtension == false)
		{
			extensions.push_back(e.extensionName);
		}
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

void TransitionImageLayout(vk::Image aImage, vk::Format aFormat, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	vk::CommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);

	vk::ImageMemoryBarrier barrier = ImageMemoryBarrier()
		.setOldLayout(oldLayout)
		.setNewLayout(newLayout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(aImage)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
		.setSrcAccessMask(vk::AccessFlagBits(0))
		.setDstAccessMask(vk::AccessFlagBits(0));

	vk::PipelineStageFlagBits sourceStage;
	vk::PipelineStageFlagBits destinationStage;


	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = PipelineStageFlagBits::eTopOfPipe;
		destinationStage = PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits(0);
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;

		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
	}
	else
	{
		LOG(ERROR) << "Unsupported layout transition";
	}

	commandBuffer.pipelineBarrier(
		sourceStage,
		destinationStage,
		vk::DependencyFlagBits(0),
		0, nullptr,
		0, nullptr,
		1, &barrier);

	EndSingleTimeCommands(device, commandBuffer, commandPool, graphicsQueue);
}


void EnumerateLayers()
{
	layers = vk::enumerateInstanceLayerProperties();
}

void SetupDevice()
{
	vk::PhysicalDevice deviceToUse = physicalDevices[0];

	familyProperties = deviceToUse.getQueueFamilyProperties();


	assert(familyProperties.size() >= 1);

	// Cache the memory properties of our physical device
	memoryProperties = physicalDevices[0].getMemoryProperties();



	float queue_priorities[1] = { 0.0 };

	std::vector<const char*> deviceExtensions;
	std::vector<ExtensionProperties> deviceExtProps = physicalDevices[0].enumerateDeviceExtensionProperties();

	std::vector<const char*> deviceLayers;
	std::vector<LayerProperties> deviceLayerProps = physicalDevices[0].enumerateDeviceLayerProperties();

	//Unsupported extensions
	std::vector<const char*> removeExtensions = { "VK_KHX_external_memory_win32", "VK_KHX_external_semaphore",  "VK_KHX_external_semaphore_win32", "VK_KHX_external_memory", "VK_KHX_win32_keyed_mutex"};

	
	bool foundUnsupportedExtension = false;
	// Fill our extensions and names in
	for (auto& e : deviceExtProps)
	{
		foundUnsupportedExtension = false;
		
		for (auto& eSec : removeExtensions)
		{

			if (strcmp(e.extensionName,eSec) == 0)
			{
				foundUnsupportedExtension = true;
			}
		}

		if (foundUnsupportedExtension != true)
		{
			deviceExtensions.push_back(e.extensionName);
			std::cout << e.extensionName << std::endl;
		}
	}

	for (auto& e : deviceLayerProps)
	{
		deviceLayers.push_back(e.layerName);
		std::cout << e.layerName << std::endl;
	}


	vk::PhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

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
		.setPEnabledFeatures(&deviceFeatures);

	device = deviceToUse.createDevice(deviceInfo);


	VmaAllocatorCreateInfo createInfo = VmaAllocatorCreateInfo();
	createInfo.device = VkDevice(device);
	createInfo.physicalDevice = VkPhysicalDevice(physicalDevices[0]);
	createInfo.flags = VMA_ALLOCATOR_FLAG_BITS_MAX_ENUM;


	// Create custom allocator
	vmaCreateAllocator(&createInfo, &allocator);
	
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
		swapchain.surface = createVulkanSurface(instance, window);
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
		.setQueueFamilyIndex(familyGraphicsIndex);

	commandPool = device.createCommandPool(commandPoolInfo);
	

	vk::CommandBufferAllocateInfo commandBuffAllInfo = vk::CommandBufferAllocateInfo()
		.setPNext(NULL)
		.setLevel(CommandBufferLevel::ePrimary)
		.setCommandBufferCount(framebuffers.size())
		.setCommandPool(commandPool);

	commandBuffers = device.allocateCommandBuffers(commandBuffAllInfo);
	
}

void SetupSwapchain()
{
	vk::Bool32* pSupportsPresent = (vk::Bool32*)malloc(familyProperties.size() * sizeof(vk::Bool32));

	// Iterate over each queue 
	for (uint32_t i = 0; i < familyProperties.size(); ++i)
	{
		physicalDevices[0].getSurfaceSupportKHR(i, swapchain.surface, &pSupportsPresent[i]);
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
	 std::vector<SurfaceFormatKHR> surfaceFormats = physicalDevices[0].getSurfaceFormatsKHR(swapchain.surface);

	 if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
	 {
		 swapchain.format = vk::Format::eB8G8R8A8Unorm;
	 }
	 else
	 {
		 assert(surfaceFormats.size() >= 1);
		 swapchain.format = surfaceFormats[0].format;
	 }

	 // Get the surface capabilities
	 vk::SurfaceCapabilitiesKHR surfCapabilities;
	 std::vector<vk::PresentModeKHR> presentModes;

	 // Get surface capabilities and present modes
	 surfCapabilities = physicalDevices[0].getSurfaceCapabilitiesKHR(swapchain.surface);
	 presentModes = physicalDevices[0].getSurfacePresentModesKHR(swapchain.surface);

	 vk::Extent2D swapchainExtent;

	 // Width and height are either both 0xFFFFFFFFF or both not 0xFFFFFFFF
	 if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
	 {
		 swapchainExtent.width = screenWidth;
		 swapchainExtent.height = screenHeight;

		 // Wrap the boundaries
		 swapchainExtent.width = std::max(swapchainExtent.width, surfCapabilities.minImageExtent.width);
		 swapchainExtent.width = std::min(swapchainExtent.width, surfCapabilities.maxImageExtent.width);

		 swapchainExtent.width = std::max(swapchainExtent.height, surfCapabilities.minImageExtent.height);
		 swapchainExtent.width = std::min(swapchainExtent.height, surfCapabilities.maxImageExtent.height);
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
		 .setSurface(swapchain.surface)
		 .setMinImageCount(desiredNumberOfSwapChainImages)
		 .setImageFormat(swapchain.format)
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

	 swapchain.swapchain = device.createSwapchainKHR(swapchainCI);

	 std::vector<vk::Image> swapchainImages = device.getSwapchainImagesKHR(swapchain.swapchain);

	 for (int i = 0; i < swapchainImages.size(); ++i)
	 {
		 swapchain.images.push_back(swapchainImages[i]);
	 }


	 for (int i = 0; i < swapchain.images.size(); ++i)
	 {
		 vk::ImageViewCreateInfo color_image_view = vk::ImageViewCreateInfo()
			 .setPNext(NULL)
			 .setImage(swapchain.images[i])
			 .setViewType(vk::ImageViewType::e2D)
			 .setFormat(swapchain.format)
			 .setComponents(vk::ComponentMapping(ComponentSwizzle::eR, ComponentSwizzle::eG, ComponentSwizzle::eB))
			 .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
			
		 swapchain.views.push_back(device.createImageView(color_image_view));
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
	

	CreateSimpleImage(allocator, depthBuffer.allocation, 
		VMA_MEMORY_USAGE_GPU_ONLY, 
		ImageUsageFlagBits::eDepthStencilAttachment, 
		vk::Format::eD16Unorm, 
		vk::ImageLayout::eUndefined, 
		depthBuffer.image, 
		screenWidth, screenHeight);


	vk::ImageViewCreateInfo view_info = vk::ImageViewCreateInfo()
		.setPNext(NULL)
		.setImage(vk::Image(nullptr))
		.setFormat(depth_format)
		.setComponents(vk::ComponentMapping(ComponentSwizzle::eR, ComponentSwizzle::eG, ComponentSwizzle::eB, ComponentSwizzle::eA))
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1))
		.setViewType(ImageViewType::e2D);

	view_info.image = depthBuffer.image;
	depthBuffer.format = Format::eD16Unorm;

	depthBuffer.view = device.createImageView(view_info);
}



void SetupUniformbuffer()
{
	CreateSimpleBuffer(allocator, 
		uniformBufferMVP.allocation, 
		VMA_MEMORY_USAGE_CPU_TO_GPU, 
		uniformBufferMVP.buffer, 
		vk::BufferUsageFlagBits::eUniformBuffer, 
		sizeof(CBMatrix));
	
	CopyDataToBuffer(VkDevice(device), uniformBufferMVP.allocation, (void*)&matrixConstantBufferData, sizeof(matrixConstantBufferData));


	uniformBufferMVP.descriptorInfo.buffer = uniformBufferMVP.buffer;
	uniformBufferMVP.descriptorInfo.offset = 0;
	uniformBufferMVP.descriptorInfo.range = sizeof(matrixConstantBufferData);
}

void UpdateUniformBufferTest()
{
	static float derp = 1.0f;
	derp += 0.01f;
	float derp2 = (sinf(derp) + 1.0f) / 2.0f;
	
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 viewMatrix = cam->GetViewMatrix();
	glm::mat4 modelMatrix = glm::scale(glm::vec3(0.01f, 0.01f, 0.01));
	glm::mat4 clipMatrix = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	
	matrixConstantBufferData.modelMatrix = modelMatrix;
	matrixConstantBufferData.viewMatrix = viewMatrix;
	matrixConstantBufferData.projectionMatrix = projectionMatrix;
	matrixConstantBufferData.viewProjectMatrix = projectionMatrix * viewMatrix;
	matrixConstantBufferData.mvpMatrix = clipMatrix * projectionMatrix * viewMatrix * modelMatrix;

	CopyDataToBuffer(VkDevice(device), uniformBufferMVP.allocation, (void*)&matrixConstantBufferData, sizeof(matrixConstantBufferData));
	//device.mapMemory(vk::DeviceMemory(uniformBufferMemory), vk::DeviceSize(0), vk::DeviceSize(mem_reqs.size), vk::MemoryMapFlagBits(0), (void**)&pData);
}

void SetupPipelineLayout()
{
	vk::DescriptorSetLayoutBinding sampler_layout_binding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(DescriptorType::eCombinedImageSampler)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);


	vk::DescriptorSetLayoutBinding pureSampler_layout_binding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(DescriptorType::eSampler)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	
	std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { sampler_layout_binding, pureSampler_layout_binding };

	vk::DescriptorSetLayoutCreateInfo descriptor_layout = vk::DescriptorSetLayoutCreateInfo()
		.setPNext(NULL)
		.setBindingCount(static_cast<uint32_t>(bindings.size()))
		.setPBindings(bindings.data());


	vk::DescriptorSetLayoutBinding uniform_binding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setPImmutableSamplers(NULL);

	std::array<vk::DescriptorSetLayoutBinding, 1> uniformBinding = { uniform_binding };


	vk::DescriptorSetLayoutCreateInfo descriptor_layoutUniform = vk::DescriptorSetLayoutCreateInfo()
		.setPNext(NULL)
		.setBindingCount(static_cast<uint32_t>(uniformBinding.size()))
		.setPBindings(uniformBinding.data());



	//desc_layout.resize(NUM_DESCRIPTOR_SETS);

	desc_layout.push_back(device.createDescriptorSetLayout(descriptor_layout));
	desc_layout.push_back(device.createDescriptorSetLayout(descriptor_layoutUniform));

}

void SetupDescriptorSet()
{
	descriptorPool = std::make_unique<DescriptorPoolVulkan>();
	
	descriptorPool->Create(device, 100, 10, 10, 10);

	descriptor_set.resize(NUM_DESCRIPTOR_SETS);

	descriptor_set= descriptorPool->AllocateDescriptorSet(device, 2, desc_layout);
	
	
	std::array<vk::WriteDescriptorSet, 2> writes = {};

	// Create image info for the image descriptor
	vk::DescriptorImageInfo imageInfo = {};
	imageInfo.imageView = testTexture.view;
	imageInfo.sampler = testImageSampler;
	

	writes[0] = {};
	writes[0].pNext = NULL;
	writes[0].dstSet = descriptor_set[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	writes[0].pImageInfo = &imageInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	// Create image info for the image descriptor
	vk::DescriptorImageInfo pureSamplerInfo = {};

	pureSamplerInfo.imageView = vk::ImageView(nullptr);
	pureSamplerInfo.sampler = testSampler;

	writes[1] = {};
	writes[1].pNext = NULL;
	writes[1].dstSet = descriptor_set[0];
	writes[1].descriptorCount = 1; 
	writes[1].descriptorType = vk::DescriptorType::eSampler;
	writes[1].pImageInfo = &pureSamplerInfo;
	writes[1].dstArrayElement = 0;
	writes[1].dstBinding = 1;
	
	device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, NULL);

	std::array<vk::WriteDescriptorSet, 1> uniform_writes = {};

	uniform_writes[0] = {};
	uniform_writes[0].pNext = NULL;
	uniform_writes[0].dstSet = descriptor_set[1];
	uniform_writes[0].descriptorCount = 1;
	uniform_writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	uniform_writes[0].pBufferInfo = &uniformBufferMVP.descriptorInfo;
	uniform_writes[0].dstArrayElement = 0;
	uniform_writes[0].dstBinding = 0;

	device.updateDescriptorSets(static_cast<uint32_t>(uniform_writes.size()), uniform_writes.data(), 0, NULL);
}

void SetupRenderPass()
{

	vk::AttachmentDescription attachments[2] = {};
	attachments[0].format = swapchain.format;
	attachments[0].samples = NUM_SAMPLES;
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	attachments[0].flags = AttachmentDescriptionFlagBits(0);

	attachments[1].format = vk::Format::eD16Unorm;
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


	vk::SubpassDependency dependency = vk::SubpassDependency()
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits(0))
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);


	vk::RenderPassCreateInfo rp_info = RenderPassCreateInfo()
		.setPNext(NULL)
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);


	renderPass = device.createRenderPass(rp_info);
}

/*


void init_resources(TBuiltInResource &Resources) {
	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;
}

EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type) {
	switch (shader_type) {
	case VK_SHADER_STAGE_VERTEX_BIT:
		return EShLangVertex;

	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return EShLangTessControl;

	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return EShLangTessEvaluation;

	case VK_SHADER_STAGE_GEOMETRY_BIT:
		return EShLangGeometry;

	case VK_SHADER_STAGE_FRAGMENT_BIT:
		return EShLangFragment;

	case VK_SHADER_STAGE_COMPUTE_BIT:
		return EShLangCompute;

	default:
		return EShLangVertex;
	}
}

bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader, std::vector<unsigned int> &spirv) {
	EShLanguage stage = FindLanguage(shader_type);
	glslang::TShader shader(stage);
	glslang::TProgram program;
	const char *shaderStrings[1];
	TBuiltInResource Resources;
	init_resources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	shaderStrings[0] = pshader;
	shader.setStrings(shaderStrings, 1);

	if (!shader.parse(&Resources, 100, false, messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		return false;  // something didn't work
	}

	program.addShader(&shader);

	//
	// Program-level processing...
	//

	if (!program.link(messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		return false;
	}

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
	return true;
}


*/
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
}


void SetupFramebuffers()
{
	vk::ImageView attachments[2] = {};
	attachments[1] = depthBuffer.view;
	attachments[0] = vk::ImageView(nullptr);

	vk::FramebufferCreateInfo fb_info = vk::FramebufferCreateInfo()
		.setRenderPass(renderPass)
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setWidth(screenWidth)
		.setHeight(screenHeight)
		.setLayers(1);

	for (int i = 0; i < swapchain.images.size(); ++i)
	{
		attachments[0] = swapchain.views[i];
		framebuffers.push_back(device.createFramebuffer(fb_info));
	}
}

void CopyBufferMemory(vk::Buffer srcBuffer, vk::Buffer destBuffer, int32_t aSize)
{
	vk::CommandBuffer tCmdBuffer = BeginSingleTimeCommands(device , commandPool);

	vk::BufferCopy copyRegion = vk::BufferCopy()
		.setSrcOffset(0)
		.setDstOffset(0)
		.setSize(aSize);

	tCmdBuffer.copyBuffer(srcBuffer, destBuffer, 1, &copyRegion);

	EndSingleTimeCommands(device, tCmdBuffer, commandPool, graphicsQueue);
}

void CopyBufferToImage(vk::Buffer srcBuffer, vk::Image destImage, uint32_t width, uint32_t height)
{
	vk::CommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
	vk::BufferImageCopy region = vk::BufferImageCopy()
		.setBufferOffset(0)
		.setBufferRowLength(0)
		.setBufferImageHeight(0)
		.setImageOffset(vk::Offset3D(0, 0, 0))
		.setImageExtent(vk::Extent3D(width, height, 1));

	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	commandBuffer.copyBufferToImage(
		srcBuffer, 
		destImage, 
		ImageLayout::eTransferDstOptimal, 
		1, 
		&region);

	EndSingleTimeCommands(device, commandBuffer, commandPool, graphicsQueue);
}

void SetupIndexBuffer(BufferVulkan& oIndexBuffer, const RawMeshData& iRawMeshData)
{
	
	BufferVulkan indexBufferStageT;

	CreateSimpleBuffer(
		allocator,
		indexBufferStageT.allocation,
		VMA_MEMORY_USAGE_CPU_ONLY,
		indexBufferStageT.buffer,
		vk::BufferUsageFlagBits::eTransferSrc,
		sizeof(uint32_t) * iRawMeshData.indices.size());

	CopyDataToBuffer(device, indexBufferStageT.allocation, (void*)iRawMeshData.indices.data(), sizeof(uint32_t) * iRawMeshData.indices.size());

	CreateSimpleBuffer(
		allocator,
		oIndexBuffer.allocation,
		VMA_MEMORY_USAGE_GPU_ONLY,
		oIndexBuffer.buffer,
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		sizeof(uint32_t) * iRawMeshData.indices.size());

	CopyBufferMemory(indexBufferStageT.buffer, oIndexBuffer.buffer, oIndexBuffer.allocation->GetSize());

	vmaDestroyBuffer(allocator, indexBufferStageT.buffer, indexBufferStageT.allocation);
}

void SetupVertexBuffer(VertexBufferVulkan& oVertexBuffer, const RawMeshData& iRawMeshdata)
{
		size_t dataSize = sizeof(VertexData) * iRawMeshdata.vertices.size();

		BufferVulkan stagingT;

		CreateSimpleBuffer(allocator,
			stagingT.allocation,
			VMA_MEMORY_USAGE_CPU_ONLY,
			stagingT.buffer,
			BufferUsageFlagBits::eTransferSrc,
			dataSize);


		CopyDataToBuffer((VkDevice)device,
			stagingT.allocation,
			(void*)iRawMeshdata.vertices.data(),
			dataSize);


		CreateSimpleBuffer(allocator,
			oVertexBuffer.allocation,
			VMA_MEMORY_USAGE_GPU_ONLY,
			oVertexBuffer.buffer,
			BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst,
			dataSize);

		// end of setup

		oVertexBuffer.inputDescription.binding = 0;
		oVertexBuffer.inputDescription.inputRate = vk::VertexInputRate::eVertex;
		oVertexBuffer.inputDescription.stride = sizeof(VertexData);

		// 12 bits 
		// 8 bits offset = 12
		// 12 bits == 
		// 12 bits
		// 12 bits
		vk::VertexInputAttributeDescription att1;
		att1.binding = 0;
		att1.location = 0;
		att1.format = vk::Format::eR32G32B32Sfloat;

		vk::VertexInputAttributeDescription att2;
		att2.binding = 0;
		att2.location = 1;
		att2.format = vk::Format::eR32G32Sfloat;
		att2.offset = 12;

		vk::VertexInputAttributeDescription att3;
		att3.binding = 0;
		att3.location = 2;
		att3.format = vk::Format::eR32G32B32Sfloat;
		att3.offset = 20;

		vk::VertexInputAttributeDescription att4;
		att4.binding = 0;
		att4.location = 3;
		att4.format = vk::Format::eR32G32B32Sfloat;
		att4.offset = 32;

		vk::VertexInputAttributeDescription att5;
		att5.binding = 0;
		att5.location = 4;
		att5.format = vk::Format::eR32G32B32Sfloat;
		att5.offset = 44;


		oVertexBuffer.inputAttributes.push_back(att1);
		oVertexBuffer.inputAttributes.push_back(att2);
		oVertexBuffer.inputAttributes.push_back(att3);
		oVertexBuffer.inputAttributes.push_back(att4);
		oVertexBuffer.inputAttributes.push_back(att5);
		// Create staging buffer

		CopyBufferMemory(stagingT.buffer, oVertexBuffer.buffer, oVertexBuffer.allocation->GetSize());

		vmaDestroyBuffer(allocator, stagingT.buffer, stagingT.allocation);
}


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

	vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setPNext(NULL)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(NULL)
		.setSetLayoutCount(NUM_DESCRIPTOR_SETS)
		.setPSetLayouts(desc_layout.data());

	pipelineLayout = device.createPipelineLayout(pPipelineLayoutCreateInfo);

	std::vector<vk::DynamicState> dynamicStateEnables;
	dynamicStateEnables.resize(VK_DYNAMIC_STATE_RANGE_SIZE); //[VK_DYNAMIC_STATE_RANGE_SIZE];
	
	vk::PipelineDynamicStateCreateInfo dynamicState = PipelineDynamicStateCreateInfo()
		.setPDynamicStates(dynamicStateEnables.data())
		.setDynamicStateCount(0);
	
	vk::PipelineVertexInputStateCreateInfo vi = vk::PipelineVertexInputStateCreateInfo()
		.setFlags(PipelineVertexInputStateCreateFlagBits(0))
		.setPVertexBindingDescriptions(&models[0].vertexBuffer.inputDescription)
		.setPVertexAttributeDescriptions(models[0].vertexBuffer.inputAttributes.data())
		.setVertexAttributeDescriptionCount(5)
		.setVertexBindingDescriptionCount(1);

	vk::PipelineInputAssemblyStateCreateInfo ia = vk::PipelineInputAssemblyStateCreateInfo()
		.setPrimitiveRestartEnable(VK_FALSE)
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	vk::PipelineRasterizationStateCreateInfo rs = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
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
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setMinDepthBounds(0)
		.setMaxDepthBounds(1.0f)
		.setStencilTestEnable(VK_FALSE);
		//.setBack(vk::StencilOpState(StencilOp::eKeep, StencilOp::eKeep, StencilOp::eKeep, CompareOp::eAlways, 0));

	//ds.setFront(ds.back);
	
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

void InitViewports(vk::CommandBuffer aBuffer)
{
	viewPort.height = (float)screenHeight;
	viewPort.width = (float)screenWidth;
	viewPort.minDepth = (float)0.0f;
	viewPort.maxDepth = (float)1.0f;
	viewPort.x = 0;
	viewPort.y = 0;

	aBuffer.setViewport(0, 1, &viewPort);
}

void InitScissors(vk::CommandBuffer aBuffer)
{
	scissor.extent = vk::Extent2D(screenWidth, screenHeight);
	scissor.offset = vk::Offset2D(0, 0);

	aBuffer.setScissor(0, 1, &scissor);
}

void SetupDeviceQueue()
{
	graphicsQueue = device.getQueue(familyGraphicsIndex, 0);

	if (familyGraphicsIndex == familyPresenteIndex)
	{
		presentQueue = graphicsQueue;
	}
	else
	{
		presentQueue = device.getQueue(familyPresenteIndex, 0);
	}
}



void SetupSemaphores()
{
	vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();
	imageAcquiredSemaphore = device.createSemaphore(semaphoreInfo);
	rendererFinishedSemaphore = device.createSemaphore(semaphoreInfo);
}


void SetupCommandBuffers()
{
	for (int i = 0; i < commandBuffers.size(); ++i)
	{
		vk::CommandBufferBeginInfo cmd_begin = vk::CommandBufferBeginInfo();

		commandBuffers[i].begin(cmd_begin);


		vk::ClearValue clear_values[2] = {};
		clear_values[0].color.float32[0] = 0.2f;
		clear_values[0].color.float32[1] = 0.2f;
		clear_values[0].color.float32[2] = 0.2f;
		clear_values[0].color.float32[3] = 0.2f;
		clear_values[1].depthStencil.depth = 1.0f;
		clear_values[1].depthStencil.stencil = 0;

		const vk::DeviceSize offsets[1] = { 0 };

		vk::RenderPassBeginInfo rp_begin = vk::RenderPassBeginInfo()
			.setRenderPass(renderPass)
			.setFramebuffer(framebuffers[i])
			.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(screenWidth, screenHeight)))
			.setClearValueCount(2)
			.setPClearValues(clear_values);

		commandBuffers[i].beginRenderPass(&rp_begin, SubpassContents::eInline);
		commandBuffers[i].bindPipeline(PipelineBindPoint::eGraphics, pipeline);

		commandBuffers[i].bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, NUM_DESCRIPTOR_SETS, descriptor_set.data(), 0, NULL);

		InitViewports(commandBuffers[i]);
		InitScissors(commandBuffers[i]);

	
		for (int j = 0; j < models.size(); ++j)
		{
			commandBuffers[i].bindVertexBuffers(0, 1, &models[j].vertexBuffer.buffer, offsets);
			commandBuffers[i].bindIndexBuffer(models[j].indexBuffer.buffer, 0, IndexType::eUint32);
			commandBuffers[i].drawIndexed(models[j].indiceCount, 1, 0, 0, 0);
		}

		//commandBuffers[i].draw(12 * 3, 1, 0, 0);s

		commandBuffers[i].endRenderPass();
		// End the pipeline
		commandBuffers[i].end();
	}
}


void SetupTextureImage(vk::Device iDevice, std::string iFilePath, vk::Image& oImage, VmaAllocation& oAllocation)
{
	// Load texture with stbi
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(iFilePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	vk::DeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		LOG(ERROR) << "Load texture failed! Fallback to error texture..";
		pixels = stbi_load("textures/ErrorTexture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		imageSize = texWidth * texHeight * 4;
		if (!pixels)
		{
			LOG(FATAL) << "FAILED TO LOAD ERRORTEXTURE, THIS SHOULD NOT HAPPEN";
		}
	}


	// Create staging buffer for image
	BufferVulkan stagingBuffer;
	
	// create simple buffer
	CreateSimpleBuffer(allocator, 
		stagingBuffer.allocation,
		VMA_MEMORY_USAGE_CPU_ONLY, 
		stagingBuffer.buffer,
		BufferUsageFlagBits::eTransferSrc, 
		imageSize);

	// Copy image data to buffer
	CopyDataToBuffer(VkDevice(device), stagingBuffer.allocation, pixels, imageSize);

	// Free image
	stbi_image_free(pixels);


	CreateSimpleImage(allocator, 
		oAllocation, 
		VMA_MEMORY_USAGE_GPU_ONLY, 
		ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eSampled, 
		Format::eR8G8B8A8Unorm, ImageLayout::eUndefined, 
		oImage, texWidth, texHeight);

	TransitionImageLayout(oImage, 
		vk::Format::eR8G8B8A8Unorm, 
		vk::ImageLayout::eUndefined, 
		vk::ImageLayout::eTransferDstOptimal);

	CopyBufferToImage(stagingBuffer.buffer, oImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	
	TransitionImageLayout(oImage, 
		vk::Format::eR8G8B8A8Unorm, 
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal);


	vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
}


vk::ImageView CreateImageView(vk::Device aDevice, vk::Image aImage, vk::Format aFormat)
{
	vk::ImageViewCreateInfo	viewInfo = vk::ImageViewCreateInfo()
		.setImage(aImage)
		.setViewType(ImageViewType::e2D)
		.setFormat(aFormat)
		.setSubresourceRange(
			vk::ImageSubresourceRange(
				ImageAspectFlagBits::eColor, 
				0, 1, 0, 1));

	vk::ImageView imgView;
	imgView = aDevice.createImageView(viewInfo, nullptr);

	return imgView;
}

void CreateTextureSampler(vk::Device const aDevice)
{
	vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo()
		.setMagFilter(Filter::eLinear)
		.setMinFilter(Filter::eLinear)
		.setAddressModeU(SamplerAddressMode::eRepeat)
		.setAddressModeV(SamplerAddressMode::eRepeat)
		.setAddressModeW(SamplerAddressMode::eRepeat)
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(16)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setBorderColor(BorderColor::eIntOpaqueBlack)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(CompareOp::eAlways)
		.setMipmapMode(SamplerMipmapMode::eLinear)
		.setMipLodBias(0.0f)
		.setMinLod(0.0f)
		.setMaxLod(0.0f);

	testImageSampler = aDevice.createSampler(samplerInfo);

	testSampler = aDevice.createSampler(samplerInfo);

}

void DrawFrame()
{
	//device.acquireNextImageKHR(swapchain.swapchain, UINT64_MAX, imageAcquiredSemaphore, vk::Fence(currentBuffer));
	vmaSetCurrentFrameIndex(allocator, currentBuffer);
	device.acquireNextImageKHR(swapchain.swapchain, UINT64_MAX, imageAcquiredSemaphore, vk::Fence(nullptr), &currentBuffer);


	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::SubmitInfo submit_info[1] = {};
	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = &commandBuffers[currentBuffer];
	submit_info[0].signalSemaphoreCount = 1;
	submit_info[0].pSignalSemaphores = &rendererFinishedSemaphore;
	// start the pipeline

	graphicsQueue.submit(1, submit_info, vk::Fence(nullptr));

	vk::PresentInfoKHR present;
	present.swapchainCount = 1;	
	present.pSwapchains = &swapchain.swapchain;	
	present.pImageIndices = &currentBuffer;	
	present.pWaitSemaphores = &rendererFinishedSemaphore;	//
	present.waitSemaphoreCount = 1;	
	present.pResults = 0;	

	presentQueue.presentKHR(&present);
	presentQueue.waitIdle();
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

	rawMeshData = ModelLoader::LoadModel("Models/Sponza/sponza.obj", false);

	extensions = getAvailableWSIExtensions();
	EnumerateLayers();

	SetupApplication();
	SetupSDL();
	EnumerateDevices();
	SetupDevice();

	SetupSwapchain();

	SetupUniformbuffer();
	SetupPipelineLayout();
	SetupRenderPass();
	SetupShaders();

	SetupDepthbuffer();

	SetupFramebuffers();
	SetupCommandBuffer();

	SetupDeviceQueue();
	TransitionImageLayout(depthBuffer.image, depthBuffer.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	
	
	for (auto& e : rawMeshData)
	{
		ModelVulkan tModV;
		SetupVertexBuffer(tModV.vertexBuffer, e);
		SetupIndexBuffer(tModV.indexBuffer, e);

		tModV.indiceCount = e.indices.size();

		models.push_back(tModV);
	}


	SetupPipeline();
	SetupSemaphores();

	// Prepare our texture
	SetupTextureImage(device, "textures/statue-e1275469_1920.jpg", testTexture.image, testTexture.allocation);
	testTexture.view = CreateImageView(device, testTexture.image, Format::eR8G8B8A8Unorm);

	SetupTextureImage(device, "textures/Kingston_rnd1_dirt.jpg", testTexture2.image, testTexture2.allocation);
	testTexture2.view = CreateImageView(device, testTexture2.image, Format::eR8G8B8A8Unorm);


	CreateTextureSampler(device);

	SetupDescriptorSet();
	SetupCommandBuffers();

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

		UpdateUniformBufferTest();
		DrawFrame();
		
        SDL_Delay(10);
    }
	

	descriptorPool->Destroy(device);

	device.destroySampler(testSampler);
	device.destroySampler(testImageSampler);
	vmaDestroyImage(allocator, (VkImage)testTexture.image, testTexture.allocation);
	vmaDestroyImage(allocator, (VkImage)depthBuffer.image, depthBuffer.allocation);
	vmaDestroyBuffer(allocator, (VkBuffer)uniformBufferMVP.buffer, uniformBufferMVP.allocation);

	for (auto& e : models)
	{
		vmaDestroyBuffer(allocator, e.vertexBuffer.buffer, e.vertexBuffer.allocation);
		vmaDestroyBuffer(allocator, e.indexBuffer.buffer, e.indexBuffer.allocation);
	}

	device.destroyImageView(testTexture.view, nullptr);
	//device.destroyImage(depthImage);
	//device.destroyImage(testTexture.image);


	// Clean up.
	vmaDestroyAllocator(allocator);

	device.waitIdle();
	device.destroy();
	instance.destroySurfaceKHR(swapchain.surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
    instance.destroy();
	std::cin.get();

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
