#pragma once

#include "iObjectRenderingData.h"

#include "RenderingIncludes.h"
#include "VulkanDataObjects.h"

class ObjectRenderingDataVulkan : public iObjectRenderingData
{
public:
	ObjectRenderingDataVulkan();
	virtual ~ObjectRenderingDataVulkan();


	vk::DescriptorSet positionBufferSet;
	UniformBufferVulkan positionUniformBuffer;
};

