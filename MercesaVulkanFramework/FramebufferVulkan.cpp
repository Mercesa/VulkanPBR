#include "FramebufferVulkan.h"

using namespace vk;

FramebufferVulkan::FramebufferVulkan(const uint32_t& iWidth, const uint32_t& iHeight) : width(iWidth), height(iHeight)
{
}


FramebufferVulkan::~FramebufferVulkan()
{
}

void FramebufferVulkan::AddAttachment(
	const vk::Device& iDevice,
	const AttachmentCreateInfo& iAttachmentCreateInfo,
	const VmaAllocator& iAllocator)
{
	FramebufferAttachmentVulkan attachment;
	attachment.format = iAttachmentCreateInfo.format;

	vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits(0);

	// If attachment is color, make it a color attachment
	// Otherwise if depth, make it depth attachment, if it contains a stencil. Make it a stencil attachment
	if (iAttachmentCreateInfo.usage & vk::ImageUsageFlagBits::eColorAttachment)
	{
		aspectMask = vk::ImageAspectFlagBits::eColor;
	}
	
	if (iAttachmentCreateInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
	{
		if (attachment.hasDepth())
		{
			aspectMask = vk::ImageAspectFlagBits::eDepth;
		}

		if (attachment.hasStencil())
		{
			aspectMask = aspectMask | vk::ImageAspectFlagBits::eStencil;
		}
	}

	assert(aspectMask != vk::ImageAspectFlagBits(0));


	vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(iAttachmentCreateInfo.format)
		.setExtent(vk::Extent3D(iAttachmentCreateInfo.width, iAttachmentCreateInfo.height, 1.0f))
		.setMipLevels(1)
		.setArrayLayers(iAttachmentCreateInfo.layerCount)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(iAttachmentCreateInfo.usage);

	VmaAllocationCreateInfo allocateInfo = {};
	allocateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	
	
	VkImage tImg;
	vmaCreateImage(iAllocator, &(VkImageCreateInfo)imageInfo,
		&allocateInfo, &tImg, &attachment.allocation, nullptr);
	
	attachment.image = tImg;


	attachment.subresourceRange = vk::ImageSubresourceRange()
		.setAspectMask(aspectMask)
		.setLevelCount(1)
		.setLayerCount(1);


	vk::ImageViewCreateInfo imageViewInfo = vk::ImageViewCreateInfo()
		.setViewType((iAttachmentCreateInfo.layerCount == 1) ? vk::ImageViewType::e2D : vk::ImageViewType::e2DArray)
		.setFormat(iAttachmentCreateInfo.format)
		.setSubresourceRange(attachment.subresourceRange)
		.setImage(attachment.image);

	imageViewInfo.subresourceRange.aspectMask = (attachment.hasDepth()) ? vk::ImageAspectFlagBits::eDepth : aspectMask;

	attachment.view = iDevice.createImageView(imageViewInfo);

	attachment.description = vk::AttachmentDescription()
		.setSamples(SampleCountFlagBits::e1)
		.setLoadOp(AttachmentLoadOp::eClear)
		.setStencilLoadOp(AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(AttachmentStoreOp::eDontCare)
		.setFormat(iAttachmentCreateInfo.format)
		.setInitialLayout(ImageLayout::eUndefined);

	attachment.description.storeOp = (iAttachmentCreateInfo.usage & vk::ImageUsageFlagBits::eSampled) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare;
	
	if (attachment.hasDepth() || attachment.hasStencil())
	{
		attachment.description.finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
	}
	else
	{
		attachment.description.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	}


	attachments.push_back(attachment);
}


void FramebufferVulkan::CreateRenderpass(const vk::Device& iDevice)
{
	std::vector<vk::AttachmentDescription> attachmentDescription;

	for (auto& attachment : attachments)
	{
		attachmentDescription.push_back(attachment.description);
	}

	std::vector<vk::AttachmentReference> colorReferences;
	vk::AttachmentReference depthReference = vk::AttachmentReference();

	bool hasDepth = false;
	bool hasColor = false;

	uint32_t attachmentIndex = 0;

	for (auto& attachment : attachments)
	{
		if (attachment.isDepthStencil())
		{
			assert(!hasDepth);
			depthReference.attachment = attachmentIndex;
			depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			hasDepth = true;
		}

		else
		{
			colorReferences.push_back({ attachmentIndex, vk::ImageLayout::eColorAttachmentOptimal });
			hasColor = true;
		}

		attachmentIndex++;
	}

	vk::SubpassDescription subpass = vk::SubpassDescription();
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

	if (hasColor)
	{
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	}

	if (hasDepth)
	{
		subpass.pDepthStencilAttachment = &depthReference;
	}

	std::array<vk::SubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
	dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
	dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
	dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	dependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
	dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;


	vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo()
		.setPAttachments(attachmentDescription.data())
		.setAttachmentCount(static_cast<uint32_t>(attachmentDescription.size()))
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(2)
		.setPDependencies(dependencies.data());

	renderpass = iDevice.createRenderPass(renderPassInfo);

	std::vector<vk::ImageView> attachmentviews;
	for (auto& attachment : attachments)
	{
		attachmentviews.push_back(attachment.view);
	}

	uint32_t maxLayers = 0;
	for (auto attachment : attachments)
	{
		if (attachment.subresourceRange.layerCount > maxLayers)
		{
			maxLayers = attachment.subresourceRange.layerCount;
		}
	}


	vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo()
		.setRenderPass(renderpass)
		.setPAttachments(attachmentviews.data())
		.setAttachmentCount(static_cast<uint32_t>(attachmentviews.size()))
		.setWidth(width)
		.setHeight(height)
		.setLayers(maxLayers);

	framebuffer = iDevice.createFramebuffer(framebufferInfo);
}

void FramebufferVulkan::Destroy(const VmaAllocator& iAllocator, const vk::Device& iDevice)
{
	for (auto& e : attachments)
	{
		vmaDestroyImage(iAllocator, e.image, e.allocation);
		iDevice.destroyImageView(e.view);
	}
	
	iDevice.destroyRenderPass(renderpass);
	iDevice.destroyFramebuffer(framebuffer);
}