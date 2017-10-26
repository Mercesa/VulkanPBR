#include "BackendVulkan.h"


#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include "RenderingIncludes.h"
#include "iLowLevelWindow.h"
#include "GLFWLowLevelWindow.h"
#include "easylogging++.h"

using namespace vk;

// Copy paste from vulkan helper, since otherwise 
inline bool CreateSimpleImage(
	VmaAllocator iAllocator,
	VmaAllocation& oAllocation,
	VmaMemoryUsage iMemUsage,
	vk::ImageUsageFlags iUsageFlags,
	vk::Format iFormat,
	vk::ImageLayout iLayout,
	vk::Image& oImage,
	uint32_t iWidth, uint32_t iHeight,
	vk::SampleCountFlagBits iSamples)
{

	vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
		.setImageType(ImageType::e2D)
		.setExtent(vk::Extent3D(iWidth, iHeight, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(iFormat)
		.setTiling(ImageTiling::eOptimal)
		.setInitialLayout(iLayout)
		.setUsage(iUsageFlags)
		.setSharingMode(SharingMode::eExclusive)
		.setSamples(iSamples);

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImage img = (VkImage)oImage;

	vmaCreateImage(iAllocator,
		&(VkImageCreateInfo)imageInfo,
		&alloc_info,
		&img,
		&oAllocation,
		nullptr);

	oImage = img;

	return true;
}


static std::vector<const char*> UnwantedInstanceLayers = {};
static std::vector<const char*> UnwantedInstanceExtensions = { "VK_KHR_get_surface_capabilities2", "VK_KHX_device_group_creation", "VK_KHR_external_fence_capabilities" };
static std::vector<const char*> UnwantedDeviceExtensions = {};

static const int swapInterval = 1;

static std::vector<const char*> g_deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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


// NOTE: Most of the functionality of this class is based on the rendering backend of the neo VK renderer of VKDoom3
// though I will take credit for converting it to the vulkan.hpp functionality :)

void BackendVulkan::Init(const GFXParams& iParams, iLowLevelWindow* const iWindow)
{
	context.currentParameters = iParams;

	CreateInstance(iWindow);
	
	CreateDebugCallbacks();

	CreateSurface(iWindow);
	
	EnumeratePhysicalDevices();
	
	SelectPhysicalDevice();
	
	CreateLogicalDeviceAndQueues();

	CreateMemoryAllocator();

	CreateSemaphores();

	CreateSwapChain();
	
	CreateRenderTargets();
	
	CreateDepthbuffer();

	CreateRenderpass();

	CreateFramebuffers();

	CreateCommandPool();

	CreateCommandBuffer();
}

void BackendVulkan::Shutdown()
{

	DestroyFramebuffers();
	DestroyDepthBuffer();
	DestroyRenderTargets();
	DestroySemaphores(); 
	context.device.destroyRenderPass(context.renderpass);

	vmaDestroyAllocator(allocator);
	DestroySwapchain();
	context.device.destroy();
	instance.destroy();
}


void BackendVulkan::GetInstanceLayers(std::vector<const char*>& iResult)
{
	//std::vector<LayerProperties> layers;//enumerateInstanceLayerProperties();

#ifdef _DEBUG
	iResult.push_back("VK_LAYER_LUNARG_standard_validation");
	//layerNames.push_back("VK_LAYER_LUNARG_core_validation");
	//layerNames.push_back("VK_LAYER_LUNARG_parameter_validation");
	//layerNames.push_back("VK_LAYER_RENDERDOC_Capture");
#endif

}


// currently broken, returns garbage in the vector itself
void BackendVulkan::GetInstanceExtensions(std::vector<const char*>& iResult)
{
	std::vector<vk::ExtensionProperties> extProps = vk::enumerateInstanceExtensionProperties();
	iResult = getAvailableWSIExtensions();


	// Find an unwanted extension and skip them in the adding process
	bool foundUnwantedExtension = false;
	for (auto& e : extProps)
	{
		foundUnwantedExtension = false;
		for (auto& eTwo : UnwantedInstanceExtensions)
		{
			if (strcmp(e.extensionName, eTwo) == 0)
			{
				foundUnwantedExtension = true;
			}
		}

		if (foundUnwantedExtension == false)
		{
			iResult.push_back(e.extensionName);
		}
	}

}

std::vector<const char*> BackendVulkan::GetDeviceExtensions()
{
	return std::vector<const char*>();
}

// Boilerplate code

void BackendVulkan::CreateInstance(iLowLevelWindow* const iWindow)
{
	vk::ApplicationInfo appInfo = vk::ApplicationInfo()
		.setPApplicationName("Vulkan")
		.setApplicationVersion(1)
		.setPEngineName("Engine")
		.setEngineVersion(1)
		.setApiVersion(VK_API_VERSION_1_0);


	std::vector<vk::ExtensionProperties> extProps = vk::enumerateInstanceExtensionProperties();
	instanceExtensions = getAvailableWSIExtensions();
	GetInstanceLayers(instanceLayers);
	deviceExtensions = g_deviceExtensions;

	// Find an unwanted extension and skip them in the adding process
	bool foundUnwantedExtension = false;
	for (auto& e : extProps)
	{
		foundUnwantedExtension = false;
		for (auto& eTwo : UnwantedInstanceExtensions)
		{
			if (strcmp(e.extensionName, eTwo) == 0)
			{
				foundUnwantedExtension = true;
			}
		}

		if (foundUnwantedExtension == false)
		{
			instanceExtensions.push_back(e.extensionName);
		}
	}


	// Add required window extensions
	for (auto& e : iWindow->GetRequiredExtensions())
	{
		instanceExtensions.push_back(e);
	}

	// vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
	// are needed.
	vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
		.setFlags(vk::InstanceCreateFlags())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()))
		.setPpEnabledExtensionNames(instanceExtensions.data())
		.setEnabledLayerCount(static_cast<uint32_t>(instanceLayers.size()))
		.setPpEnabledLayerNames(instanceLayers.data());

	// Create instance
	try {
		instance = vk::createInstance(instInfo);
	}
	catch (const std::exception& e) {
		std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
	}
}


void BackendVulkan::CreateSurface(iLowLevelWindow* const iWindow)
{
	GLFWLowLevelWindow* tempWindow = dynamic_cast<GLFWLowLevelWindow*>(iWindow);

	try {
			VkSurfaceKHR tSurface;
		
			glfwCreateWindowSurface(
				instance, 
				tempWindow->window, nullptr, &tSurface);
		
			surface = tSurface;
		}

	catch (const std::exception& e) {
		std::cout << "Failed to create Vulkan surface: " << e.what() << std::endl;
		instance.destroy();
		return;
	}
}

void BackendVulkan::EnumeratePhysicalDevices()
{
	std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

	context.gpus.resize(devices.size());

	for (uint32_t i = 0; i < devices.size(); ++i)
	{
		GPUinfo& gpu = context.gpus[i];
		gpu.device = devices[i];


		// Query family and extension properties
		{
			gpu.queueFamilyProps =  gpu.device.getQueueFamilyProperties();
			LOG_IF(gpu.queueFamilyProps.size() == 0, ERROR) << "Backend::EnumeratePhysicalDevices - device returned zero queues";
		}

		{
			gpu.extensionProps = gpu.device.enumerateDeviceExtensionProperties();
			LOG_IF(gpu.extensionProps.size() == 0, ERROR) << "Backend::EnumeratePhysicalDevices - device returned zero extensions";
		}

		gpu.surfaceCaps = gpu.device.getSurfaceCapabilitiesKHR(surface);
		
		// Query surface formats and present modes
		{
			gpu.surfaceFormats = gpu.device.getSurfaceFormatsKHR(surface);
			LOG_IF(gpu.surfaceFormats.size() == 0, ERROR) << "Backend::EnumeratePhysicalDevices - device returned zero surface formats";
		}

		{
			gpu.presentModes = gpu.device.getSurfacePresentModesKHR(surface);
			LOG_IF(gpu.presentModes.size() == 0, ERROR) << "Backend::EnumeratePhysicalDevices - device returned zero presentmodes";
		}

		gpu.memProps = gpu.device.getMemoryProperties();
		gpu.props = gpu.device.getProperties();

	}
}
#include <string>

static bool CheckPhysicalDeviceExtensionSupport(GPUinfo& iGPU, std::vector<const char*>& iRequiredExtensions)
{
	int required = static_cast<int>(iRequiredExtensions.size());
	int available = 0;

	for (int i = 0; i < iRequiredExtensions.size(); ++i)
	{
		for (int j = 0; j < iGPU.extensionProps.size(); ++j)
		{
			if (std::string(iRequiredExtensions[i]) == std::string(iGPU.extensionProps[j].extensionName))
			{
				available++;
				break;
			}
		}
	}

	// Make sure to log if the GPU does not meet the required extensions
	LOG_IF(available != required, INFO) << "CheckPhysicalDeviceExtensionSupport with GPU: " << iGPU.device.getProperties().deviceName << " does not support all required extensions!";

	return available == required;
}

void BackendVulkan::SelectPhysicalDevice()
{
	for (int i = 0; i < context.gpus.size(); ++i)
	{
		GPUinfo& gpu = context.gpus[i];

		int graphicsID = -1;
		int presentID = -1;

		if (!CheckPhysicalDeviceExtensionSupport(gpu, deviceExtensions)) 
		{
			continue;
		}

		if (gpu.surfaceFormats.size() == 0)
		{
			continue;
		}

		if (gpu.presentModes.size() == 0)
		{
			continue;
		}
	
		for (int j = 0; j < gpu.queueFamilyProps.size(); ++j)
		{
			vk::QueueFamilyProperties& props = gpu.queueFamilyProps[j];

			if (props.queueCount == 0)
			{
				continue;
			}

			if (props.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				graphicsID = j;
				break;
			}
		}

		for (int j = 0; j < gpu.queueFamilyProps.size(); ++j)
		{
			vk::QueueFamilyProperties& props = gpu.queueFamilyProps[j];

			if (props.queueCount == 0)
			{
				continue;
			}

			VkBool32 supportsPresent = VK_FALSE;
			gpu.device.getSurfaceSupportKHR(j, surface, &supportsPresent);
			if (supportsPresent)
			{
				presentID = j;
				break;
			}
		}

		if (graphicsID >= 0 && presentID >= 0)
		{
			context.graphicsFamilyIndex = graphicsID;
			context.presentFamilyIndex = presentID;
			physicalDevice = gpu.device;
			context.gpu = &gpu;
			physicalDeviceFeatures = physicalDevice.getFeatures();

			vk::PhysicalDeviceProperties deviceProps = gpu.device.getProperties();
			LOG(INFO) << "SelectPhysicalDevice: Found a suitable device\n" << "Vendor: " << deviceProps.vendorID << "\n" << "Name: " << deviceProps.deviceName << "\n" << "Driver version: " << deviceProps.driverVersion;


			return;
		}
	}

	LOG(FATAL) << "Could not find a physical device which fits our desired profile";
}


void BackendVulkan::CreateLogicalDeviceAndQueues()
{
	std::map<int, int> uniqueFamilyIDS;

	uniqueFamilyIDS[context.graphicsFamilyIndex] += 1;
	uniqueFamilyIDS[context.presentFamilyIndex] += 1;

	std::vector<vk::DeviceQueueCreateInfo> devQCreateInfo;

	const float priority = 1.0f;

	for (auto& e : uniqueFamilyIDS)
	{
		vk::DeviceQueueCreateInfo qInfo = vk::DeviceQueueCreateInfo()
			.setQueueFamilyIndex(e.first)
			.setQueueCount(1)
			.setPQueuePriorities(&priority);

		devQCreateInfo.push_back(qInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures;
	deviceFeatures.textureCompressionBC = VK_TRUE;
	deviceFeatures.imageCubeArray = VK_TRUE;
	deviceFeatures.depthClamp = VK_TRUE;
	deviceFeatures.depthBiasClamp = VK_TRUE;
	deviceFeatures.depthBounds = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	vk::DeviceCreateInfo info = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(static_cast<uint32_t>(devQCreateInfo.size()))
		.setPQueueCreateInfos(devQCreateInfo.data())
		.setPEnabledFeatures(&deviceFeatures)
		.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
		.setPpEnabledExtensionNames(deviceExtensions.data())
		.setEnabledLayerCount(0); // Device layers are deprecated and need to be enabled if the user wants backwards compatibilit. Me, being the user. Does not care about backwards compatibility in this project

	context.device = physicalDevice.createDevice(info);

	context.graphicsQueue	= context.device.getQueue(context.graphicsFamilyIndex, 0);
	context.presentQueue	= context.device.getQueue(context.presentFamilyIndex, 0);

	LOG(INFO) << "CreateLogicalDeviceAndQueues: Succesfully created device and queues";
}

void BackendVulkan::CreateMemoryAllocator()
{
	VmaAllocatorCreateInfo createInfo = VmaAllocatorCreateInfo();
	createInfo.device = VkDevice(context.device);
	createInfo.physicalDevice = VkPhysicalDevice(context.gpu->device);
	createInfo.flags = VMA_ALLOCATOR_FLAG_BITS_MAX_ENUM;

	// Create custom allocator
	vmaCreateAllocator(&createInfo, &allocator);
}


void BackendVulkan::CreateSemaphores()
{
	vk::SemaphoreCreateInfo semaphoreCreateInfo;

	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		acquireSemaphores.push_back(context.device.createSemaphore(semaphoreCreateInfo));
		renderCompleteSemaphore.push_back(context.device.createSemaphore(semaphoreCreateInfo));

	}
}

void BackendVulkan::DestroySemaphores()
{
	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		context.device.destroySemaphore(acquireSemaphores[i]);
		context.device.destroySemaphore(renderCompleteSemaphore[i]);
	}
}


vk::SurfaceFormatKHR ChooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR>& iFormats)
{
	vk::SurfaceFormatKHR result;

	if (iFormats.size() == 0 && iFormats[0].format == vk::Format::eUndefined)
	{
		result.format = vk::Format::eB8G8R8A8Unorm;
		result.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
		return result;
	}

	for (int i = 0; i < iFormats.size(); ++i)
	{
		vk::SurfaceFormatKHR& fmt = iFormats[i];
		if (fmt.format == vk::Format::eB8G8R8A8Unorm && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return fmt;
		}
	}

	return iFormats[0];
}

vk::PresentModeKHR ChoosePresentMode(std::vector<vk::PresentModeKHR>& iModes)
{
	vk::PresentModeKHR desiredMode = vk::PresentModeKHR::eFifo;

	if (swapInterval < 1)
	{
		for (int i = 0; i < iModes.size(); ++i)
		{
			if (iModes[i] == vk::PresentModeKHR::eMailbox)
			{
				return vk::PresentModeKHR::eMailbox;
			}

			if ((iModes[i] != vk::PresentModeKHR::eMailbox) && (iModes	[i] == vk::PresentModeKHR::eImmediate))
			{
				return vk::PresentModeKHR::eImmediate;
			}
		}
	}

	for (int i = 0; i < iModes.size(); ++i)
	{
		if (iModes[i] == desiredMode)
		{
			return desiredMode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}


vk::Extent2D ChooseSurfaceExtent(vk::SurfaceCapabilitiesKHR& iCapabilities, const uint32_t& iWidth, const uint32_t& iHeight)
{
	vk::Extent2D extent;

	// Width and height are either both 0xFFFFFFFFF or both not 0xFFFFFFFF
	if (iCapabilities.currentExtent.width == 0xFFFFFFFF)
	{
		extent.width = iWidth;
		extent.height = iHeight;

		// Wrap the boundaries
		extent.width = std::max(extent.width, iCapabilities.minImageExtent.width);
		extent.width = std::min(extent.width, iCapabilities.maxImageExtent.width);

		extent.width = std::max(extent.height, iCapabilities.minImageExtent.height);
		extent.width = std::min(extent.height, iCapabilities.maxImageExtent.height);
	}
	else
	{
		extent = iCapabilities.currentExtent;
	}

	return extent;
}

void BackendVulkan::CreateSwapChain()
{
	GPUinfo & gpu = *context.gpu;

	vk::SurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(gpu.surfaceFormats);
	vk::PresentModeKHR  tPresentMode = ChoosePresentMode(gpu.presentModes);
	vk::Extent2D extent = ChooseSurfaceExtent(gpu.surfaceCaps, context.currentParameters.width, context.currentParameters.height);

	vk::SwapchainCreateInfoKHR info = vk::SwapchainCreateInfoKHR()
		.setSurface(surface)
		.setMinImageCount(NUM_FRAMES)
		.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc);

	// If families are not the same, put sharing mode to concurrent mode
	if (context.graphicsFamilyIndex != context.presentFamilyIndex)
	{
		uint32_t indices[] = { (uint32_t)context.graphicsFamilyIndex, (uint32_t)context.presentFamilyIndex };

		info.imageSharingMode = vk::SharingMode::eConcurrent;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = indices;
	}
	else
	{
		info.imageSharingMode = vk::SharingMode::eExclusive;
	}

	info.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	info.presentMode = tPresentMode;
	info.clipped = VK_TRUE;

	swapchain = context.device.createSwapchainKHR(info);

	swapchainFormat = surfaceFormat.format;
	presentMode = tPresentMode;
	swapchainExtent = extent;
	fullscreen = (bool)context.currentParameters.fullscreen;

	swapchainImages = context.device.getSwapchainImagesKHR(swapchain);

	LOG_IF(swapchainImages.size() < 1, ERROR) << "CreateSwapchain returned zero swapchain images present.";


	for (uint32_t i = 0; i < NUM_FRAMES; ++i)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = swapchainFormat;
		imageViewCreateInfo.components.r = vk::ComponentSwizzle::eR;
		imageViewCreateInfo.components.g = vk::ComponentSwizzle::eG;
		imageViewCreateInfo.components.b = vk::ComponentSwizzle::eB;
		imageViewCreateInfo.components.a = vk::ComponentSwizzle::eA;
		imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		swapchainViews.push_back(context.device.createImageView(imageViewCreateInfo));
	}

	LOG(INFO) << "CreateSwapchain: Swapchain succesfully created";

}

void BackendVulkan::DestroySwapchain()
{
	for (uint32_t i = 0; i < NUM_FRAMES; ++i)
	{
		context.device.destroyImageView(swapchainViews[i]);	
	}

	swapchainViews.resize(0);

	context.device.destroySwapchainKHR(swapchain);
}




static VKAPI_ATTR VkBool32 VKAPI_CALL dbgFunc(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
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

	std::cout << message.str() << std::endl;

	/*
	* false indicates that layer should not bail-out of an
	* API call that had validation failures. This may mean that the
	* app dies inside the driver due to invalid parameter(s).
	* That's what would happen without validation layers, so we'll
	* keep that behavior here.
	*/
	return false;
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

void BackendVulkan::CreateDebugCallbacks()
{
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	createInfo.pfnCallback = dbgFunc;

	if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callBack) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug callback!");
	}

	
}

void BackendVulkan::CreateDepthbuffer()
{
	vk::ImageCreateInfo image_Info = {};
	const vk::Format depth_format = vk::Format::eD24UnormS8Uint;
	
	// Query if the format we supplied is supported by our device
	vk::FormatProperties props = physicalDevice.getFormatProperties(depth_format);
	
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
	
	
	CreateSimpleImage(allocator, depthAllocation,
		VMA_MEMORY_USAGE_GPU_ONLY,
		ImageUsageFlagBits::eDepthStencilAttachment,
		vk::Format::eD24UnormS8Uint,
		vk::ImageLayout::eUndefined,
		depthImage,
		context.currentParameters.width, context.currentParameters.height,
		context.sampleCount);
	
	
	vk::ImageViewCreateInfo view_info = vk::ImageViewCreateInfo()
		.setPNext(NULL)
		.setImage(vk::Image(nullptr))
		.setFormat(depth_format)
		.setComponents(vk::ComponentMapping(ComponentSwizzle::eR, ComponentSwizzle::eG, ComponentSwizzle::eB, ComponentSwizzle::eA))
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1))
		.setViewType(ImageViewType::e2D);
	
	view_info.image = depthImage;
	context.depthFormat= Format::eD24UnormS8Uint;
	
	depthView = context.device.createImageView(view_info);
}

void BackendVulkan::DestroyDepthBuffer()
{
	vmaDestroyImage(allocator, depthImage, depthAllocation);
	context.device.destroyImageView(depthView);
}

void BackendVulkan::CreateRenderTargets()
{
	vk::ImageFormatProperties fmtProps = {};

	physicalDevice.getImageFormatProperties(swapchainFormat, vk::ImageType::e2D, ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageCreateFlagBits(0));

	
	const int samples = (int)MULTISAMPLES;

	if (samples >= 16 && (fmtProps.sampleCounts & vk::SampleCountFlagBits::e16)){
		context.sampleCount = SampleCountFlagBits::e16;
	}

	else if (samples >= 8 && (fmtProps.sampleCounts & vk::SampleCountFlagBits::e8)){
		context.sampleCount = SampleCountFlagBits::e8;
	}

	else if (samples >= 4 && (fmtProps.sampleCounts & vk::SampleCountFlagBits::e4)){
		context.sampleCount = SampleCountFlagBits::e4;
	}

	else if (samples >= 2 && (fmtProps.sampleCounts & vk::SampleCountFlagBits::e2)){
		context.sampleCount = SampleCountFlagBits::e2;
	}

	if (context.sampleCount > vk::SampleCountFlagBits::e1)
	{
		context.supersampling = physicalDeviceFeatures.sampleRateShading == VK_TRUE;

		vk::ImageCreateInfo createInfo = vk::ImageCreateInfo()
			.setFormat(swapchainFormat)
			.setExtent(vk::Extent3D(swapchainExtent.width, swapchainExtent.height, 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(context.sampleCount)
			.setTiling(ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled)
			.setInitialLayout(vk::ImageLayout::eUndefined);


		msaaImage = context.device.createImage(createInfo);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VkImage tImage = msaaImage;

		vmaCreateImage(allocator, &(VkImageCreateInfo)createInfo, &alloc_info, &tImage, &msaaVmaAllocation, nullptr);

		msaaImage = tImage;


		vk::ImageViewCreateInfo viewInfo = {};
		viewInfo.format = swapchainFormat;
		viewInfo.image = msaaImage;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		msaaImageView = context.device.createImageView(viewInfo);
	}

	context.sampleCount = vk::SampleCountFlagBits(samples);
}

void BackendVulkan::DestroyRenderTargets()
{
	context.device.destroyImageView(msaaImageView, NULL);

	vmaDestroyImage(allocator, msaaImage, msaaVmaAllocation);

	msaaAllocation = VmaAllocationInfo();
	msaaVmaAllocation = NULL;
}

void BackendVulkan::CreateFramebuffers()
{
	vk::ImageView attachments[3] = {};

	if (depthView == vk::ImageView(nullptr))
	{
		LOG(FATAL) << "CreateFramebuffers: Depth view not present";
	}
	else
	{
		attachments[1] = depthView;
	}

	const bool resolve = context.sampleCount > vk::SampleCountFlagBits::e1;
	if (resolve)
	{
		LOG(INFO) << "CreateFramebuffers: Contains multisample resolve, adding to attachments";
		attachments[2] = msaaImageView;
	}

	vk::FramebufferCreateInfo framebufferCreateInfo = vk::FramebufferCreateInfo()
		.setRenderPass(context.renderpass)
		.setAttachmentCount(resolve ? 3 : 2)
		.setPAttachments(attachments)
		.setWidth(context.currentParameters.width)
		.setHeight(context.currentParameters.height)
		.setLayers(1);

	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		attachments[0] = swapchainViews[i];
		framebuffers.push_back(context.device.createFramebuffer(framebufferCreateInfo, nullptr));
	}

	LOG(INFO) << "CreateFramebuffers: Succesfully created framebuffers";
}

void BackendVulkan::DestroyFramebuffers()
{
	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		context.device.destroyFramebuffer(framebuffers[i], nullptr);
	}
}

void BackendVulkan::CreateRenderpass()
{
	std::vector<vk::AttachmentDescription> attachments;

	const bool resolve = context.sampleCount > vk::SampleCountFlagBits::e1;

	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = swapchainFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eGeneral;

	attachments.push_back(colorAttachment);

	vk::AttachmentDescription depthAttachment;
	depthAttachment.format = context.depthFormat;
	depthAttachment.samples = context.sampleCount;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	attachments.push_back(depthAttachment);

	vk::AttachmentDescription resolveAttachment;
	resolveAttachment.format = swapchainFormat;
	resolveAttachment.samples = context.sampleCount;
	resolveAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	resolveAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	resolveAttachment.initialLayout = vk::ImageLayout::eUndefined;
	resolveAttachment.finalLayout = vk::ImageLayout::eGeneral;

	attachments.push_back(resolveAttachment);

	vk::AttachmentReference colorRef = {};
	colorRef.attachment = resolve ? 2 : 0;
	colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthRef = {};
	depthRef.attachment = 1;
	depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference resolveRef = {};
	resolveRef.attachment = 0;
	resolveRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass = vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorRef)
		.setPDepthStencilAttachment(&depthRef);

	if (resolve)
	{
		subpass.pResolveAttachments = &resolveRef;
	}

	vk::RenderPassCreateInfo renderpassCreateInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(resolve ? 3 : 2)
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(0);

	context.renderpass = context.device.createRenderPass(renderpassCreateInfo);
}

void BackendVulkan::BeginFrame()
{
	context.commandBuffer = context.commandBuffers[context.currentFrame];
	context.device.acquireNextImageKHR(swapchain, UINT64_MAX, acquireSemaphores[context.currentFrame], vk::Fence(nullptr), &currentSwapIndex);

	vk::CommandBufferBeginInfo cmdBufferBeginInfo = vk::CommandBufferBeginInfo();

	context.commandBuffer.begin(cmdBufferBeginInfo);

	vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(context.renderpass)
		.setFramebuffer(framebuffers[currentSwapIndex]);

	renderPassBeginInfo.renderArea.extent = swapchainExtent;

	context.commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
}

void BackendVulkan::EndFrame()
{
	context.commandBuffer.endRenderPass();

	vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier()
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(swapchainImages[currentSwapIndex])
		.setOldLayout(vk::ImageLayout::eGeneral)
		.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstAccessMask(vk::AccessFlagBits(0));

	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	context.commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eColorAttachmentOutput, 
		vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlagBits(0),
		(uint32_t)0, NULL, 0, NULL, (uint32_t)1, &barrier);

	context.commandBuffer.end();
	context.commandBufferRecorded[context.currentFrame] = true;

	vk::Semaphore* acquire = &acquireSemaphores[context.currentFrame];
	vk::Semaphore* finished = &renderCompleteSemaphore[context.currentFrame];

	vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::SubmitInfo subInfo = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(&context.commandBuffer)
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(acquire)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(finished)
		.setPWaitDstStageMask(&dstStageMask);


	context.graphicsQueue.submit(1, &subInfo, context.commandBufferFences[context.currentFrame]);
}

void BackendVulkan::BlockSwapBuffers()
{
	if (context.commandBufferRecorded[context.currentFrame] == false)
	{
		return;
	}

	context.device.waitForFences(context.commandBufferFences[context.currentFrame], VK_TRUE, UINT64_MAX);
	context.device.resetFences(context.commandBufferFences[context.currentFrame]);
	context.commandBufferRecorded[context.currentFrame] = false;

	vk::Semaphore* finished = &renderCompleteSemaphore[context.currentFrame];

	vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(finished)
		.setSwapchainCount(1)
		.setPSwapchains(&swapchain)
		.setPImageIndices(&currentSwapIndex);

	context.presentQueue.presentKHR(presentInfo);
	
	context.counter++;
	context.currentFrame = context.counter % NUM_FRAMES;
}

void BackendVulkan::CreateCommandPool()
{
	vk::CommandPoolCreateInfo cmdPoolCI = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(context.graphicsFamilyIndex)
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	context.cmdPool = context.device.createCommandPool(cmdPoolCI);
}

void BackendVulkan::CreateCommandBuffer()
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
		.setLevel(CommandBufferLevel::ePrimary)
		.setCommandPool(context.cmdPool)
		.setCommandBufferCount(NUM_FRAMES);

	context.commandBuffers = context.device.allocateCommandBuffers(commandBufferAllocateInfo);


	vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo();

	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		context.commandBufferFences.push_back(context.device.createFence(fenceCreateInfo));
	}

	context.commandBufferRecorded.resize(NUM_FRAMES);

	for (auto& e : context.commandBufferRecorded)
	{
		e = false;
	}
}