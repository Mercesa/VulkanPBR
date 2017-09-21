#pragma once


#include "RenderingIncludes.h"
#include "VulkanDataObjects.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"

using namespace vk;


inline bool memory_type_from_properties(PhysicalDeviceMemoryProperties memProps, uint32_t typeBits, vk::MemoryPropertyFlags requirements_mask, uint32_t *typeIndex) {
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memProps.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

inline bool CreateSimpleBuffer(
	VmaAllocator aAllocator, 
	VmaAllocation& aAllocation,
	VmaMemoryUsage aMemUsage,
	vk::Buffer& aBuffer, 
	vk::BufferUsageFlags aUsageFlags,
	size_t aSizeofData)
{
	
	vk::BufferCreateInfo buf_info = vk::BufferCreateInfo()
	.setUsage(aUsageFlags)
	.setSize(aSizeofData)
	.setQueueFamilyIndexCount(0)
	.setPQueueFamilyIndices(NULL)
	.setSharingMode(SharingMode::eExclusive)
	.setFlags(vk::BufferCreateFlagBits(0));
	
	
	VmaAllocationCreateInfo memReq = {};
	memReq.usage = aMemUsage;
	
	VkBuffer buffer = (VkBuffer)aBuffer;
	
	vmaCreateBuffer(aAllocator, &(VkBufferCreateInfo)buf_info, &memReq, &buffer, &aAllocation, nullptr);
	
	aBuffer = (vk::Buffer)buffer;

	return true;
}

inline bool CopyDataToBuffer(VkDevice aDevice, VmaAllocation aAllocation, void* aData, size_t aDataSize)
{
	uint8_t* pData;
	vkMapMemory(aDevice, aAllocation->GetMemory(), aAllocation->GetOffset(), aAllocation->GetSize(), 0, (void**)&pData);
	memcpy(pData, aData, aDataSize);
	vkUnmapMemory(aDevice, aAllocation->GetMemory());

	return true;
}

// Get available extensions for WSI
inline std::vector<const char*> getAvailableWSIExtensions()
{
	std::vector<const char*> extensions;
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
	extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

	return extensions;
}

// Two functions for creating a command buffer for single time usage
// After usage the buffer is deleted
inline vk::CommandBuffer BeginSingleTimeCommands(vk::Device aDevice, vk::CommandPool aPoolToUse)
{
	vk::CommandBufferAllocateInfo alloc_info = vk::CommandBufferAllocateInfo()
		.setCommandPool(aPoolToUse)
		.setCommandBufferCount(1);

	vk::CommandBuffer commandBuffer = aDevice.allocateCommandBuffers(alloc_info)[0];

	vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

inline void EndSingleTimeCommands(vk::Device aDevice, vk::CommandBuffer aBuffer, vk::CommandPool aPool, vk::Queue aQueue)
{
	aBuffer.end();

	vk::SubmitInfo submitInfo = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(&aBuffer);

	aQueue.submit(submitInfo, vk::Fence(nullptr));
	aQueue.waitIdle();

	aDevice.freeCommandBuffers(aPool, aBuffer);
}
