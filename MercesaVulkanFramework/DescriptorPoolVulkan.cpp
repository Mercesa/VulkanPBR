#include "DescriptorPoolVulkan.h"

#include "easylogging++.h"

DescriptorPoolVulkan::DescriptorPoolVulkan() : hasInitialized(false)
{
}

bool DescriptorPoolVulkan::Create(const vk::Device aDevice, 
	uint32_t iMaxSets,
	uint32_t iCombinedImgSamplerCount,
	uint32_t iSamplerCount,
	uint32_t iUniformBufferCount,
	uint32_t iSampledImgCount)
{
	if (hasInitialized)
	{
		LOG(ERROR) << "DescriptorPoolVulkan::Create User attempted to create pool, pool already has been created";
		return false;
	}

	std::array<vk::DescriptorPoolSize, 4> type_count;
	

	// Initialize our pool with these values
	type_count[0].type = vk::DescriptorType::eCombinedImageSampler;
	type_count[0].descriptorCount = iCombinedImgSamplerCount;

	type_count[1].type = vk::DescriptorType::eSampler;
	type_count[1].descriptorCount = iSamplerCount;
	
	type_count[2].type = vk::DescriptorType::eUniformBuffer;
	type_count[2].descriptorCount = iUniformBufferCount;

	type_count[3].type = vk::DescriptorType::eSampledImage;
	type_count[3].descriptorCount = iSampledImgCount;

	vk::DescriptorPoolCreateInfo createInfo = vk::DescriptorPoolCreateInfo()
		.setPNext(nullptr)
		.setMaxSets(iMaxSets)
		.setPoolSizeCount(1)
		.setPPoolSizes(type_count.data());
	pool = aDevice.createDescriptorPool(createInfo);

	currentResources.combinedImgSamplerCount = iCombinedImgSamplerCount;
	currentResources.samplerCount = iSamplerCount;
	currentResources.uniformBufferCount = iUniformBufferCount;
	currentResources.setCount = iMaxSets;




	// Create the descriptor pool
	hasInitialized = true;
	return true;
}

bool DescriptorPoolVulkan::Destroy(const vk::Device aDevice)
{
	// Make sure pool exists
	//assert(pool != VK_NULL_HANDLE);
	assert(hasInitialized);


	aDevice.freeDescriptorSets(pool, allocatedSets);

	aDevice.destroyDescriptorPool(pool);
	return true;
}

bool DescriptorPoolVulkan::ValidateResourcesAmounts(const PoolData& iResourceAmount)
{
	// Validate if we have enough resources 
	if (this->currentResources.combinedImgSamplerCount - iResourceAmount.combinedImgSamplerCount < 0)
	{
		return false;
	}

	if ((this->currentResources.samplerCount - iResourceAmount.samplerCount) < 0)
	{
		return false;
	}

	if (this->currentResources.uniformBufferCount- iResourceAmount.uniformBufferCount < 0)
	{
		return false;
	}

	if (this->currentResources.setCount - iResourceAmount.setCount < 0)
	{
		return false;
	}

	return true;
}

bool DescriptorPoolVulkan::UpdateResourceAmounts(const PoolData& iResourceAmounts)
{
	currentResources.combinedImgSamplerCount -= iResourceAmounts.combinedImgSamplerCount;
	currentResources.samplerCount -= iResourceAmounts.samplerCount;
	currentResources.uniformBufferCount -= iResourceAmounts.uniformBufferCount;
	currentResources.sampledImgCount -= iResourceAmounts.sampledImgCount;

	currentResources.setCount -= iResourceAmounts.setCount;

	return true;
}

bool DescriptorPoolVulkan::CalculateResourceCostOfDescriptorSet(
	const std::vector<vk::DescriptorSetLayoutBinding>& iLayoutBindings,
	const uint32_t& iSetAmount,
	PoolData& oCost)
{
	if (iSetAmount == 0)
	{
		return false;
	}

	// Calculate the resources for all the bindings
	for (auto& e : iLayoutBindings)
	{
		switch (e.descriptorType)
		{
		case vk::DescriptorType::eCombinedImageSampler:
			oCost.combinedImgSamplerCount += e.descriptorCount * iSetAmount;
			break;

		case vk::DescriptorType::eSampler:
			oCost.samplerCount += e.descriptorCount * iSetAmount;
			break;

		case vk::DescriptorType::eUniformBuffer:
			oCost.uniformBufferCount += e.descriptorCount * iSetAmount;
			break;

		case vk::DescriptorType::eSampledImage:
			oCost.sampledImgCount += e.descriptorCount * iSetAmount;
			break;

		default:
			LOG(WARNING) << "DescriptorPoolVulkan::CalculateResourceCostOfDescriptorSet Unknown descriptor type detected, consider all calculations invalid";
			return false;
			break;
		}
	}

	oCost.setCount += iSetAmount;

	return true;
}

//std::vector<vk::DescriptorSet> DescriptorPoolVulkan::AllocateDescriptorSet(
//	const vk::Device iDevice,
//	const uint32_t iNumToAllocate,
//	const std::vector<vk::DescriptorSetLayout>& iDescriptorLayouts)
//{
//	assert(hasInitialized);
//
//	// If the user inputs 0 descriptors, throw a warning
//	if (iNumToAllocate == 0)
//	{
//		LOG(WARNING) << "DescriptorPoolVulkan::AllocateDescriptorSet iNumToAllocate needs to be more than zero";
//		return std::vector<vk::DescriptorSet>();
//	}
//
//	vk::DescriptorSetAllocateInfo alloc_info[1] = {};
//	alloc_info[0].pNext = NULL;
//	alloc_info[0].setDescriptorPool(pool);
//	alloc_info[0].setDescriptorSetCount(iNumToAllocate);
//	alloc_info[0].setPSetLayouts(&iDescriptorLayouts[0]);
//
//	// Prepare a vector to fill with descriptors
//	std::vector<vk::DescriptorSet> tDescriptors;
//	tDescriptors.resize(iNumToAllocate);
//
//	if (iDevice.allocateDescriptorSets(alloc_info, tDescriptors.data()) == vk::Result::eSuccess)
//	{
//		LOG(INFO) << "AllocateDescriptorSet Succesfully allocated descriptor";
//	}
//
//	return tDescriptors;
//}

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

	PoolData tResourceCost{};

	// Calculate the cost of our single descriptor layout
	if (!CalculateResourceCostOfDescriptorSet(iDescriptorLayoutBindings, iNumToAllocate, tResourceCost))
	{
		LOG(WARNING) << "DescriptorPoolVulkan::AllocateDescriptorSet Resource cost query failed, consider allocation invalid";
		return std::vector<vk::DescriptorSet>() = { vk::DescriptorSet() };
	}

	// Validate if we have enough resources left in our pool
	if (!ValidateResourcesAmounts(tResourceCost))
	{
		LOG(WARNING) << "DescriptorPoolVulkan::AlocateDescriptorSet Insufficient resources to allocate descriptor set, consider allocation invalid";
		return std::vector<vk::DescriptorSet>() = { vk::DescriptorSet() };
	}

	vk::DescriptorSetAllocateInfo alloc_info = {};
	alloc_info.pNext = NULL;
	alloc_info.setDescriptorPool(pool);
	alloc_info.setDescriptorSetCount(iNumToAllocate);
	alloc_info.setPSetLayouts(&iDescriptorLayouts);

	std::vector<vk::DescriptorSet> tDescriptors;
	tDescriptors.resize(iNumToAllocate);

	// Allocate descriptors if all is well
	if (iDevice.allocateDescriptorSets(&alloc_info, tDescriptors.data()) == vk::Result::eSuccess)
	{
		LOG(INFO) << "DescriptorPoolVulkan::AllocateDescriptorSet Allocated set!";
	}

	else
	{
		LOG(WARNING) << " DescriptorPoolVulkan::AllocatDescriptorSet FAILED TO ALLOCATE";
	}

	// Update our resources
	UpdateResourceAmounts(tResourceCost);

	return tDescriptors;
}

const vk::DescriptorPool DescriptorPoolVulkan::GetDescriptorPool()
{
	return pool;
}