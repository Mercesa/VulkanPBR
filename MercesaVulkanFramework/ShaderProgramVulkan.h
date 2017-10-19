#pragma once

#include "RenderingIncludes.h"

struct ShaderDataVulkan;

class ShaderProgramVulkan
{
public:
	ShaderProgramVulkan();
	~ShaderProgramVulkan();

	bool LoadShaders(const vk::Device& iDevice, std::vector<ShaderDataVulkan> iShadersToLoad);
	void Destroy(const vk::Device& iDevice);

	std::vector<vk::PipelineShaderStageCreateInfo> GetPipelineShaderInfo();
	std::vector<vk::DescriptorSetLayout> GetShaderProgramLayout();

private:
	std::vector<ShaderDataVulkan> shadersData;
	std::vector<vk::DescriptorSetLayout> shaderLayout;
};

