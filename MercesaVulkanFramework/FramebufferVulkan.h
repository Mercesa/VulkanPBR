#pragma once

#include "RenderingIncludes.h"
#include "vk_mem_alloc.h"


// https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanFrameBuffer.hpp
// this class is heavily inspired (if not pretty much copied from) Sascha Willem' Vulkan framebuffer
// Though I will take credit for converting it into Vulkan C++ API and VMA :)
/**
* @brief Encapsulates a single frame buffer attachment
*/

struct FramebufferAttachmentVulkan
{
	vk::Image image;
	vk::ImageView view;
	vk::Format format;
	vk::ImageSubresourceRange subresourceRange;
	vk::AttachmentDescription description;
	VmaAllocation allocation;

	/**
	* @brief Returns true if the attachment has a depth component
	*/
	bool hasDepth()
	{
		std::vector<vk::Format> formats =
		{
			vk::Format::eD16Unorm,
			vk::Format::eD32Sfloat,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD32SfloatS8Uint
		};

		return std::find(formats.begin(), formats.end(), format) != std::end(formats);
	}

	/**
	* @brief Returns true if the attachment has a stencil component
	*/
	bool hasStencil()
	{
		std::vector<vk::Format> formats =
		{
			vk::Format::eS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD32SfloatS8Uint
		};
		return std::find(formats.begin(), formats.end(), format) != std::end(formats);
	}

	/**
	* @brief Returns true if the attachment is a depth and/or stencil attachment
	*/
	bool isDepthStencil()
	{
		return(hasDepth() || hasStencil());
	}
};

struct AttachmentCreateInfo
{
	uint32_t width, height;
	uint32_t layerCount;
	vk::Format format;
	vk::ImageUsageFlags usage;
};

class FramebufferVulkan
{
public:
	FramebufferVulkan(const uint32_t& iWidth, const uint32_t& iHeight);
	~FramebufferVulkan();
	
	// Width and height, the framebuffer itself and the renderpass
	uint32_t width, height;
	vk::Framebuffer framebuffer;
	vk::RenderPass renderpass;


	void AddAttachment(const vk::Device& iDevice, const AttachmentCreateInfo& iAttachmentCreateInfo, const VmaAllocator& iAllocator);
	void CreateRenderpass(const vk::Device& iDevice);
	void Destroy(const VmaAllocator& iAllocator, const vk::Device& iDevice);

	std::vector<FramebufferAttachmentVulkan> attachments;

private:
	
};

