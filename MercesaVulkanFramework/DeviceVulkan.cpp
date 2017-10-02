#include "DeviceVulkan.h"

#include <iostream>

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

void DeviceVulkan::CreateDevice()
{

	std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

	assert(physicalDevices.size() != 0);

	physicalDevice = physicalDevices[0];

	std::vector<vk::QueueFamilyProperties> familyProperties;


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
