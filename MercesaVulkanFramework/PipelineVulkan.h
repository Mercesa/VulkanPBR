#pragma once

#include <vector>
class ShaderVulkan;

class PipelineVulkan
{
public:
	PipelineVulkan();
	~PipelineVulkan();

	void Create(const std::vector<ShaderVulkan>& iShaders);
};

