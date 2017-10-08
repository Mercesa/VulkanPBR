#pragma once
#include <string>
#include <ctype.h>

#include "RenderingIncludes.h"
#include "VulkanDataObjects.h"


class DeviceVulkan
{
public:
	DeviceVulkan();
	~DeviceVulkan();

	//void InitializeApplication();

	void CreateSwapchain(const int32_t& iWidth, const int32_t& iHeight);
	
	void CreateDevice();

	void CreateInstance(
		const std::string& iApplicationName, const uint32_t& iApplicationVersion, 
		const std::string& iEngineName, const uint32_t& iEngineVersion, 
		uint32_t iApiVersion);

	void CreateDebugCallbacks();
	//void SelectOptimalPhysicalDevice();
	
	void SetupDeviceQueue();

	vk::Instance instance;
	vk::Device device;
	vk::PhysicalDevice physicalDevice;
	SwapchainVulkan swapchain;


	uint32_t familyIndexGraphics;
	uint32_t familyIndexPresent;

	std::vector<vk::QueueFamilyProperties> familyProperties;
	std::vector<vk::PhysicalDevice> physicalDevices;

	vk::Queue graphicsQueue;
	vk::Queue presentQueue;


private:
};

