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

// Updates designated UniformBufferVulkan object
inline bool UpdateUniformBuffer(vk::Device aDevice, UniformBufferVulkan* const aUniformBuffer, void* aData, size_t aSizeOfData)
{
	uint8_t* pData;
	vk::Result result;
	result = aDevice.mapMemory(vk::DeviceMemory(aUniformBuffer->memory), vk::DeviceSize(0), vk::DeviceSize(aUniformBuffer->memReqs.size), vk::MemoryMapFlagBits(0), (void**)&pData);

	assert(result == Result::eSuccess);

	memcpy(pData, aData, aSizeOfData);

	 aDevice.unmapMemory(aUniformBuffer->memory);
	
	 return true;
}

inline bool CreateBuffer()
	//vk::Device aDevice, 
	//vk::DeviceSize aSize, 
	//vk::BufferUsageFlags aUsage, 
	//vk::MemoryPropertyFlagBits aProperties, 
	//VertexBufferVulkan* const aVertexBuffer)
{	//
	//vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo()
	//	.setSize(aSize)
	//	.setUsage(aUsage)
	//	.setSharingMode(vk::SharingMode::eExclusive);
	//
	//aVertexBuffer->buffer = aDevice.createBuffer(bufferInfo, nullptr);
	//
	//aVertexBuffer->memReqs = aDevice.getBufferMemoryRequirements(aVertexBuffer->buffer);
	//
	//
	//
	//vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
	//	.setMemoryTypeIndex(0)
	//	.setAllocationSize(aVertexBuffer->memReqs.size);
	//
	//memory_type_from_properties(memoryProperties, aVertexBuffer->memReqs.memoryTypeBits, aProperties, &allocInfo.memoryTypeIndex);
	//
	//aDevice.allocateMemory(&allocInfo, nullptr, &aVertexBuffer->memory);
	//
	//aDevice.bindBufferMemory(aVertexBuffer->buffer, aVertexBuffer->memory, 0);
	return true;
}