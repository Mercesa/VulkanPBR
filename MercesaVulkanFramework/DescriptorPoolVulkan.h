#pragma once

#include "RenderingIncludes.h"


struct PoolData
{
	PoolData(
		int32_t iCombinedImgSamplerCount,
		int32_t iSamplerCount,
		int32_t iUniformBuffercount,
		int32_t iSampledImgCount,
		int32_t iUniformDynamicCount,
		int32_t iSetCount,
		int32_t iStorageImageCount) : 	combinedImgSamplerCount(iCombinedImgSamplerCount), 
		samplerCount(iSamplerCount), 
		uniformBufferCount(iUniformBuffercount), 
		sampledImgCount(iSampledImgCount), 
		uniformDynamicCount(iUniformDynamicCount),
		setCount(iSetCount),
		storageImageCount(iStorageImageCount){}

	PoolData() : combinedImgSamplerCount(0), samplerCount(0), uniformBufferCount(0), sampledImgCount(0), uniformDynamicCount(0), setCount(0) {}

	int32_t combinedImgSamplerCount = 0;
	int32_t samplerCount = 0;
	int32_t uniformBufferCount = 0;
	int32_t sampledImgCount = 0;
	int32_t uniformDynamicCount = 0;
	int32_t storageImageCount = 0;
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
		const PoolData& iPoolData );

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

