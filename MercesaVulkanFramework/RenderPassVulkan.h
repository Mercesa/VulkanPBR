#pragma once

#include "RenderingIncludes.h"

using namespace vk;
class RenderPassVulkan
{
public:
	RenderPassVulkan() = default;
	~RenderPassVulkan();

	//void SetVertexAttributes(std::vector<)

private:
	vk::Framebuffer framebuffer;
	uint32_t screenWidth;
	uint32_t screenHeight;

	
	bool isDirty = false;
};

