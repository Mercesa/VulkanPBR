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
	VertexBufferVulkan vertexBuffer;
	MaterialVulkan material;
	vk::DescriptorSet textureSet;
	vk::DescriptorSet positionBufferSet;
	UniformBufferVulkan positionUniformBuffer;
};

