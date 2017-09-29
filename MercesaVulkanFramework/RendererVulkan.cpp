#include "RendererVulkan.h"



// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define NOMINMAX

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "RenderingIncludes.h"


#include <iostream>
#include <math.h>
#include <algorithm>

#include "VulkanDataObjects.h"
#include "GraphicsStructures.h"
#include "DescriptorPoolVulkan.h"

#include "VulkanHelper.h"
#include "Helper.h"
#include "ModelLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Camera.h"

#include <ostream>
#include <sstream>
#include <string.h>
#include <iterator>
#include "ConstantBuffers.h"

#include "DescriptorPoolVulkan.h"
#include "CommandpoolVulkan.h"

#include "easylogging++.h"

CBMatrix matrixConstantBufferData;
CBLights lightConstantBufferData ;

#define NUM_SAMPLES vk::SampleCountFlagBits::e1
#define NUM_DESCRIPTOR_SETS 3
#define FENCE_TIMEOUT 100000000
#define NUM_FRAMES 2

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);

static const int screenWidth = 1280;
static const int screenHeight = 720;

using namespace vk;

vk::PhysicalDevice physicalDevice = vk::PhysicalDevice(nullptr);

uint32_t currentBuffer = 0;

std::vector<vk::Framebuffer> framebuffers;

// Descriptor set layout
std::vector<vk::DescriptorSetLayout> desc_layout;
std::vector<std::vector<vk::DescriptorSet>> descriptor_set;

vk::PipelineLayout pipelineLayout;

TextureVulkan depthBuffer;
TextureVulkan postProcBuffer;

std::vector<UniformBufferVulkan> uniformBufferMVP;
std::vector<UniformBufferVulkan> uniformBufferLights;

vk::RenderPass renderPass;

// Device and instance
vk::Device device = vk::Device(nullptr);
vk::Instance instance;

// Queues 
vk::Queue graphicsQueue;
vk::Queue presentQueue;

std::unique_ptr<CommandpoolVulkan> cmdPool;
std::vector<vk::CommandBuffer> commandBuffers;

// Physical devices, layers and information
std::vector<PhysicalDevice> physicalDevices;
std::vector<LayerProperties> layers;
std::vector<const char*> extensions;

// Information about our device its memory and family properties
std::vector<QueueFamilyProperties> familyProperties;

std::unique_ptr<DescriptorPoolVulkan> descriptorPool;

SwapchainVulkan swapchain;

//BufferVulkan vertexBufferStaging;
//VertexBufferVulkan vertexBuffer;
//std::vector<VertexBufferVulkan> vertexBuffers;

std::vector<ModelVulkan> models;

VmaAllocator allocator;

// Family indices
int32_t familyGraphicsIndex = 0;
int32_t familyPresenteIndex = 0;

SDL_Window* window = nullptr;


std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

vk::Pipeline pipeline;



VkDebugReportCallbackEXT callback;

vk::Viewport viewPort;
vk::Rect2D scissor;

std::vector<RawMeshData> rawMeshData;

vk::Sampler testImageSampler;
vk::Sampler testSampler;

vk::Semaphore imageAcquiredSemaphore;
vk::Semaphore rendererFinishedSemaphore;

std::vector<vk::DescriptorSetLayoutBinding> bindings;
std::vector<vk::DescriptorSetLayoutBinding> uniformBinding;
std::vector<vk::DescriptorSetLayoutBinding> normalTextureBinding;

std::vector<ShaderVulkan> shaders;

RendererVulkan::RendererVulkan()
{
}


RendererVulkan::~RendererVulkan()
{
}

PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
PFN_vkDebugReportMessageEXT DebugReportMessageCallback = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT dbgReportCallBack = VK_NULL_HANDLE;


vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window)
{
	SDL_SysWMinfo windowInfo;
	SDL_VERSION(&windowInfo.version);
	if (!SDL_GetWindowWMInfo(window, &windowInfo)) {
		throw std::system_error(std::error_code(), "SDK window manager info is not available.");
	}

	switch (windowInfo.subsystem) {

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

#ifdef _DEBUG
	layerNames.push_back("VK_LAYER_LUNARG_standard_validation");
	layerNames.push_back("VK_LAYER_LUNARG_core_validation");

#endif;
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

void TransitionImageLayout(vk::CommandBuffer iBuffer, vk::Image aImage, vk::Format aFormat, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{

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

	iBuffer.pipelineBarrier(
		sourceStage,
		destinationStage,
		vk::DependencyFlagBits(0),
		0, nullptr,
		0, nullptr,
		1, &barrier);

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

	float queue_priorities[1] = { 0.0 };

	std::vector<const char*> deviceExtensions;
	std::vector<ExtensionProperties> deviceExtProps = physicalDevices[0].enumerateDeviceExtensionProperties();

	std::vector<const char*> deviceLayers;
	std::vector<LayerProperties> deviceLayerProps = physicalDevices[0].enumerateDeviceLayerProperties();

	//Unsupported extensions
	std::vector<const char*> removeExtensions = { "VK_KHX_external_memory_win32", "VK_KHX_external_semaphore",  "VK_KHX_external_semaphore_win32", "VK_KHX_external_memory", "VK_KHX_win32_keyed_mutex" };


	bool foundUnsupportedExtension = false;
	// Fill our extensions and names in
	for (auto& e : deviceExtProps)
	{
		foundUnsupportedExtension = false;

		for (auto& eSec : removeExtensions)
		{

			if (strcmp(e.extensionName, eSec) == 0)
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
	cmdPool = std::make_unique<CommandpoolVulkan>();
	cmdPool->Create(device, familyGraphicsIndex, CommandPoolCreateFlagBits::eResetCommandBuffer);
	commandBuffers = cmdPool->AllocateBuffer(device, CommandBufferLevel::ePrimary, NUM_FRAMES);
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
	// Create our uniform buffers
	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		UniformBufferVulkan tUniformBuff;

		CreateSimpleBuffer(allocator,
			tUniformBuff.allocation,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			tUniformBuff.buffer,
			vk::BufferUsageFlagBits::eUniformBuffer,
			sizeof(CBMatrix));

		CopyDataToBuffer(VkDevice(device), tUniformBuff.allocation, (void*)&matrixConstantBufferData, sizeof(matrixConstantBufferData));


		tUniformBuff.descriptorInfo.buffer = tUniformBuff.buffer;
		tUniformBuff.descriptorInfo.offset = 0;
		tUniformBuff.descriptorInfo.range = sizeof(matrixConstantBufferData);

		uniformBufferMVP.push_back(tUniformBuff);
	}

	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		UniformBufferVulkan tUniformBuff;

		CreateSimpleBuffer(allocator,
			tUniformBuff.allocation,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			tUniformBuff.buffer,
			vk::BufferUsageFlagBits::eUniformBuffer,
			sizeof(CBMatrix));

		CopyDataToBuffer(VkDevice(device), tUniformBuff.allocation, (void*)&lightConstantBufferData, sizeof(CBLights));


		tUniformBuff.descriptorInfo.buffer = tUniformBuff.buffer;
		tUniformBuff.descriptorInfo.offset = 0;
		tUniformBuff.descriptorInfo.range = sizeof(CBLights);

		uniformBufferLights.push_back(tUniformBuff);
	}
}

void UpdateUniformBufferTest(int32_t iCurrentBuff, const Camera& iCam, const std::vector<Light>& iLights)
{
	static float derp = 1.0f;
	derp += 0.01f;
	float derp2 = (sinf(derp) + 1.0f) / 2.0f;

	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 viewMatrix = iCam.GetViewMatrix();
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

	CopyDataToBuffer(VkDevice(device), uniformBufferMVP[iCurrentBuff].allocation, (void*)&matrixConstantBufferData, sizeof(matrixConstantBufferData));

	lightConstantBufferData.currAmountOfLights = std::min(static_cast<uint32_t>(iLights.size()), (uint32_t)16);

	for (int i = 0; i < lightConstantBufferData.currAmountOfLights; ++i)
	{
		lightConstantBufferData.lights[i] = iLights[i];
	}

	CopyDataToBuffer(VkDevice(device), uniformBufferLights[iCurrentBuff].allocation, (void*)&lightConstantBufferData, sizeof(lightConstantBufferData));

	//device.mapMemory(vk::DeviceMemory(uniformBufferMemory), vk::DeviceSize(0), vk::DeviceSize(mem_reqs.size), vk::MemoryMapFlagBits(0), (void**)&pData);
}

void SetupPipelineLayout()
{
	vk::DescriptorSetLayoutBinding pureSampler_layout_binding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(DescriptorType::eSampler)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);


	bindings = { pureSampler_layout_binding };

	vk::DescriptorSetLayoutCreateInfo descriptor_layout = vk::DescriptorSetLayoutCreateInfo()
		.setPNext(NULL)
		.setBindingCount(static_cast<uint32_t>(bindings.size()))
		.setPBindings(bindings.data());


	vk::DescriptorSetLayoutBinding uniform_binding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex |  vk::ShaderStageFlagBits::eFragment)
		.setPImmutableSamplers(NULL);

	vk::DescriptorSetLayoutBinding light_binding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
		.setPImmutableSamplers(NULL);

	uniformBinding = { uniform_binding, light_binding };


	vk::DescriptorSetLayoutCreateInfo descriptor_layoutUniform = vk::DescriptorSetLayoutCreateInfo()
		.setPNext(NULL)
		.setBindingCount(static_cast<uint32_t>(uniformBinding.size()))
		.setPBindings(uniformBinding.data());


	vk::DescriptorSetLayoutBinding pureImage_layout_binding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(DescriptorType::eSampledImage)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);



	normalTextureBinding = { pureImage_layout_binding };

	vk::DescriptorSetLayoutCreateInfo descripor_layoutTexture = vk::DescriptorSetLayoutCreateInfo()
		.setPNext(NULL)
		.setBindingCount(static_cast<uint32_t>(normalTextureBinding.size()))
		.setPBindings(normalTextureBinding.data());


	desc_layout.push_back(device.createDescriptorSetLayout(descriptor_layout));
	desc_layout.push_back(device.createDescriptorSetLayout(descriptor_layoutUniform));
	desc_layout.push_back(device.createDescriptorSetLayout(descripor_layoutTexture));

}

void SetupDescriptorSet()
{
	descriptorPool = std::make_unique<DescriptorPoolVulkan>();

	descriptorPool->Create(device, 400, 10, 10, 10, 400);

	for (int i = 0; i < 2; ++i)
	{
		std::vector<DescriptorSet> tDescSet;
		tDescSet.resize(NUM_DESCRIPTOR_SETS);

		tDescSet[0] = descriptorPool->AllocateDescriptorSet(device, 1, desc_layout[0], bindings)[0];
		tDescSet[1] = descriptorPool->AllocateDescriptorSet(device, 1, desc_layout[1], uniformBinding)[0];

		descriptor_set.push_back(tDescSet);
	}



	std::array<vk::WriteDescriptorSet, 1> textureWrites = {};

	for (auto& e : models)
	{
		e.textureSet = descriptorPool->AllocateDescriptorSet(device, 1, desc_layout[2], normalTextureBinding)[0];


		vk::DescriptorImageInfo pureImageInfo = {};

		//pureImageInfo.imageView = vk::ImageView(nullptr);
		pureImageInfo.imageView = e.texture.view;



		textureWrites[0] = {};
		textureWrites[0].pNext = NULL;
		textureWrites[0].dstSet = e.textureSet;
		textureWrites[0].descriptorCount = 1;
		textureWrites[0].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[0].pImageInfo = &pureImageInfo;
		textureWrites[0].dstArrayElement = 0;
		textureWrites[0].dstBinding = 0;

		device.updateDescriptorSets(static_cast<uint32_t>(textureWrites.size()), textureWrites.data(), 0, NULL);
	}

	//descriptor_set[1] = descriptorPool->AllocateDescriptorSet(device, 1, desc_layout[2], uniformBinding)[0];


	for (int i = 0; i < descriptor_set.size(); ++i)
	{
		std::array<vk::WriteDescriptorSet, 1> writes = {};


		// Create image info for the image descriptor
		vk::DescriptorImageInfo pureSamplerInfo = {};

		pureSamplerInfo.imageView = vk::ImageView(nullptr);
		pureSamplerInfo.sampler = testSampler;

		writes[0] = {};
		writes[0].pNext = NULL;
		writes[0].dstSet = descriptor_set[i][0];
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = vk::DescriptorType::eSampler;
		writes[0].pImageInfo = &pureSamplerInfo;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = 0;


		device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, NULL);

		std::array<vk::WriteDescriptorSet, 2> uniform_writes = {};

		uniform_writes[0] = {};
		uniform_writes[0].pNext = NULL;
		uniform_writes[0].dstSet = descriptor_set[i][1];
		uniform_writes[0].descriptorCount = 1;
		uniform_writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniform_writes[0].pBufferInfo = &uniformBufferMVP[i].descriptorInfo;
		uniform_writes[0].dstArrayElement = 0;
		uniform_writes[0].dstBinding = 0;

		uniform_writes[1] = {};
		uniform_writes[1].pNext = NULL;
		uniform_writes[1].dstSet = descriptor_set[i][1];
		uniform_writes[1].descriptorCount = 1;
		uniform_writes[1].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniform_writes[1].pBufferInfo = &uniformBufferLights[i].descriptorInfo;
		uniform_writes[1].dstArrayElement = 0;
		uniform_writes[1].dstBinding = 1;


		device.updateDescriptorSets(static_cast<uint32_t>(uniform_writes.size()), uniform_writes.data(), 0, NULL);
	}

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


void SetupShaders()
{
	auto vertexShader = CreateShader(device, "Shaders/vert.spv", "main", ShaderStageFlagBits::eVertex);
	auto fragmentShader = CreateShader(device ,"Shaders/frag.spv", "main", ShaderStageFlagBits::eFragment);

	shaders.push_back(vertexShader);
	shaders.push_back(fragmentShader);

	for (int i = 0; i < 2; ++i)
	{
		PipelineShaderStageCreateInfo shaderCInfo = vk::PipelineShaderStageCreateInfo()
			.setStage(shaders[i].shaderStage)
			.setPName(shaders[i].entryPointName.c_str())
			.setModule(shaders[i].shaderModule);

		shaderStages.push_back(shaderCInfo);
	}
}

void SetupFramebuffers()
{
	vk::ImageView attachments[3] = {};
	attachments[2] = postProcBuffer.view;
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
	vk::CommandBuffer tCmdBuffer = BeginSingleTimeCommands(device, cmdPool->GetPool());

	vk::BufferCopy copyRegion = vk::BufferCopy()
		.setSrcOffset(0)
		.setDstOffset(0)
		.setSize(aSize);

	tCmdBuffer.copyBuffer(srcBuffer, destBuffer, 1, &copyRegion);

	EndSingleTimeCommands(device, tCmdBuffer, cmdPool->GetPool(), graphicsQueue);
}

void CopyBufferToImage(vk::CommandBuffer iBuffer, vk::Buffer srcBuffer, vk::Image destImage, uint32_t width, uint32_t height)
{
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

	iBuffer.copyBufferToImage(
		srcBuffer,
		destImage,
		ImageLayout::eTransferDstOptimal,
		1,
		&region);
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

	oVertexBuffer.inputDescription.binding = 0;
	oVertexBuffer.inputDescription.inputRate = vk::VertexInputRate::eVertex;
	oVertexBuffer.inputDescription.stride = sizeof(VertexData);

	// 12 bits 
	// 8  bits 
	// 12 bits
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
		.setDynamicStateCount(1);

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



vk::Fence graphicsQueueFinishedFence;
void SetupSemaphores()
{
	vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();
	imageAcquiredSemaphore = device.createSemaphore(semaphoreInfo);
	rendererFinishedSemaphore = device.createSemaphore(semaphoreInfo);

	vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo();
	graphicsQueueFinishedFence = device.createFence(fenceInfo);
}


void SetupCommandBuffers(const vk::CommandBuffer& iBuffer, uint32_t index)
{
	vk::CommandBufferBeginInfo cmd_begin = vk::CommandBufferBeginInfo();

	iBuffer.begin(cmd_begin);

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
		.setFramebuffer(framebuffers[index])
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(screenWidth, screenHeight)))
		.setClearValueCount(2)
		.setPClearValues(clear_values);

	iBuffer.beginRenderPass(&rp_begin, SubpassContents::eInline);
	iBuffer.bindPipeline(PipelineBindPoint::eGraphics, pipeline);

	
	InitViewports(iBuffer);
	InitScissors(iBuffer);


	for (int j = 0; j < models.size(); ++j)
	{
		descriptor_set[index][2] = models[j].textureSet;
		iBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, NUM_DESCRIPTOR_SETS, descriptor_set[index].data(), 0, NULL);
		iBuffer.bindVertexBuffers(0, 1, &models[j].vertexBuffer.buffer, offsets);
		iBuffer.bindIndexBuffer(models[j].indexBuffer.buffer, 0, IndexType::eUint32);
		iBuffer.drawIndexed(models[j].indiceCount, 1, 0, 0, 0);
	}

	iBuffer.endRenderPass();
	// End the pipeline
	iBuffer.end();

}

void SetupRTTexture(
	vk::Device iDevice, 
	uint32_t iTextureWidth, uint32_t iTextureHeight, 
	vk::Image& oImage, VmaAllocation& oAllocation)
{
	CreateSimpleImage(allocator,
		oAllocation,
		VMA_MEMORY_USAGE_GPU_ONLY,
		ImageUsageFlagBits::eColorAttachment | ImageUsageFlagBits::eSampled,
		Format::eR8G8B8A8Unorm, ImageLayout::ePreinitialized,
		oImage, iTextureWidth, iTextureHeight);

}


void SetupTextureImage(vk::CommandBuffer iBuffer, vk::Device iDevice, std::string iFilePath, vk::Image& oImage, VmaAllocation& oAllocation, std::vector<BufferVulkan>& oStaging)
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

	TransitionImageLayout(iBuffer,
		oImage,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal);

	CopyBufferToImage(iBuffer, stagingBuffer.buffer, oImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	TransitionImageLayout(iBuffer,
		oImage,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal);


	oStaging.push_back(stagingBuffer);
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

bool firstFrame = true;
void RendererVulkan::Render()
{
	if (firstFrame)
	{
		SetupCommandBuffers(commandBuffers[currentBuffer], currentBuffer);
		firstFrame = false;
	}

	else
	{
		std::thread CommandSetup = std::thread(SetupCommandBuffers, commandBuffers[(currentBuffer + 1) % 2], (currentBuffer + 1) % 2);


		device.waitForFences(1, &graphicsQueueFinishedFence, vk::Bool32(true), FENCE_TIMEOUT);
		//LOG(INFO) << "FENCE WAITING OVER";
		LOG(INFO) << "Current buffer " << currentBuffer;
		device.resetFences(graphicsQueueFinishedFence);
		CommandSetup.join();
	}

	device.acquireNextImageKHR(swapchain.swapchain, UINT64_MAX, imageAcquiredSemaphore, vk::Fence(nullptr), &currentBuffer);
	vmaSetCurrentFrameIndex(allocator, currentBuffer);


	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::SubmitInfo submit_info[1] = {};
	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = &commandBuffers[currentBuffer];
	submit_info[0].signalSemaphoreCount = 1;
	submit_info[0].pSignalSemaphores = &rendererFinishedSemaphore;

	graphicsQueue.submit(1, submit_info, graphicsQueueFinishedFence);

	vk::PresentInfoKHR present;
	present.swapchainCount = 1;
	present.pSwapchains = &swapchain.swapchain;
	present.pImageIndices = &currentBuffer;
	present.pWaitSemaphores = &rendererFinishedSemaphore;	//
	present.waitSemaphoreCount = 1;
	present.pResults = 0;

	presentQueue.presentKHR(&present);

	//presentQueue.waitIdle();
}

void RendererVulkan::BeginFrame(const Camera& iCamera, const std::vector<Light>& lights)
{
	UpdateUniformBufferTest((currentBuffer + 1) % 2, iCamera, lights);
}

void RendererVulkan::Create()
{
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
	SetupRTTexture(device, screenWidth, screenHeight, postProcBuffer.image, postProcBuffer.allocation);
	postProcBuffer.view = CreateImageView(device, postProcBuffer.image, Format::eR8G8B8A8Unorm);

	SetupFramebuffers();
	SetupCommandBuffer();

	SetupDeviceQueue();
	
	vk::CommandBuffer stageBuffer = BeginSingleTimeCommands(device, cmdPool->GetPool());

	
	TransitionImageLayout(stageBuffer, depthBuffer.image, depthBuffer.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	// Staging buffers we will need to delete
	std::vector<BufferVulkan> stagingBuffers;
	for (auto& e : rawMeshData)
	{
		ModelVulkan tModV;
		SetupVertexBuffer(tModV.vertexBuffer, e);
		SetupIndexBuffer(tModV.indexBuffer, e);

		tModV.indiceCount = e.indices.size();

		SetupTextureImage(stageBuffer, device, e.filepaths[0].c_str(), tModV.texture.image, tModV.texture.allocation, stagingBuffers);
		tModV.texture.view = CreateImageView(device, tModV.texture.image, Format::eR8G8B8A8Unorm);
		models.push_back(tModV);
	}
	EndSingleTimeCommands(device, stageBuffer, cmdPool->GetPool(), graphicsQueue);

	for (auto& e : stagingBuffers)
	{
		vmaDestroyBuffer(allocator, e.buffer, e.allocation);
	}

	SetupPipeline();
	SetupSemaphores();

	CreateTextureSampler(device);

	SetupDescriptorSet();
}

void RendererVulkan::Destroy()
{
	
	presentQueue.waitIdle();
	for (auto& e : commandBuffers)
	{
		e.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	}

	descriptorPool->Destroy(device);

	device.destroySemaphore(rendererFinishedSemaphore);
	device.destroySemaphore(imageAcquiredSemaphore);

	device.destroySampler(testSampler);
	device.destroySampler(testImageSampler);


	device.destroyFence(graphicsQueueFinishedFence);

	for (auto& e : shaders)
	{
		device.destroyShaderModule(e.shaderModule);
	}
	
	cmdPool->Destroy(device);
	//device.destroyCommandPool(commandPool);
	//vice.destroySwapchainKHR(swapchain.swapchain);

	vmaDestroyImage(allocator, (VkImage)depthBuffer.image, depthBuffer.allocation);


	for (auto& e : desc_layout)
	{
		device.destroyDescriptorSetLayout(e);
	}

	for (auto& e : framebuffers)
	{
		device.destroyFramebuffer(e);
	}


	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipelineLayout);
	device.destroyRenderPass(renderPass);

	for (auto& e : uniformBufferMVP)
	{
		vmaDestroyBuffer(allocator, (VkBuffer)e.buffer, e.allocation);
	}

	for (auto& e : uniformBufferLights)
	{
		vmaDestroyBuffer(allocator, (VkBuffer)e.buffer, e.allocation);
	}

	for (auto& e : models)
	{
		vmaDestroyBuffer(allocator, e.vertexBuffer.buffer, e.vertexBuffer.allocation);
		vmaDestroyBuffer(allocator, e.indexBuffer.buffer, e.indexBuffer.allocation);
		vmaDestroyImage(allocator, e.texture.image, e.texture.allocation);
		device.destroyImageView(e.texture.view);
	}

	vmaDestroyImage(allocator, postProcBuffer.image, postProcBuffer.allocation);
	device.destroyImageView(postProcBuffer.view);
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
}