#pragma once


#include "RenderingIncludes.h"
#include "VulkanDataObjects.h"


// NOTE
// All functions will return a boolean, to signify if function succeeded or not
// All functions will ask either for a reference or pointer

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

inline bool OptimalFamilyQuery()
{

}

inline bool SetupBasicVertexBuffer(vk::Device aDevice,
	VertexBufferVulkan* aVertexBuffer,
	PhysicalDeviceMemoryProperties aPhysDevMemProps,
	void* aData,
	size_t aSizeOfData)
{
	vk::BufferCreateInfo buf_info = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
		.setSize(sizeof(aData))
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(NULL)
		.setSharingMode(SharingMode::eExclusive)
		.setFlags(vk::BufferCreateFlagBits(0));

	aVertexBuffer->buffer = aDevice.createBuffer(buf_info);

	vk::MemoryRequirements memReqs = aDevice.getBufferMemoryRequirements(aVertexBuffer->buffer);

	vk::MemoryAllocateInfo alloc_info = vk::MemoryAllocateInfo()
		.setMemoryTypeIndex(0)
		.setAllocationSize(memReqs.size);

	memory_type_from_properties(aPhysDevMemProps, memReqs.memoryTypeBits, MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &alloc_info.memoryTypeIndex);

	vk::Result res;

	res = aDevice.allocateMemory(&alloc_info, nullptr, &aVertexBuffer->memory);
	assert(res == vk::Result::eSuccess);

	uint8_t *pData;

	res = aDevice.mapMemory(aVertexBuffer->memory, 0, vk::DeviceSize(memReqs.size), vk::MemoryMapFlagBits(0), (void**)&pData);
	assert(res == vk::Result::eSuccess);

	memcpy(pData, aData, sizeof(aData));

	aDevice.unmapMemory(aVertexBuffer->memory);

	aDevice.bindBufferMemory(aVertexBuffer->buffer, aVertexBuffer->memory, 0);

	return true;
}