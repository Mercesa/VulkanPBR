#pragma once
#include "iModel.h"

#include "VulkanDataObjects.h"

class ModelVulkan :
	public iModel
{
public:
	ModelVulkan();
	virtual ~ModelVulkan();


	BufferVulkan indexBuffer;
	BufferVulkan vertexBuffer;
	vk::DescriptorSet textureSet;
};

