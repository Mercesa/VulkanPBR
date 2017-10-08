#pragma once

#include <cassert>

#include "RenderingIncludes.h"
#include "stb_image.h"
#include "Helper.h"
#include "easylogging++.h"
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
	VmaAllocator iAllocator, 
	VmaAllocation& oAllocation,
	VmaMemoryUsage iMemUsage,
	vk::Buffer& oBuffer, 
	vk::BufferUsageFlags iUsageFlags,
	size_t iSizeOfData)
{
	
	vk::BufferCreateInfo buf_info = vk::BufferCreateInfo()
	.setUsage(iUsageFlags)
	.setSize(iSizeOfData)
	.setQueueFamilyIndexCount(0)
	.setPQueueFamilyIndices(NULL)
	.setSharingMode(SharingMode::eExclusive)
	.setFlags(vk::BufferCreateFlagBits(0));
	
	
	VmaAllocationCreateInfo memReq = {};
	memReq.usage = iMemUsage;
	
	VkBuffer buffer = (VkBuffer)oBuffer;
	
	vmaCreateBuffer(iAllocator, &(VkBufferCreateInfo)buf_info, &memReq, &buffer, &oAllocation, nullptr);
	
	oBuffer = (vk::Buffer)buffer;

	return true;
}


inline bool CreateSimpleImage(
	VmaAllocator iAllocator,
	VmaAllocation& oAllocation,
	VmaMemoryUsage iMemUsage,
	vk::ImageUsageFlags iUsageFlags,
	vk::Format iFormat,
	vk::ImageLayout iLayout,
	vk::Image& oImage,
	uint32_t iWidth, uint32_t iHeight)
{

	vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
		.setImageType(ImageType::e2D)
		.setExtent(vk::Extent3D(iWidth, iHeight, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(iFormat)
		.setTiling(ImageTiling::eOptimal)
		.setInitialLayout(iLayout)
		.setUsage(iUsageFlags)
		.setSharingMode(SharingMode::eExclusive)
		.setSamples(vk::SampleCountFlagBits::e1);

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImage img = (VkImage)oImage;

	vmaCreateImage(iAllocator,
		&(VkImageCreateInfo)imageInfo,
		&alloc_info,
		&img,
		&oAllocation,
		nullptr);

	oImage = img;

	return true;
}


inline vk::ImageView CreateImageView(vk::Device aDevice, vk::Image aImage, vk::Format aFormat)
{
	vk::ImageViewCreateInfo	viewInfo = vk::ImageViewCreateInfo()
		.setImage(aImage)
		.setViewType(ImageViewType::e2D)
		.setFormat(aFormat)
		.setSubresourceRange(
			vk::ImageSubresourceRange(
				ImageAspectFlagBits::eColor,
				0, 1, 0, 1));

	vk::ImageView imgView;
	imgView = aDevice.createImageView(viewInfo, nullptr);

	return imgView;
}


inline bool CopyDataToBuffer(VkDevice aDevice, VmaAllocation aAllocation, void* aData, size_t aDataSize)
{
	uint8_t* pData;
	vkMapMemory(aDevice, aAllocation->GetMemory(), aAllocation->GetOffset(), aAllocation->GetSize(), 0, (void**)&pData);
	memcpy(pData, aData, aDataSize);
	vkUnmapMemory(aDevice, aAllocation->GetMemory());

	return true;
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
	aDevice.waitIdle();
}

ShaderVulkan CreateShader(
	const vk::Device& iDevice,
	const std::string& iFilePath, 
	const std::string& iEntryPoint,
	vk::ShaderStageFlagBits iShaderStage)
{
	auto code = readFile(iFilePath);

	// Ensure our code is not zero
	assert(code.size() > 0);

	ShaderVulkan tShader = ShaderVulkan();
	
	vk::ShaderModuleCreateInfo moduleInfo = vk::ShaderModuleCreateInfo()
		.setCodeSize(code.size())
		.setPCode(reinterpret_cast<const uint32_t*>(code.data()));

	tShader.shaderModule = iDevice.createShaderModule(moduleInfo);

	tShader.entryPointName = iEntryPoint;
	tShader.shaderFile = iFilePath;
	tShader.shaderStage = iShaderStage;

	return tShader;
}

inline void TransitionImageLayout(vk::CommandBuffer iBuffer, vk::Image aImage, vk::Format aFormat, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{

	vk::ImageMemoryBarrier barrier = ImageMemoryBarrier()
		.setOldLayout(oldLayout)
		.setNewLayout(newLayout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(aImage)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
		.setSrcAccessMask(vk::AccessFlagBits(0))
		.setDstAccessMask(vk::AccessFlagBits(0));

	vk::PipelineStageFlagBits sourceStage;
	vk::PipelineStageFlagBits destinationStage;


	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = PipelineStageFlagBits::eTopOfPipe;
		destinationStage = PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits(0);
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;

		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
	}


	else
	{
		LOG(ERROR) << "Unsupported layout transition";
	}

	iBuffer.pipelineBarrier(
		sourceStage,
		destinationStage,
		vk::DependencyFlagBits(0),
		0, nullptr,
		0, nullptr,
		1, &barrier);

}




//inline void LoadTextureSimple(const vk::Device iDevice, const VmaAllocator& iAllocator, std::string iFilePath, vk::Image& oImage, VmaAllocation& oAllocation)
//{
//	// Load texture with stbi
//	int texWidth, texHeight, texChannels;
//	stbi_uc* pixels = stbi_load(iFilePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//
//	vk::DeviceSize imageSize = texWidth * texHeight * 4;
//
//	if (!pixels)
//	{
//		LOG(ERROR) << "Load texture failed!";
//	}
//
//	// Create staging buffer for image
//	BufferVulkan stagingBuffer;
//
//	// create simple buffer
//	CreateSimpleBuffer(iAllocator,
//		stagingBuffer.allocation,
//		VMA_MEMORY_USAGE_CPU_ONLY,
//		stagingBuffer.buffer,
//		BufferUsageFlagBits::eTransferSrc,
//		imageSize);
//
//	// Copy image data to buffer
//	CopyDataToBuffer(VkDevice(device), stagingBuffer.allocation, pixels, imageSize);
//
//	// Free image
//	stbi_image_free(pixels);
//
//
//	CreateSimpleImage(iAllocator,
//		oAllocation,
//		VMA_MEMORY_USAGE_GPU_ONLY,
//		ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eSampled,
//		Format::eR8G8B8A8Unorm, ImageLayout::eUndefined,
//		oImage, texWidth, texHeight);
//
//	TransitionImageLayout(oImage,
//		vk::Format::eR8G8B8A8Unorm,
//		vk::ImageLayout::eUndefined,
//		vk::ImageLayout::eTransferDstOptimal);
//
//	CopyBufferToImage(stagingBuffer.buffer, oImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
//
//	TransitionImageLayout(oImage,
//		vk::Format::eR8G8B8A8Unorm,
//		vk::ImageLayout::eTransferDstOptimal,
//		vk::ImageLayout::eShaderReadOnlyOptimal);
//
//
//	vmaDestroyBuffer(iAllocator, stagingBuffer.buffer, stagingBuffer.allocation);
//}