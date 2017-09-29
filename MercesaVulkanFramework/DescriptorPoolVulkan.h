#pragma once

#include "RenderingIncludes.h"


struct PoolData
{
	int32_t combinedImgSamplerCount = 0;
	int32_t samplerCount = 0;
	int32_t uniformBufferCount = 0;
	int32_t sampledImgCount = 0;
	int32_t setCount = 0;
};

// A descriptor is a set and a layout
struct Descriptor
{
	vk::DescriptorSet set;
	vk::DescriptorSetLayout layout;
};

class DescriptorPoolVulkan
{
public:
	DescriptorPoolVulkan();

	bool Create(const vk::Device aDevice,
		uint32_t iMaxSets,
		uint32_t iCombinedImgSamplerCount,
		uint32_t iSamplerCount,
		uint32_t iUniformBufferCount,
		uint32_t iSampledImgCount);

	bool Destroy(const vk::Device aDevice);


	inline bool ValidateResourcesAmounts(const PoolData& aResourceAmount);
	bool CalculateResourceCostOfDescriptorSet(
		const std::vector<vk::DescriptorSetLayoutBinding>& iLayoutBindings,
		const uint32_t& iAmount,
		PoolData& oCost);


	//std::vector<vk::DescriptorSet> AllocateDescriptorSet(
	//	const vk::Device iDevice,
	//	const uint32_t iNumToAllocate,
	//	const std::vector<vk::DescriptorSetLayout>& iDescriptorLayouts);


	std::vector<vk::DescriptorSet> AllocateDescriptorSet(
		const vk::Device iDevice,
		const uint32_t iNumToAllocate,
		const vk::DescriptorSetLayout& iDescriptorLayouts,
		const std::vector<vk::DescriptorSetLayoutBinding>& iDescriptorLayoutBindings);

	const vk::DescriptorPool GetDescriptorPool();
	//~DescriptorPoolVulkan() = delete;
	//DescriptorPoolVulkan(const DescriptorPoolVulkan& rh) = delete;
	
private:
	// Our resource pool is a self managing entity, updating the resources is an internal operation
	bool UpdateResourceAmounts(
		const PoolData& iCost);

	bool hasInitialized;
	vk::DescriptorPool pool;
	PoolData currentResources;

	std::vector<vk::DescriptorSet> allocatedSets;
};

