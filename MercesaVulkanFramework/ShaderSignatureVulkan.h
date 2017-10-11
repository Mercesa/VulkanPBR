#pragma once

#include "RenderingIncludes.h"

class ShaderSignatureVulkan
{
public:
	ShaderSignatureVulkan();
	~ShaderSignatureVulkan();

	void Setup();

	std::vector<vk::DescriptorSet> shaderDescriptorResources;
	std::vector<vk::DescriptorSetLayout> shaderDescriptorLayouts;
};

