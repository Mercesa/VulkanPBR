#include "DeviceVulkan.h"
#define NOMINMAX

#include <iostream>
#include <algorithm>


DeviceVulkan::DeviceVulkan()
{
}


DeviceVulkan::~DeviceVulkan()
{
}

// Get available extensions for WSI
inline std::vector<const char*> getAvailableWSIExtensions()
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

std::vector<const char*> layerNames;
std::vector<const char*> extensions;

using namespace vk;
void DeviceVulkan::CreateSwapchain(const int32_t& iWidth, const int32_t& iHeight)
{
	familyProperties = physicalDevice.getQueueFamilyProperties();
	vk::Bool32* pSupportsPresent = (vk::Bool32*)malloc(familyProperties.size() * sizeof(vk::Bool32));

	// Iterate over each queue 
	for (uint32_t i = 0; i < familyProperties.size(); ++i)
	{
		physicalDevice.getSurfaceSupportKHR(i, swapchain.surface, &pSupportsPresent[i]);
	}


	familyIndexGraphics = UINT32_MAX;
	familyIndexPresent = UINT32_MAX;

	for (uint32_t i = 0; i < familyProperties.size(); ++i)
	{
		if ((familyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
		{
			if (familyIndexGraphics == UINT32_MAX)
			{
				familyIndexGraphics = i;
			}

			if (pSupportsPresent[i] == VK_TRUE) {
				familyIndexGraphics = i;
				familyIndexPresent = i;
				break;
			}
		}
	}

	// Find a seperate queue for present if there is no family that supports both
	if (familyIndexPresent == UINT32_MAX)
	{
		for (int i = 0; i < familyProperties.size(); ++i)
		{
			if (pSupportsPresent[i] == VK_TRUE)
			{
				familyIndexPresent = i;
				break;
			}
		}
	}

	free(pSupportsPresent);

	if (familyIndexGraphics == UINT32_MAX && familyIndexGraphics == UINT32_MAX)
	{
		std::cout << "Could not find queues for graphics and present\n";
		exit(-1);
	}

	// get the list of of supported formats
	std::vector<SurfaceFormatKHR> surfaceFormats = physicalDevice.getSurfaceFormatsKHR(swapchain.surface);

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
	surfCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(swapchain.surface);
	presentModes = physicalDevice.getSurfacePresentModesKHR(swapchain.surface);

	vk::Extent2D swapchainExtent;

	// Width and height are either both 0xFFFFFFFFF or both not 0xFFFFFFFF
	if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
	{
		swapchainExtent.width = iWidth;
		swapchainExtent.height = iHeight;

		// Wrap the boundaries
		swapchainExtent.width = max(swapchainExtent.width, surfCapabilities.minImageExtent.width);
		swapchainExtent.width = min(swapchainExtent.width, surfCapabilities.maxImageExtent.width);

		swapchainExtent.width = max(swapchainExtent.height, surfCapabilities.minImageExtent.height);
		swapchainExtent.width = min(swapchainExtent.height, surfCapabilities.maxImageExtent.height);
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

	uint32_t queueFamilyIndices[2] = { familyIndexGraphics, familyIndexPresent };

	if (familyIndexGraphics != familyIndexPresent)
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

void DeviceVulkan::CreateDevice()
{

	physicalDevices = instance.enumeratePhysicalDevices();

	assert(physicalDevices.size() != 0);

	physicalDevice = physicalDevices[0];


	familyProperties = physicalDevice.getQueueFamilyProperties();

	assert(familyProperties.size() >= 1);

	float queue_priorities[1] = { 0.0 };

	std::vector<const char*> deviceExtensions;
	std::vector<vk::ExtensionProperties> deviceExtProps = physicalDevice.enumerateDeviceExtensionProperties();

	std::vector<const char*> deviceLayers;
	std::vector<vk::LayerProperties> deviceLayerProps = physicalDevice.enumerateDeviceLayerProperties();

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

	vk::DeviceQueueCreateInfo queueInfo = vk::DeviceQueueCreateInfo()
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

	device = physicalDevice.createDevice(deviceInfo);
}

void DeviceVulkan::CreateInstance(const std::string& iApplicationName, const uint32_t& iApplicationVersion, const std::string& iEngineName, const uint32_t& iEngineVersion, uint32_t iApiVersion)
{
	vk::ApplicationInfo appInfo = vk::ApplicationInfo()
		.setPApplicationName(iApplicationName.c_str())
		.setApplicationVersion(iApplicationVersion)
		.setPEngineName(iEngineName.c_str())
		.setEngineVersion(iEngineVersion)
		.setApiVersion(iApiVersion);


	//for (auto &e : layers)
	//{
	//	std::cout << e.layerName << std::endl;
	//	//layerNames.push_back(e.layerName);
	//}

#ifdef _DEBUG
	layerNames.push_back("VK_LAYER_LUNARG_standard_validation");
	layerNames.push_back("VK_LAYER_LUNARG_core_validation");
#endif;
	extensions = getAvailableWSIExtensions();

	std::vector<const char*> removeExtensions = { "VK_KHR_get_surface_capabilities2", "VK_KHX_device_group_creation", "VK_KHR_external_fence_capabilities" };

	std::vector<vk::ExtensionProperties> extProps = vk::enumerateInstanceExtensionProperties();

	// Find an unwanted extension and skip them in the adding process
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

//VkDebugReportCallbackEXT callback;
//
//PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
//PFN_vkDebugReportMessageEXT DebugReportMessageCallback = VK_NULL_HANDLE;
//PFN_vkDestroyDebugReportCallbackEXT dbgReportCallBack = VK_NULL_HANDLE;
//
//
//VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
//	VkDebugReportFlagsEXT       flags,
//	VkDebugReportObjectTypeEXT  objectType,
//	uint64_t                    object,
//	size_t                      location,
//	int32_t                     messageCode,
//	const char*                 pLayerPrefix,
//	const char*                 pMessage,
//	void*                       pUserData)
//{
//	std::cerr << pMessage << std::endl;
//	return VK_FALSE;
//}



void DeviceVulkan::CreateDebugCallbacks()
{

	//VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
	//callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	//callbackCreateInfo.pNext = nullptr;
	//callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
	//	VK_DEBUG_REPORT_WARNING_BIT_EXT |
	//	VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	//callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
	//callbackCreateInfo.pUserData = nullptr;
	//
	///* Register the callback */
	//
	//VkInstance tempInst = VkInstance(instance);
	//callback = instance.createDebugReportCallbackEXT(callbackCreateInfo);
	//PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
	//	reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
	//	(vkGetInstanceProcAddr(tempInst, "vkCreateDebugReportCallbackEXT"));
	//
	//PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT =
	//	reinterpret_cast<PFN_vkDebugReportMessageEXT>
	//	(vkGetInstanceProcAddr(tempInst, "vkDebugReportMessageEXT"));
	//PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
	//	reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
	//	(vkGetInstanceProcAddr(tempInst, "vkDestroyDebugReportCallbackEXT"));
	//
	//VkResult result = vkCreateDebugReportCallbackEXT(tempInst, &callbackCreateInfo, nullptr, &callback);
}

void DeviceVulkan::SetupDeviceQueue()
{
	graphicsQueue = device.getQueue(familyIndexGraphics, 0);

	if (familyIndexGraphics == familyIndexPresent)
	{
		presentQueue = graphicsQueue;
	}
	else
	{
		presentQueue = device.getQueue(familyIndexGraphics, 0);
	}
}
