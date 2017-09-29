#include "CommandpoolVulkan.h"



void CommandpoolVulkan::Create(const vk::Device& iDevice, 
	const uint32_t & iFamily,
	vk::CommandPoolCreateFlagBits)
{
	vk::CommandPoolCreateInfo commandPoolInfo = vk::CommandPoolCreateInfo()
		.setPNext(nullptr)
		.setQueueFamilyIndex(iFamily)
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	pool = iDevice.createCommandPool(commandPoolInfo);
}

void CommandpoolVulkan::Destroy(const vk::Device& iDevice)
{
	iDevice.freeCommandBuffers(pool, commandBuffers);
	iDevice.destroyCommandPool(pool);
}

std::vector<vk::CommandBuffer> CommandpoolVulkan::AllocateBuffer(
	const vk::Device& iDevice,
	vk::CommandBufferLevel iLevel,
	const uint32_t& iCount)
{
	vk::CommandBufferAllocateInfo commandBuffAllInfo = vk::CommandBufferAllocateInfo()
		.setPNext(NULL)
		.setLevel(iLevel)
		.setCommandBufferCount(iCount)
		.setCommandPool(pool);


	std::vector<vk::CommandBuffer> tCmdbuffers = iDevice.allocateCommandBuffers(commandBuffAllInfo);

	this->commandBuffers.insert(commandBuffers.end(), tCmdbuffers.begin(), tCmdbuffers.end());

	return tCmdbuffers;
}


CommandpoolVulkan::~CommandpoolVulkan()
{
}
