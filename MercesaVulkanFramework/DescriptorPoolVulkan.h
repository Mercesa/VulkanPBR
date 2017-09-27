#pragma once

#include "RenderingIncludes.h"


struct DescriptorSizes
{
	int32_t combinedImgSamplerCount;
	int32_t samplerCount;
	int32_t uniformBufferCount;
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
		int32_t iMaxSets,
		int32_t iCombinedImgSamplerCount,
		int32_t iSamplerCount,
		int32_t iUniformBufferCount);

	bool Destroy(const vk::Device aDevice);

	bool ValidateResourcesAmounts(const DescriptorSizes& aResourceAmount);
	bool CalculateResourceCostOfDescriptorSet(
		const std::vector<vk::DescriptorSetLayoutBinding>& iLayoutBindings,
		DescriptorSizes& oCost);



	std::vector<vk::DescriptorSet> AllocateDescriptorSet(
		const vk::Device iDevice,
		const uint32_t iNumToAllocate,
		const std::vector<vk::DescriptorSetLayout>& iDescriptorLayouts);

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
		const DescriptorSizes& iCost);

	bool hasInitialized;
	vk::DescriptorPool pool;
	DescriptorSizes currentResources;
};

