#pragma once

#include "RenderingIncludes.h"

class RenderScenePass
{
public:
	RenderScenePass();
	~RenderScenePass();

	
	void CreateRenderpass(const vk::Device& iDevice, vk::Format iSwapchainFormat, vk::SampleCountFlagBits iSamples);
	void Destroy(const vk::Device& iDevice);
	
	vk::RenderPass renderpass;
};

