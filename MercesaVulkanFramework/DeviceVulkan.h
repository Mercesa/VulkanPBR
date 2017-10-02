#pragma once
#include <string>
#include <ctype.h>


#include "RenderingIncludes.h"

class DeviceVulkan
{
public:
	DeviceVulkan();
	~DeviceVulkan();

	void InitializeApplication();

	void CreateSwapchain();
	void CreateDevice();

	void CreateInstance(
		const std::string& iApplicationName, const uint32_t& iApplicationVersion, 
		const std::string& iEngineName, const uint32_t& iEngineVersion, 
		uint32_t iApiVersion);

	void SelectOptimalPhysicalDevice();
	
	vk::Instance instance;
	vk::Device device;
	vk::PhysicalDevice physicalDevice;

	uint32_t familyDeviceQueue;
	uint32_t familyPresentQueue;

private:
};

