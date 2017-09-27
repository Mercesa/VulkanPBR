#include "DescriptorPoolVulkan.h"

#include "easylogging++.h"

DescriptorPoolVulkan::DescriptorPoolVulkan() : hasInitialized(false)
{
}


//DescriptorPoolVulkan::~DescriptorPoolVulkan()
//{
//}

bool DescriptorPoolVulkan::Create(const vk::Device aDevice, 
	const int32_t iMaxSets,
	const int32_t iCombinedImgSamplerCount,
	const int32_t iSamplerCount,
	const int32_t iUniformBufferCount)
{
	if (hasInitialized)
	{
		LOG(ERROR) << "DescriptorPoolVulkan::Create User attempted to create pool, pool already has been created";
		return false;
	}

	std::array<vk::DescriptorPoolSize, 3> type_count;

	// Initialize our pool with these values
	type_count[0].type = vk::DescriptorType::eCombinedImageSampler;
	type_count[0].descriptorCount = iCombinedImgSamplerCount;

	type_count[1].type = vk::DescriptorType::eSampler;
	type_count[1].descriptorCount = iSamplerCount;

	type_count[2].type = vk::DescriptorType::eUniformBuffer;
	type_count[2].descriptorCount = iUniformBufferCount;

	currentResources.combinedImgSamplerCount = iCombinedImgSamplerCount;
	currentResources.samplerCount = iSamplerCount;
	currentResources.uniformBufferCount = iUniformBufferCount;


	vk::DescriptorPoolCreateInfo createInfo = vk::DescriptorPoolCreateInfo()
		.setPNext(nullptr)
		.setMaxSets(iMaxSets)
		.setPoolSizeCount(static_cast<uint32_t>(type_count.size()))
		.setPPoolSizes(type_count.data());

	// Create the descriptor pool
	pool = aDevice.createDescriptorPool(createInfo);	
		
	hasInitialized = true;
	return true;
}

bool DescriptorPoolVulkan::Destroy(const vk::Device aDevice)
{
	// Make sure pool exists
	//assert(pool != VK_NULL_HANDLE);
	assert(hasInitialized);

	aDevice.destroyDescriptorPool(pool);
	return true;
}

bool DescriptorPoolVulkan::ValidateResourcesAmounts(const DescriptorSizes& iResourceAmount)
{
	// Validate if we have enough resources 
	if (this->currentResources.combinedImgSamplerCount - iResourceAmount.combinedImgSamplerCount < 0)
	{
		return false;
	}

	if (this->currentResources.samplerCount - iResourceAmount.samplerCount < 0)
	{
		return false;
	}

	if (this->currentResources.samplerCount - iResourceAmount.uniformBufferCount < 0)
	{
		return false;
	}

	return true;
}

bool DescriptorPoolVulkan::UpdateResourceAmounts(const DescriptorSizes& iResourceAmounts)
{
	currentResources.combinedImgSamplerCount - iResourceAmounts.combinedImgSamplerCount;
	currentResources.samplerCount - iResourceAmounts.samplerCount;
	currentResources.uniformBufferCount - iResourceAmounts.uniformBufferCount;

	return true;
}

bool DescriptorPoolVulkan::CalculateResourceCostOfDescriptorSet(
	const std::vector<vk::DescriptorSetLayoutBinding>& iLayoutBindings,
	DescriptorSizes& oCost)
{
	DescriptorSizes tCost;

	// Calculate the resources for all the bindings
	for (auto& e : iLayoutBindings)
	{
		switch (e.descriptorType)
		{
		case vk::DescriptorType::eCombinedImageSampler:
			tCost.combinedImgSamplerCount += e.descriptorCount;
			break;

		case vk::DescriptorType::eSampler:
			tCost.samplerCount += e.descriptorCount;
			break;

		case vk::DescriptorType::eUniformBuffer:
			tCost.uniformBufferCount += e.descriptorCount;
			break;

		default:
			LOG(WARNING) << "DescriptorPoolVulkan::CalculateResourceCostOfDescriptorSet Unknown descriptor type detected, consider all calculations invalid";
			return false;
			break;
		}
	}

	return true;
}

std::vector<vk::DescriptorSet> DescriptorPoolVulkan::AllocateDescriptorSet(
	const vk::Device iDevice,
	const uint32_t iNumToAllocate,
	const std::vector<vk::DescriptorSetLayout>& iDescriptorLayouts)
{
	assert(hasInitialized);

	// If the user inputs 0 descriptors, throw a warning
	if (iNumToAllocate == 0)
	{
		LOG(WARNING) << "DescriptorPoolVulkan::AllocateDescriptorSet iNumToAllocate needs to be more than zero";
		return std::vector<vk::DescriptorSet>();
	}

	vk::DescriptorSetAllocateInfo alloc_info[1] = {};
	alloc_info[0].pNext = NULL;
	alloc_info[0].setDescriptorPool(pool);
	alloc_info[0].setDescriptorSetCount(iNumToAllocate);
	alloc_info[0].setPSetLayouts(&iDescriptorLayouts[0]);

	// Prepare a vector to fill with descriptors
	std::vector<vk::DescriptorSet> tDescriptors;
	tDescriptors.resize(iNumToAllocate);

	iDevice.allocateDescriptorSets(alloc_info, tDescriptors.data());

	return tDescriptors;
}

std::vector<vk::DescriptorSet> DescriptorPoolVulkan::AllocateDescriptorSet(
	const vk::Device iDevice,
	const uint32_t iNumToAllocate,
	const vk::DescriptorSetLayout& iDescriptorLayouts,
	const std::vector<vk::DescriptorSetLayoutBinding>& iDescriptorLayoutBindings)
{
	assert(hasInitialized);

	// If the user inputs 0 descriptors, throw a warning
	if (iNumToAllocate == 0)
	{
		LOG(WARNING) << "DescriptorPoolVulkan::AllocateDescriptorSet iNumToAllocate needs to be more than zero";
		return std::vector<vk::DescriptorSet>();
	}

	// Calculate the cost of our single descriptor layout

	DescriptorSizes tResourceCost; 
	if (!CalculateResourceCostOfDescriptorSet(iDescriptorLayoutBindings, tResourceCost))
	{
		LOG(WARNING) << "DescriptorPoolVulkan::AllocateDescriptorSet Resource cost query failed, consider allocation invalid";
	}

	if (!ValidateResourcesAmounts(tResourceCost))
	{
		LOG(WARNING) << "DescriptorPoolVulkan::AlocateDescriptorSet Insufficient resources to allocate descriptor set";
	}

	vk::DescriptorSetAllocateInfo alloc_info[1] = {};
	alloc_info[0].pNext = NULL;
	alloc_info[0].setDescriptorPool(pool);
	alloc_info[0].setDescriptorSetCount(iNumToAllocate);
	alloc_info[0].setPSetLayouts(&iDescriptorLayouts);

	std::vector<vk::DescriptorSet> tDescriptors;
	tDescriptors.resize(iNumToAllocate);

	iDevice.allocateDescriptorSets(alloc_info, tDescriptors.data());

	// Update our resources
	UpdateResourceAmounts(tResourceCost);

	return tDescriptors;

}

const vk::DescriptorPool DescriptorPoolVulkan::GetDescriptorPool()
{
	return pool;
}