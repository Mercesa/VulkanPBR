#pragma once

#include "RenderingIncludes.h"
#include <cinttypes>

class CommandpoolVulkan
{
public:

	// Perhaps a set family which resets the whole command pool
	void Create(const vk::Device& iDevice, 
		const uint32_t& iFamily,
		vk::CommandPoolCreateFlagBits iFlags);

	void Destroy(const vk::Device& iDevice);
	std::vector<vk::CommandBuffer> AllocateBuffer(
		const vk::Device& iDevice,
		vk::CommandBufferLevel iLevel,
		const uint32_t& iCount);
	
	uint32_t GetFamily() { return currentFamily; };
	vk::CommandPool GetPool() { return pool; };

	CommandpoolVulkan() = default;
	~CommandpoolVulkan();

private:
	vk::CommandPool pool;
	std::vector<vk::CommandBuffer> commandBuffers;
	uint32_t currentFamily = -1;
};

