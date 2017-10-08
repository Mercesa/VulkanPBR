#pragma once
#include "PhysicalResourceManager.h"
#include "VulkanDataObjects.h"

class VulkanResourceManager :
	public PhysicalResourceManager
{
public:
	VulkanResourceManager();
	virtual ~VulkanResourceManager();


	//std::map<uint64_t, ModelVulkan> modelDatabase;
	//std::map<uint64_t, > modelDatabase;

};

