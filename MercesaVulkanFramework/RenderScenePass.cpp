#include "RenderScenePass.h"

#include <vector>

using namespace vk;

RenderScenePass::RenderScenePass()
{
}


RenderScenePass::~RenderScenePass()
{
}


void RenderScenePass::CreateRenderpass(const vk::Device& iDevice, vk::Format iSwapchainFormat, vk::SampleCountFlagBits iSamples)
{
	vk::AttachmentDescription attachments[2] = {};
	attachments[0].format = iSwapchainFormat;
	attachments[0].samples = iSamples;
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	attachments[0].flags = AttachmentDescriptionFlagBits(0);

	attachments[1].format = vk::Format::eD24UnormS8Uint;
	attachments[1].samples = iSamples;
	attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].initialLayout = vk::ImageLayout::eUndefined;
	attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	attachments[1].flags = AttachmentDescriptionFlagBits(0);



	std::vector<AttachmentReference> references;

	vk::AttachmentReference color_reference = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);


	references.push_back(color_reference);


	vk::AttachmentReference depth_reference = vk::AttachmentReference()
		.setAttachment(1)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass = vk::SubpassDescription()
		.setPipelineBindPoint(PipelineBindPoint::eGraphics)
		.setFlags(vk::SubpassDescriptionFlagBits(0))
		.setInputAttachmentCount(0)
		.setPInputAttachments(0)
		.setColorAttachmentCount(1)
		.setPColorAttachments(references.data())
		.setPResolveAttachments(NULL)
		.setPDepthStencilAttachment(&depth_reference)
		.setPreserveAttachmentCount(0)
		.setPResolveAttachments(NULL);

	vk::SubpassDependency dependency = vk::SubpassDependency()
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits(0))
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);


	vk::RenderPassCreateInfo rp_info = RenderPassCreateInfo()
		.setPNext(NULL)
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);


	renderpass = iDevice.createRenderPass(rp_info);
}


void RenderScenePass::Destroy(const vk::Device& iDevice)
{
	iDevice.destroyRenderPass(renderpass);
}