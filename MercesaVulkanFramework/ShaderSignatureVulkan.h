#pragma once

#include "RenderingIncludes.h"

class ShaderSignatureVulkan
{
public:
	ShaderSignatureVulkan() = default;
	~ShaderSignatureVulkan() = default;

	void Setup(std::vector<vk::DescriptorSetLayout> iLayouts);

	std::vector<vk::DescriptorSet> shaderDescriptorResources;
	std::vector<vk::DescriptorSetLayout> shaderDescriptorLayouts;
};

