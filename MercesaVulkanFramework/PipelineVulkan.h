#pragma once

#include <vector>
class ShaderDataVulkan;

class PipelineVulkan
{
public:
	PipelineVulkan();
	~PipelineVulkan();

	void Create(const std::vector<ShaderDataVulkan>& iShaders);
};

