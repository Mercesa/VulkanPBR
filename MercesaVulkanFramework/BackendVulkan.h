#pragma once

#include "RenderingIncludes.h"

// Backend has all the necessary data, instance, swapchain etc
class BackendVulkan
{

public:
	BackendVulkan() = default;
	~BackendVulkan() = default;

	void Init();
	void Shutdown();

private:

	void CreateInstance();
	void CreatePhysicalDevice();
	void CreateLogicalDevice();

	vk::Instance instance;
	vk::PhysicalDevice physicalDevice;
	vk::PhysicalDeviceFeatures physicalDeviceFeatures;

};