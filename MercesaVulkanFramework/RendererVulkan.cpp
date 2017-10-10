#include "RendererVulkan.h"



// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define NOMINMAX
//#define WIN32_MEAN_AND_LEAN

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "RenderingIncludes.h"

#include <iostream>
#include <math.h>
#include <algorithm>
#include <utility>
#include <future>
#include <map>

#include "VulkanDataObjects.h"
#include "GraphicsStructures.h"
#include "DescriptorPoolVulkan.h"

#include "VulkanHelper.h"
#include "Helper.h"
#include "ModelLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Camera.h"

#include <ostream>
#include <sstream>
#include <string.h>
#include <iterator>
#include "ConstantBuffers.h"

#include "DescriptorPoolVulkan.h"
#include "CommandpoolVulkan.h"
#include "DeviceVulkan.h"

#include "easylogging++.h"
#include "SDLLowLevelWindow.h"
#include "libs/Spir-v cross/spirv_glsl.hpp"
#include "DescriptorLayoutHelper.h"
#include "NewCamera.h"
#include "Helper.h"
#include "Game.h"
#include "FramebufferVulkan.h"

CBMatrix matrixConstantBufferData;
CBLights lightConstantBufferData;



#define NUM_SAMPLES vk::SampleCountFlagBits::e1
#define NUM_DESCRIPTOR_SETS 3
#define FENCE_TIMEOUT 100000000
#define NUM_FRAMES 2

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, iLowLevelWindow* window);

static const int screenWidth = 1280;
static const int screenHeight = 720;

using namespace vk;


uint32_t currentBuffer = 0;

std::vector<vk::Framebuffer> framebuffers;
std::vector<vk::Framebuffer> framebufferScene;

std::unique_ptr<FramebufferVulkan> fbVulkan;

// Descriptor set layout
std::vector<vk::DescriptorSetLayout> desc_layout;
std::vector<std::vector<vk::DescriptorSet>> descriptor_set;

vk::PipelineLayout pipelineLayout;

TextureVulkan depthBuffer;
TextureVulkan postProcBuffer;

std::vector<UniformBufferVulkan> uniformBufferMVP;
std::vector<UniformBufferVulkan> uniformBufferLights;

vk::RenderPass renderPassPostProc;

// Device and instance
vk::Instance instance;

std::unique_ptr<CommandpoolVulkan> cmdPool;
std::vector<vk::CommandBuffer> commandBuffers;

std::shared_ptr<iLowLevelWindow> window;
// Information about our device its memory and family properties

std::unique_ptr<DescriptorPoolVulkan> descriptorPool;

std::vector<std::unique_ptr<ModelVulkan>> models;

VmaAllocator allocator;

std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

vk::Pipeline pipeline;

vk::Pipeline pipelineRenderScene;

vk::Viewport viewPort;
vk::Rect2D scissor;


vk::Sampler testImageSampler;
vk::Sampler testSampler;

vk::Semaphore imageAcquiredSemaphore;
vk::Semaphore rendererFinishedSemaphore;

std::vector<vk::DescriptorSetLayoutBinding> bindings;
std::vector<vk::DescriptorSetLayoutBinding> uniformBinding;
std::vector<vk::DescriptorSetLayoutBinding> textureBinding;

std::vector<ShaderVulkan> shaders;

std::unique_ptr<DeviceVulkan> deviceVulkan;

RendererVulkan::RendererVulkan()
{
}


RendererVulkan::~RendererVulkan()
{
}


vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, iLowLevelWindow* const window)
{
	vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
		.setHinstance(GetModuleHandle(NULL))
		.setHwnd(window->GetWindowHandle());
	return instance.createWin32SurfaceKHR(surfaceInfo);
}


void SetupApplication()
{
	// vk::ApplicationInfo allows the programmer to specifiy some basic information about the
	// program, which can be useful for layers and tools to provide more debug information.
	deviceVulkan = std::make_unique<DeviceVulkan>();
	deviceVulkan->CreateInstance("PBR Vulkan", 1, "Engine", 1, VK_API_VERSION_1_0);
	deviceVulkan->CreateDebugCallbacks();

	instance = deviceVulkan->instance;
}


void SetupDevice()
{
	deviceVulkan->CreateDevice();

	VmaAllocatorCreateInfo createInfo = VmaAllocatorCreateInfo();
	createInfo.device = VkDevice(deviceVulkan->device);
	createInfo.physicalDevice = VkPhysicalDevice(deviceVulkan->physicalDevice);
	createInfo.flags = VMA_ALLOCATOR_FLAG_BITS_MAX_ENUM;


	// Create custom allocator
	vmaCreateAllocator(&createInfo, &allocator);
}


void SetupSDL()
{
	window = std::make_shared<SDLLowLevelWindow>();

	window->Create(screenWidth, screenHeight);
	
	// Create a Vulkan surface for rendering
	try {
		deviceVulkan->swapchain.surface = createVulkanSurface(instance, window.get());
	}
	catch (const std::exception& e) {
		std::cout << "Failed to create Vulkan surface: " << e.what() << std::endl;
		instance.destroy();
		return;
	}
}


void SetupCommandBuffer()
{
	cmdPool = std::make_unique<CommandpoolVulkan>();
	cmdPool->Create(deviceVulkan->device, deviceVulkan->familyIndexGraphics, CommandPoolCreateFlagBits::eResetCommandBuffer);
	commandBuffers = cmdPool->AllocateBuffer(deviceVulkan->device, CommandBufferLevel::ePrimary, NUM_FRAMES);
}


void SetupSwapchain()
{
	deviceVulkan->CreateSwapchain(screenWidth, screenHeight);
}


void SetupDepthbuffer()
{
	vk::ImageCreateInfo image_Info = {};
	const vk::Format depth_format = vk::Format::eD24UnormS8Uint;

	// Query if the format we supplied is supported by our device
	vk::FormatProperties props = deviceVulkan->physicalDevice.getFormatProperties(depth_format);

	if (props.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
	{
		image_Info.tiling = vk::ImageTiling::eLinear;
	}
	else if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
	{
		image_Info.tiling = vk::ImageTiling::eOptimal;
	}
	else
	{
		std::cout << "VK_FORMAT_D16_UNORM Unsupported.\n" << std::endl;
		exit(-1);
	}


	CreateSimpleImage(allocator, depthBuffer.allocation,
		VMA_MEMORY_USAGE_GPU_ONLY,
		ImageUsageFlagBits::eDepthStencilAttachment,
		vk::Format::eD24UnormS8Uint,
		vk::ImageLayout::eUndefined,
		depthBuffer.image,
		screenWidth, screenHeight);


	vk::ImageViewCreateInfo view_info = vk::ImageViewCreateInfo()
		.setPNext(NULL)
		.setImage(vk::Image(nullptr))
		.setFormat(depth_format)
		.setComponents(vk::ComponentMapping(ComponentSwizzle::eR, ComponentSwizzle::eG, ComponentSwizzle::eB, ComponentSwizzle::eA))
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1))
		.setViewType(ImageViewType::e2D);

	view_info.image = depthBuffer.image;
	depthBuffer.format = Format::eD24UnormS8Uint;

	depthBuffer.view = deviceVulkan->device.createImageView(view_info);
}


void SetupUniformbuffer()
{
	// Create our uniform buffers
	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		UniformBufferVulkan tUniformBuff;

		CreateSimpleBuffer(allocator,
			tUniformBuff.allocation,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			tUniformBuff.buffer,
			vk::BufferUsageFlagBits::eUniformBuffer,
			sizeof(CBMatrix));

		CopyDataToBuffer(VkDevice(deviceVulkan->device), tUniformBuff.allocation, (void*)&matrixConstantBufferData, sizeof(matrixConstantBufferData));


		tUniformBuff.descriptorInfo.buffer = tUniformBuff.buffer;
		tUniformBuff.descriptorInfo.offset = 0;
		tUniformBuff.descriptorInfo.range = sizeof(matrixConstantBufferData);

		uniformBufferMVP.push_back(tUniformBuff);
	}

	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		UniformBufferVulkan tUniformBuff;

		CreateSimpleBuffer(allocator,
			tUniformBuff.allocation,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			tUniformBuff.buffer,
			vk::BufferUsageFlagBits::eUniformBuffer,
			sizeof(CBLights));

		CopyDataToBuffer(VkDevice(deviceVulkan->device), tUniformBuff.allocation, (void*)&lightConstantBufferData, sizeof(CBLights));


		tUniformBuff.descriptorInfo.buffer = tUniformBuff.buffer;
		tUniformBuff.descriptorInfo.offset = 0;
		tUniformBuff.descriptorInfo.range = sizeof(CBLights);

		uniformBufferLights.push_back(tUniformBuff);
	}
}


void UpdateUniformBufferTest(int32_t iCurrentBuff, const NewCamera& iCam, const std::vector<Light>& iLights)
{
	static float derp = 1.0f;
	derp += 0.01f;
	float derp2 = (sinf(derp) + 1.0f) / 2.0f;

	glm::mat4 projectionMatrix = iCam.matrices.perspective;
	glm::mat4 viewMatrix = iCam.matrices.view;//glm::lookAt(glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, 2.0f));
	
	//viewMatrix[1][3] *= -1;
	//projectionMatrix[0][0] *= -1;
	
	glm::mat4 clipMatrix = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);


	matrixConstantBufferData.modelMatrix = modelMatrix;
	matrixConstantBufferData.viewMatrix = viewMatrix;
	matrixConstantBufferData.projectionMatrix = projectionMatrix;
	matrixConstantBufferData.viewProjectMatrix = projectionMatrix * viewMatrix;
	matrixConstantBufferData.mvpMatrix = clipMatrix * projectionMatrix * viewMatrix * modelMatrix;

	matrixConstantBufferData.viewPos = iCam.position;

	CopyDataToBuffer(VkDevice(deviceVulkan->device), uniformBufferMVP[iCurrentBuff].allocation, (void*)&matrixConstantBufferData, sizeof(matrixConstantBufferData));

	lightConstantBufferData.currAmountOfLights = std::min(static_cast<uint32_t>(iLights.size()), (uint32_t)16);

	for (int i = 0; i < lightConstantBufferData.currAmountOfLights; ++i)
	{
		lightConstantBufferData.lights[i] = iLights[i];
	}

	CopyDataToBuffer(VkDevice(deviceVulkan->device), uniformBufferLights[iCurrentBuff].allocation, (void*)&lightConstantBufferData, sizeof(lightConstantBufferData));
}


void SetupPipelineLayout()
{
	desc_layout = std::move(ShaderLayoutparser::CompileShadersIntoLayouts("shaders/vertex.spv", "shaders/frag.spv", deviceVulkan->device));
}


void SetupDescriptorSet()
{
	descriptorPool = std::make_unique<DescriptorPoolVulkan>();

	descriptorPool->Create(deviceVulkan->device, 400, 10, 10, 10, 400);

	for (int i = 0; i < 2; ++i)
	{
		std::vector<DescriptorSet> tDescSet;
		tDescSet.resize(NUM_DESCRIPTOR_SETS);

		tDescSet[0] = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, desc_layout[0], bindings)[0];
		tDescSet[1] = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, desc_layout[1], uniformBinding)[0];

		descriptor_set.push_back(tDescSet);
	}

	std::array<vk::WriteDescriptorSet, 4> textureWrites = {};

	for (auto& e : models)
	{
		e->textureSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, desc_layout[2], textureBinding)[0];

		vk::DescriptorImageInfo albedoImageInfo = {};
		albedoImageInfo.imageView = e->albedoTexture.view;

		textureWrites[0] = {};
		textureWrites[0].pNext = NULL;
		textureWrites[0].dstSet = e->textureSet;
		textureWrites[0].descriptorCount = 1;
		textureWrites[0].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[0].pImageInfo = &albedoImageInfo;
		textureWrites[0].dstArrayElement = 0;
		textureWrites[0].dstBinding = 0;

		vk::DescriptorImageInfo specularImageInfo = {};
		specularImageInfo.imageView = e->specularTexture.view;

		textureWrites[1] = {};
		textureWrites[1].pNext = NULL;
		textureWrites[1].dstSet = e->textureSet;
		textureWrites[1].descriptorCount = 1;
		textureWrites[1].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[1].pImageInfo = &specularImageInfo;
		textureWrites[1].dstArrayElement = 0;
		textureWrites[1].dstBinding = 1;

		vk::DescriptorImageInfo normalmapImageInfo = {};
		normalmapImageInfo.imageView = e->normalTexture.view;

		textureWrites[2] = {};
		textureWrites[2].pNext = NULL;
		textureWrites[2].dstSet = e->textureSet;
		textureWrites[2].descriptorCount = 1;
		textureWrites[2].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[2].pImageInfo = &normalmapImageInfo;
		textureWrites[2].dstArrayElement = 0;
		textureWrites[2].dstBinding = 2;

		vk::DescriptorImageInfo roughnessInfo = {};

		textureWrites[3] = {};
		textureWrites[3].pNext = NULL;
		textureWrites[3].dstSet = e->textureSet;
		textureWrites[3].descriptorCount = 1;
		textureWrites[3].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[3].pImageInfo = &roughnessInfo;
		textureWrites[3].dstArrayElement = 0;
		textureWrites[3].dstBinding = 3;

		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(textureWrites.size()), textureWrites.data(), 0, NULL);
	}

	//descriptor_set[1] = descriptorPool->AllocateDescriptorSet(device, 1, desc_layout[2], uniformBinding)[0];


	for (int i = 0; i < descriptor_set.size(); ++i)
	{
		std::array<vk::WriteDescriptorSet, 1> writes = {};


		// Create image info for the image descriptor
		vk::DescriptorImageInfo pureSamplerInfo = {};

		pureSamplerInfo.imageView = vk::ImageView(nullptr);
		pureSamplerInfo.sampler = testSampler;

		writes[0] = {};
		writes[0].pNext = NULL;
		writes[0].dstSet = descriptor_set[i][0];
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = vk::DescriptorType::eSampler;
		writes[0].pImageInfo = &pureSamplerInfo;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = 0;


		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, NULL);

		std::array<vk::WriteDescriptorSet, 2> uniform_writes = {};

		uniform_writes[0] = {};
		uniform_writes[0].pNext = NULL;
		uniform_writes[0].dstSet = descriptor_set[i][1];
		uniform_writes[0].descriptorCount = 1;
		uniform_writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniform_writes[0].pBufferInfo = &uniformBufferMVP[i].descriptorInfo;
		uniform_writes[0].dstArrayElement = 0;
		uniform_writes[0].dstBinding = 0;

		uniform_writes[1] = {};
		uniform_writes[1].pNext = NULL;
		uniform_writes[1].dstSet = descriptor_set[i][1];
		uniform_writes[1].descriptorCount = 1;
		uniform_writes[1].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniform_writes[1].pBufferInfo = &uniformBufferLights[i].descriptorInfo;
		uniform_writes[1].dstArrayElement = 0;
		uniform_writes[1].dstBinding = 1;


		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(uniform_writes.size()), uniform_writes.data(), 0, NULL);
	}

}


void SetupRenderPass()
{

	vk::AttachmentDescription attachments[3] = {};
	attachments[0].format = deviceVulkan->swapchain.format;
	attachments[0].samples = NUM_SAMPLES;
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	attachments[0].flags = AttachmentDescriptionFlagBits(0);

	attachments[1].format = vk::Format::eD24UnormS8Uint;
	attachments[1].samples = NUM_SAMPLES;
	attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].initialLayout = vk::ImageLayout::eUndefined;
	attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	attachments[1].flags = AttachmentDescriptionFlagBits(0);

	attachments[2].format = postProcBuffer.format;
	attachments[2].samples = NUM_SAMPLES;
	attachments[2].loadOp = vk::AttachmentLoadOp::eLoad;
	attachments[2].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[2].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[2].initialLayout = vk::ImageLayout::eUndefined;
	attachments[2].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	attachments[2].flags = AttachmentDescriptionFlagBits(0);

	std::vector<AttachmentReference> references;

	vk::AttachmentReference color_reference = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference secondColorReference = vk::AttachmentReference()
		.setAttachment(2)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);


	references.push_back(color_reference);
	references.push_back(secondColorReference);


	vk::AttachmentReference depth_reference = vk::AttachmentReference()
		.setAttachment(1)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass = vk::SubpassDescription()
		.setPipelineBindPoint(PipelineBindPoint::eGraphics)
		.setFlags(vk::SubpassDescriptionFlagBits(0))
		.setInputAttachmentCount(0)
		.setPInputAttachments(0)
		.setColorAttachmentCount(2)
		.setPColorAttachments(references.data())
		.setPResolveAttachments(NULL)
		.setPDepthStencilAttachment(&depth_reference)
		.setPreserveAttachmentCount(0)
		.setPResolveAttachments(NULL);

	vk::SubpassDependency dependency = vk::SubpassDependency()
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits(0))
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);


	vk::RenderPassCreateInfo rp_info = RenderPassCreateInfo()
		.setPNext(NULL)
		.setAttachmentCount(3)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	
	renderPassPostProc = deviceVulkan->device.createRenderPass(rp_info);
}

void SetupShaders()
{
	auto vertexShader = CreateShader(deviceVulkan->device, "Shaders/vertex.spv", "main", ShaderStageFlagBits::eVertex);
	auto fragmentShader = CreateShader(deviceVulkan->device, "Shaders/frag.spv", "main", ShaderStageFlagBits::eFragment);

	shaders.push_back(vertexShader);
	shaders.push_back(fragmentShader);

	for (int i = 0; i < 2; ++i)
	{
		PipelineShaderStageCreateInfo shaderCInfo = vk::PipelineShaderStageCreateInfo()
			.setStage(shaders[i].shaderStage)
			.setPName(shaders[i].entryPointName.c_str())
			.setModule(shaders[i].shaderModule);

		shaderStages.push_back(shaderCInfo);
	}
}

vk::Framebuffer CreateFrameBuffer(
	const vk::Device& iDevice,
	const std::vector<vk::ImageView>& iAttachments,
	const uint32_t iWidth, const uint32_t iHeight,
	const vk::RenderPass& iRenderPass)
{
	vk::FramebufferCreateInfo fb_info = vk::FramebufferCreateInfo()
		.setRenderPass(iRenderPass)
		.setAttachmentCount(iAttachments.size())
		.setPAttachments(iAttachments.data())
		.setWidth(iWidth)
		.setHeight(iHeight)
		.setLayers(1);

	vk::Framebuffer tFBuffer = iDevice.createFramebuffer(fb_info);

	return tFBuffer;
}


void SetupFramebuffers()
{

	fbVulkan = std::make_unique<FramebufferVulkan>(screenWidth, screenHeight);
	AttachmentCreateInfo attchCinfo;
	attchCinfo.format = Format::eR8G8B8A8Unorm;
	attchCinfo.height = screenHeight;
	attchCinfo.width = screenWidth;
	attchCinfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
	attchCinfo.layerCount = 1;


	fbVulkan->AddAttachment(deviceVulkan->device, attchCinfo, allocator);
	fbVulkan->CreateRenderpass(deviceVulkan->device);


	std::vector<vk::ImageView> attachments;

	//attachments[2] = postProcBuffer.view;
	attachments.push_back(vk::ImageView(nullptr));
	attachments.push_back(depthBuffer.view);
	attachments.push_back(postProcBuffer.view);

	for (int i = 0; i < deviceVulkan->swapchain.images.size(); ++i)
	{
		attachments[0] = deviceVulkan->swapchain.views[i];
		framebuffers.push_back(CreateFrameBuffer(deviceVulkan->device, attachments, screenWidth, screenHeight, renderPassPostProc));
	}

	//std::vector<vk::ImageView> attachments2;;
	//
	//attachments2.push_back(postProcBuffer.view);
	//attachments2.push_back(depthBuffer.view);
	//CreateFrameBuffer(deviceVulkan->device, attachments2, screenWidth, screenHeight, renderPassPostProc);
}


void CopyBufferMemory(vk::Buffer srcBuffer, vk::Buffer destBuffer, int32_t aSize)
{
	vk::CommandBuffer tCmdBuffer = BeginSingleTimeCommands(deviceVulkan->device, cmdPool->GetPool());

	vk::BufferCopy copyRegion = vk::BufferCopy()
		.setSrcOffset(0)
		.setDstOffset(0)
		.setSize(aSize);

	tCmdBuffer.copyBuffer(srcBuffer, destBuffer, 1, &copyRegion);

	EndSingleTimeCommands(deviceVulkan->device, tCmdBuffer, cmdPool->GetPool(), deviceVulkan->graphicsQueue);
}


void CopyBufferToImage(vk::CommandBuffer iBuffer, vk::Buffer srcBuffer, vk::Image destImage, uint32_t width, uint32_t height)
{
	vk::BufferImageCopy region = vk::BufferImageCopy()
		.setBufferOffset(0)
		.setBufferRowLength(0)
		.setBufferImageHeight(0)
		.setImageOffset(vk::Offset3D(0, 0, 0))
		.setImageExtent(vk::Extent3D(width, height, 1));

	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	iBuffer.copyBufferToImage(
		srcBuffer,
		destImage,
		ImageLayout::eTransferDstOptimal,
		1,
		&region);
}


void SetupIndexBuffer(BufferVulkan& oIndexBuffer, const RawMeshData& iRawMeshData)
{
	BufferVulkan indexBufferStageT;

	CreateSimpleBuffer(
		allocator,
		indexBufferStageT.allocation,
		VMA_MEMORY_USAGE_CPU_ONLY,
		indexBufferStageT.buffer,
		vk::BufferUsageFlagBits::eTransferSrc,
		sizeof(uint32_t) * iRawMeshData.indices.size());

	CopyDataToBuffer(deviceVulkan->device, indexBufferStageT.allocation, (void*)iRawMeshData.indices.data(), sizeof(uint32_t) * iRawMeshData.indices.size());

	CreateSimpleBuffer(
		allocator,
		oIndexBuffer.allocation,
		VMA_MEMORY_USAGE_GPU_ONLY,
		oIndexBuffer.buffer,
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		sizeof(uint32_t) * iRawMeshData.indices.size());

	CopyBufferMemory(indexBufferStageT.buffer, oIndexBuffer.buffer, oIndexBuffer.allocation->GetSize());

	vmaDestroyBuffer(allocator, indexBufferStageT.buffer, indexBufferStageT.allocation);
}

void SetupVertexBuffer(VertexBufferVulkan& oVertexBuffer, const RawMeshData& iRawMeshdata)
{
	size_t dataSize = sizeof(VertexData) * iRawMeshdata.vertices.size();

	BufferVulkan stagingT;

	CreateSimpleBuffer(allocator,
		stagingT.allocation,
		VMA_MEMORY_USAGE_CPU_ONLY,
		stagingT.buffer,
		BufferUsageFlagBits::eTransferSrc,
		dataSize);


	CopyDataToBuffer((VkDevice)deviceVulkan->device,
		stagingT.allocation,
		(void*)iRawMeshdata.vertices.data(),
		dataSize);


	CreateSimpleBuffer(allocator,
		oVertexBuffer.allocation,
		VMA_MEMORY_USAGE_GPU_ONLY,
		oVertexBuffer.buffer,
		BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst,
		dataSize);

	oVertexBuffer.inputDescription.binding = 0;
	oVertexBuffer.inputDescription.inputRate = vk::VertexInputRate::eVertex;
	oVertexBuffer.inputDescription.stride = sizeof(VertexData);

	// 12 bits 
	// 8  bits 
	// 12 bits
	// 12 bits
	// 12 bits
	vk::VertexInputAttributeDescription att1;
	att1.binding = 0;
	att1.location = 0;
	att1.format = vk::Format::eR32G32B32Sfloat;

	vk::VertexInputAttributeDescription att2;
	att2.binding = 0;
	att2.location = 1;
	att2.format = vk::Format::eR32G32Sfloat;
	att2.offset = 12;

	vk::VertexInputAttributeDescription att3;
	att3.binding = 0;
	att3.location = 2;
	att3.format = vk::Format::eR32G32B32Sfloat;
	att3.offset = 20;

	vk::VertexInputAttributeDescription att4;
	att4.binding = 0;
	att4.location = 3;
	att4.format = vk::Format::eR32G32B32Sfloat;
	att4.offset = 32;

	vk::VertexInputAttributeDescription att5;
	att5.binding = 0;
	att5.location = 4;
	att5.format = vk::Format::eR32G32B32Sfloat;
	att5.offset = 44;


	oVertexBuffer.inputAttributes.push_back(att1);
	oVertexBuffer.inputAttributes.push_back(att2);
	oVertexBuffer.inputAttributes.push_back(att3);
	oVertexBuffer.inputAttributes.push_back(att4);
	oVertexBuffer.inputAttributes.push_back(att5);
	// Create staging buffer

	CopyBufferMemory(stagingT.buffer, oVertexBuffer.buffer, oVertexBuffer.allocation->GetSize());

	vmaDestroyBuffer(allocator, stagingT.buffer, stagingT.allocation);
	
}




void SetupPipeline()
{

	vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setPNext(NULL)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(NULL)
		.setSetLayoutCount(NUM_DESCRIPTOR_SETS)
		.setPSetLayouts(desc_layout.data());

	pipelineLayout = deviceVulkan->device.createPipelineLayout(pPipelineLayoutCreateInfo);

	std::vector<vk::DynamicState> dynamicStateEnables;
	dynamicStateEnables.resize(VK_DYNAMIC_STATE_RANGE_SIZE); //[VK_DYNAMIC_STATE_RANGE_SIZE];

	vk::PipelineDynamicStateCreateInfo dynamicState = PipelineDynamicStateCreateInfo()
		.setPDynamicStates(dynamicStateEnables.data())
		.setDynamicStateCount(1);

	assert(models[0]->vertexBuffer.inputAttributes.size() != 0);
	vk::PipelineVertexInputStateCreateInfo vi = vk::PipelineVertexInputStateCreateInfo()
		.setFlags(PipelineVertexInputStateCreateFlagBits(0))
		.setPVertexBindingDescriptions(&models[0]->vertexBuffer.inputDescription)
		.setPVertexAttributeDescriptions(models[0]->vertexBuffer.inputAttributes.data())
		.setVertexAttributeDescriptionCount(5)
		.setVertexBindingDescriptionCount(1);

	vk::PipelineInputAssemblyStateCreateInfo ia = vk::PipelineInputAssemblyStateCreateInfo()
		.setPrimitiveRestartEnable(VK_FALSE)
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	vk::PipelineRasterizationStateCreateInfo rs = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setDepthBiasEnable(VK_FALSE)
		.setDepthBiasConstantFactor(0)
		.setDepthBiasClamp(0)
		.setDepthBiasSlopeFactor(0)
		.setLineWidth(1.0f);


	vk::PipelineColorBlendStateCreateInfo cb = {};

	vk::PipelineColorBlendAttachmentState att_state[2] = {};
	att_state[0].colorWriteMask = vk::ColorComponentFlagBits(0xF);
	att_state[0].blendEnable = VK_FALSE;
	att_state[0].alphaBlendOp = vk::BlendOp::eAdd;
	att_state[0].colorBlendOp = vk::BlendOp::eAdd;
	att_state[0].srcColorBlendFactor = vk::BlendFactor::eZero;
	att_state[0].dstColorBlendFactor = vk::BlendFactor::eZero;
	att_state[0].srcAlphaBlendFactor = vk::BlendFactor::eZero;
	att_state[0].dstAlphaBlendFactor = vk::BlendFactor::eZero;

	att_state[1].colorWriteMask = vk::ColorComponentFlagBits(0xF);
	att_state[1].blendEnable = VK_FALSE;
	att_state[1].alphaBlendOp = vk::BlendOp::eAdd;
	att_state[1].colorBlendOp = vk::BlendOp::eAdd;
	att_state[1].srcColorBlendFactor = vk::BlendFactor::eZero;
	att_state[1].dstColorBlendFactor = vk::BlendFactor::eZero;
	att_state[1].srcAlphaBlendFactor = vk::BlendFactor::eZero;
	att_state[1].dstAlphaBlendFactor = vk::BlendFactor::eZero;

	cb.attachmentCount = 2;
	cb.pAttachments = att_state;
	cb.logicOpEnable = VK_FALSE;
	cb.logicOp = vk::LogicOp::eNoOp;
	cb.blendConstants[0] = 1.0f;
	cb.blendConstants[1] = 1.0f;
	cb.blendConstants[2] = 1.0f;
	cb.blendConstants[3] = 1.0f;


	vk::PipelineViewportStateCreateInfo vp = vk::PipelineViewportStateCreateInfo()
		.setViewportCount(1)
		.setScissorCount(1)
		.setPScissors(NULL)
		.setPViewports(NULL);

	dynamicStateEnables[dynamicState.dynamicStateCount++] = vk::DynamicState::eViewport;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = vk::DynamicState::eScissor;

	vk::PipelineDepthStencilStateCreateInfo ds = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(VK_TRUE)
		.setDepthWriteEnable(VK_TRUE)
		.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setMinDepthBounds(0)
		.setMaxDepthBounds(1.0f)
		.setStencilTestEnable(VK_FALSE);
	//.setBack(vk::StencilOpState(StencilOp::eKeep, StencilOp::eKeep, StencilOp::eKeep, CompareOp::eAlways, 0));

	//ds.setFront(ds.back);

	vk::PipelineMultisampleStateCreateInfo ms = PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(NUM_SAMPLES)
		.setSampleShadingEnable(VK_FALSE)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE)
		.setMinSampleShading(0.0f)
		.setPSampleMask(VK_NULL_HANDLE);


	vk::GraphicsPipelineCreateInfo gfxPipe = GraphicsPipelineCreateInfo()
		.setLayout(pipelineLayout)
		.setBasePipelineHandle(nullptr)
		.setBasePipelineIndex(0)
		.setPVertexInputState(&vi)
		.setPInputAssemblyState(&ia)
		.setPRasterizationState(&rs)
		.setPColorBlendState(&cb)
		.setPTessellationState(VK_NULL_HANDLE)
		.setPMultisampleState(&ms)
		.setPDynamicState(&dynamicState)
		.setPViewportState(&vp)
		.setPDepthStencilState(&ds)
		.setPStages(shaderStages.data())
		.setStageCount(2)
		.setRenderPass(renderPassPostProc)
		.setSubpass(0);

	pipeline = deviceVulkan->device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipe);


	vk::PipelineColorBlendAttachmentState att_state2[1] = {};
	att_state[0].colorWriteMask = vk::ColorComponentFlagBits(0xF);
	att_state[0].blendEnable = VK_FALSE;
	att_state[0].alphaBlendOp = vk::BlendOp::eAdd;
	att_state[0].colorBlendOp = vk::BlendOp::eAdd;
	att_state[0].srcColorBlendFactor = vk::BlendFactor::eZero;
	att_state[0].dstColorBlendFactor = vk::BlendFactor::eZero;
	att_state[0].srcAlphaBlendFactor = vk::BlendFactor::eZero;
	att_state[0].dstAlphaBlendFactor = vk::BlendFactor::eZero;


	vk::PipelineColorBlendStateCreateInfo cb2 = {};

	cb2.attachmentCount = 1;
	cb2.pAttachments = att_state;
	cb2.logicOpEnable = VK_FALSE;
	cb2.logicOp = vk::LogicOp::eNoOp;
	cb2.blendConstants[0] = 1.0f;	
	cb2.blendConstants[1] = 1.0f;
	cb2.blendConstants[2] = 1.0f;
	cb2.blendConstants[3] = 1.0f;

	vk::GraphicsPipelineCreateInfo gfxPipe2 = GraphicsPipelineCreateInfo()
		.setLayout(pipelineLayout)
		.setBasePipelineHandle(nullptr)
		.setBasePipelineIndex(0)
		.setPVertexInputState(&vi)
		.setPInputAssemblyState(&ia)
		.setPRasterizationState(&rs)
		.setPColorBlendState(&cb2)
		.setPTessellationState(VK_NULL_HANDLE)
		.setPMultisampleState(&ms)
		.setPDynamicState(&dynamicState)
		.setPViewportState(&vp)
		.setPDepthStencilState(&ds)
		.setPStages(shaderStages.data())
		.setStageCount(2)
		.setRenderPass(fbVulkan->renderpass)
		.setSubpass(0);

	pipelineRenderScene = deviceVulkan->device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipe2);

	//vk::PipelineDynamicStateCreateInfo dynamicState = vk
}

void InitViewports(vk::CommandBuffer aBuffer)
{
	viewPort.height = (float)screenHeight;
	viewPort.width = (float)screenWidth;
	viewPort.minDepth = (float)0.0f;
	viewPort.maxDepth = (float)1.0f;
	viewPort.x = 0;
	viewPort.y = 0;

	aBuffer.setViewport(0, 1, &viewPort);
}

void InitScissors(vk::CommandBuffer aBuffer)
{
	scissor.extent = vk::Extent2D(screenWidth, screenHeight);
	scissor.offset = vk::Offset2D(0, 0);

	aBuffer.setScissor(0, 1, &scissor);
}



vk::Fence graphicsQueueFinishedFence;
void SetupSemaphores()
{
	vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();
	imageAcquiredSemaphore = deviceVulkan->device.createSemaphore(semaphoreInfo);
	rendererFinishedSemaphore = deviceVulkan->device.createSemaphore(semaphoreInfo);

	vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo();
	graphicsQueueFinishedFence = deviceVulkan->device.createFence(fenceInfo);
}


void SetupCommandBuffers(const vk::CommandBuffer& iBuffer, uint32_t index, const std::vector<Object>& iObjects)
{
	vk::CommandBufferBeginInfo cmd_begin = vk::CommandBufferBeginInfo();

	iBuffer.begin(cmd_begin);

	vk::ClearValue clear_values[3] = {};
	clear_values[0].color.float32[0] = 0.2f;
	clear_values[0].color.float32[1] = 0.2f;
	clear_values[0].color.float32[2] = 0.2f;
	clear_values[0].color.float32[3] = 0.2f;
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0;


	clear_values[2].color.float32[0] = 0.2f;
	clear_values[2].color.float32[1] = 0.8f;
	clear_values[2].color.float32[2] = 0.2f;
	clear_values[2].color.float32[3] = 1.0f;

	const vk::DeviceSize offsets[1] = { 0 };

	vk::RenderPassBeginInfo rp_begin = vk::RenderPassBeginInfo()
		.setRenderPass(renderPassPostProc)
		.setFramebuffer(framebuffers[index])
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(screenWidth, screenHeight)))
		.setClearValueCount(3)
		.setPClearValues(clear_values);

	iBuffer.beginRenderPass(&rp_begin, SubpassContents::eInline);
	iBuffer.bindPipeline(PipelineBindPoint::eGraphics, pipeline);

	
	InitViewports(iBuffer);
	InitScissors(iBuffer);


	for (int j = 0; j < iObjects.size(); ++j)
	{
		descriptor_set[index][2] = models[iObjects[j].vulkanModelID]->textureSet;
		iBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, NUM_DESCRIPTOR_SETS, descriptor_set[index].data(), 0, NULL);
		iBuffer.bindVertexBuffers(0, 1, &models[iObjects[j].vulkanModelID]->vertexBuffer.buffer, offsets);
		iBuffer.bindIndexBuffer(models[iObjects[j].vulkanModelID]->indexBuffer.buffer, 0, IndexType::eUint32);
		iBuffer.drawIndexed(models[iObjects[j].vulkanModelID]->indiceCount, 1, 0, 0, 0);
	}

	iBuffer.endRenderPass();


	//vk::ClearValue clear_values2[3] = {};
	//clear_values[0].color.float32[0] = 1.0f;
	//clear_values[0].color.float32[1] = 0.2f;
	//clear_values[0].color.float32[2] = 0.2f;
	//clear_values[0].color.float32[3] = 0.2f;
	//
	//vk::RenderPassBeginInfo rp_begin2 = vk::RenderPassBeginInfo()
	//	.setRenderPass(fbVulkan->renderpass)
	//	.setFramebuffer(fbVulkan->framebuffer)
	//	.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(screenWidth, screenHeight)))
	//	.setClearValueCount(1)
	//	.setPClearValues(clear_values);
	//
	//iBuffer.beginRenderPass(&rp_begin2, SubpassContents::eInline);
	//iBuffer.bindPipeline(PipelineBindPoint::eGraphics, pipelineRenderScene);
	//
	//InitViewports(iBuffer);
	//InitScissors(iBuffer);
	//
	//for (int j = 0; j < iObjects.size(); ++j)
	//{
	//	descriptor_set[index][2] = models[iObjects[j].vulkanModelID]->textureSet;
	//	iBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, NUM_DESCRIPTOR_SETS, descriptor_set[index].data(), 0, NULL);
	//	iBuffer.bindVertexBuffers(0, 1, &models[iObjects[j].vulkanModelID]->vertexBuffer.buffer, offsets);
	//	iBuffer.bindIndexBuffer(models[iObjects[j].vulkanModelID]->indexBuffer.buffer, 0, IndexType::eUint32);
	//	iBuffer.drawIndexed(models[iObjects[j].vulkanModelID]->indiceCount, 1, 0, 0, 0);
	//}
	//
	//iBuffer.endRenderPass();

	// End the pipeline
	iBuffer.end();

}

void SetupRTTexture(
	vk::Device iDevice, 
	uint32_t iTextureWidth, uint32_t iTextureHeight, 
	vk::Image& oImage, VmaAllocation& oAllocation,
	vk::ImageView& oImageView)
{
	CreateSimpleImage(allocator,
		oAllocation,
		VMA_MEMORY_USAGE_GPU_ONLY,
		ImageUsageFlagBits::eColorAttachment | ImageUsageFlagBits::eSampled,
		Format::eR8G8B8A8Unorm, ImageLayout::eUndefined,
		oImage, iTextureWidth, iTextureHeight);

	oImageView = CreateImageView(iDevice, oImage, Format::eR8G8B8A8Unorm);


	//std::vector<vk::AttachmentDescription> attachmentDescriptions = {};

	//vk::AttachmentDescription renderTargetAttachment;
	//
	//renderTargetAttachment.format = vk::Format::eR8G8B8Unorm;
	//renderTargetAttachment.samples = NUM_SAMPLES;
	//renderTargetAttachment.loadOp = AttachmentLoadOp::eClear;
	//renderTargetAttachment.storeOp = AttachmentStoreOp::eStore;
	//renderTargetAttachment.stencilLoadOp = AttachmentLoadOp::eDontCare;
	//renderTargetAttachment.stencilStoreOp= AttachmentStoreOp::eDontCare;
	//renderTargetAttachment.initialLayout = vk::ImageLayout::eUndefined;
	//renderTargetAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	//
	//vk::AttachmentDescription depthTargetAttachment;
	//
	//renderTargetAttachment.format = vk::Format::eR8G8B8Unorm;
	//renderTargetAttachment.samples = NUM_SAMPLES;
	//renderTargetAttachment.loadOp = AttachmentLoadOp::eClear;
	//renderTargetAttachment.storeOp = AttachmentStoreOp::eStore;
	//renderTargetAttachment.stencilLoadOp = AttachmentLoadOp::eDontCare;
	//renderTargetAttachment.stencilStoreOp = AttachmentStoreOp::eDontCare;
	//renderTargetAttachment.initialLayout = vk::ImageLayout::eUndefined;
	//renderTargetAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	//vk::AttachmentReference{ 0, vk::ImageLayout::eColorAttachmentOptimal };
	//vk::AttachmentReference{ 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };

}

stbi_uc* LoadTexture(const std::string& iFilePath, int& oWidth, int& oHeight, int& oTexChannels, uint64_t& oImageSize)
{
	// Load texture with stbi
	stbi_uc* pixels = stbi_load(iFilePath.c_str(), &oWidth, &oHeight, &oTexChannels, STBI_rgb_alpha);

	oImageSize = oWidth * oHeight * 4;

	if (!pixels)
	{
		stbi_image_free(pixels);

		LOG(ERROR) << "Load texture failed! Fallback to error texture..";
		pixels = stbi_load("textures/ErrorTexture.png", &oWidth, &oHeight, &oTexChannels, STBI_rgb_alpha);
		oImageSize = oWidth * oHeight * 4;
		if (!pixels)
		{
			LOG(FATAL) << "FAILED TO LOAD ERRORTEXTURE, THIS SHOULD NOT HAPPEN";
		}
	}

	return pixels;
}


void SetupTextureImage(vk::CommandBuffer iBuffer, vk::Device iDevice, std::string iFilePath, vk::Image& oImage, VmaAllocation& oAllocation, std::vector<BufferVulkan>& oStaging)
{
	int texWidth = 0;
	int texHeight = 0;
	int texChannels = 0;
	uint64_t imageSize = 0;

	stbi_uc* pixels = LoadTexture(iFilePath, texWidth, texHeight, texChannels, imageSize);

	// Create staging buffer for image
	BufferVulkan stagingBuffer;

	// create simple buffer
	CreateSimpleBuffer(allocator,
		stagingBuffer.allocation,
		VMA_MEMORY_USAGE_CPU_ONLY,
		stagingBuffer.buffer,
		BufferUsageFlagBits::eTransferSrc,
		vk::DeviceSize(imageSize));

	// Copy image data to buffer
	CopyDataToBuffer(VkDevice(deviceVulkan->device), stagingBuffer.allocation, pixels, imageSize);

	// Free image
	stbi_image_free(pixels);


	CreateSimpleImage(allocator,
		oAllocation,
		VMA_MEMORY_USAGE_GPU_ONLY,
		ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eSampled,
		Format::eR8G8B8A8Unorm, ImageLayout::eUndefined,
		oImage, texWidth, texHeight);

	TransitionImageLayout(iBuffer,
		oImage,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal);

	CopyBufferToImage(iBuffer, stagingBuffer.buffer, oImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	TransitionImageLayout(iBuffer,
		oImage,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal);


	oStaging.push_back(stagingBuffer);
}


void CreateTextureSampler(vk::Device const aDevice)
{
	vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo()
		.setMagFilter(Filter::eLinear)
		.setMinFilter(Filter::eLinear)
		.setAddressModeU(SamplerAddressMode::eRepeat)
		.setAddressModeV(SamplerAddressMode::eRepeat)
		.setAddressModeW(SamplerAddressMode::eRepeat)
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(16)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setBorderColor(BorderColor::eIntOpaqueBlack)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(CompareOp::eAlways)
		.setMipmapMode(SamplerMipmapMode::eLinear)
		.setMipLodBias(0.0f)
		.setMinLod(0.0f)
		.setMaxLod(0.0f);

	testImageSampler = aDevice.createSampler(samplerInfo);

	testSampler = aDevice.createSampler(samplerInfo);
}

bool firstFrame = true;
void RendererVulkan::Render(const std::vector<Object>& iObjects)
{
	if (firstFrame)
	{
		SetupCommandBuffers(commandBuffers[currentBuffer], currentBuffer, iObjects);
		firstFrame = false;
	}

	else
	{
		//std::thread CommandSetup = std::thread(SetupCommandBuffers, commandBuffers[(currentBuffer + 1) % 2], (currentBuffer + 1) % 2);
		SetupCommandBuffers(commandBuffers[(currentBuffer + 1) % 2], (currentBuffer + 1) % 2, iObjects);

		deviceVulkan->device.waitForFences(1, &graphicsQueueFinishedFence, vk::Bool32(true), FENCE_TIMEOUT);
		//LOG(INFO) << "FENCE WAITING OVER";
		//LOG(INFO) << "Current buffer " << currentBuffer;
		deviceVulkan->device.resetFences(graphicsQueueFinishedFence);
		//CommandSetup.join();
	}

	deviceVulkan->device.acquireNextImageKHR(deviceVulkan->swapchain.swapchain, UINT64_MAX, imageAcquiredSemaphore, vk::Fence(nullptr), &currentBuffer);
	vmaSetCurrentFrameIndex(allocator, currentBuffer);


	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::SubmitInfo submit_info[1] = {};
	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = &commandBuffers[currentBuffer];
	submit_info[0].signalSemaphoreCount = 1;
	submit_info[0].pSignalSemaphores = &rendererFinishedSemaphore;

	deviceVulkan->graphicsQueue.submit(1, submit_info, graphicsQueueFinishedFence);

	vk::PresentInfoKHR present;
	present.swapchainCount = 1;
	present.pSwapchains = &deviceVulkan->swapchain.swapchain;
	present.pImageIndices = &currentBuffer;
	present.pWaitSemaphores = &rendererFinishedSemaphore;	//
	present.waitSemaphoreCount = 1;
	present.pResults = 0;

	deviceVulkan->presentQueue.presentKHR(&present);

	//presentQueue.waitIdle();
}

void RendererVulkan::BeginFrame(const NewCamera& iCamera, const std::vector<Light>& lights)
{
	UpdateUniformBufferTest((currentBuffer + 1) % 2, iCamera, lights);
}

std::map<std::string, TextureVulkan> textureMap;

void RendererVulkan::Create(std::vector<Object>& iMeshes)
{

	SetupApplication();
	SetupSDL();
	SetupDevice();

	SetupSwapchain();

	SetupUniformbuffer();
	SetupPipelineLayout();
	SetupShaders();

	SetupDepthbuffer();
	SetupRTTexture(deviceVulkan->device, screenWidth, screenHeight, postProcBuffer.image, postProcBuffer.allocation, postProcBuffer.view);
	postProcBuffer.format = Format::eR8G8B8A8Unorm;

	SetupRenderPass();

	SetupFramebuffers();
	SetupCommandBuffer();

	deviceVulkan->SetupDeviceQueue();
	
	
	vk::CommandBuffer stageBuffer = BeginSingleTimeCommands(deviceVulkan->device, cmdPool->GetPool());
	TransitionImageLayout(stageBuffer, depthBuffer.image, depthBuffer.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	// Staging buffers we will need to delete
	std::vector<BufferVulkan> stagingBuffers;

	// load in albedo textures in this loop
	for (int i = 0; i < iMeshes.size(); ++i)
	{
		auto& e = iMeshes[i].rawMeshData;
		std::unique_ptr<ModelVulkan> tModV = std::make_unique<ModelVulkan>();
		SetupVertexBuffer(tModV->vertexBuffer, e);
		SetupIndexBuffer(tModV->indexBuffer, e);

		tModV->indiceCount = e.indices.size();

		// Check if the texture already exists in the map
		// Albedo texture
		if (textureMap.find(e.filepaths[0]) == textureMap.end())
		{
			SetupTextureImage(stageBuffer, deviceVulkan->device, e.filepaths[0], tModV->albedoTexture.image, tModV->albedoTexture.allocation, stagingBuffers);
			tModV->albedoTexture.view = CreateImageView(deviceVulkan->device, tModV->albedoTexture.image, Format::eR8G8B8A8Unorm);
			textureMap[e.filepaths[0]] = tModV->albedoTexture;
		}
		else
		{
			tModV->albedoTexture = textureMap.at(e.filepaths[0]);
		}

		// Specular texture
		if (textureMap.find(e.filepaths[1]) == textureMap.end())
		{
			SetupTextureImage(stageBuffer, deviceVulkan->device, e.filepaths[1].c_str(), tModV->specularTexture.image, tModV->specularTexture.allocation, stagingBuffers);
			tModV->specularTexture.view = CreateImageView(deviceVulkan->device, tModV->specularTexture.image, Format::eR8G8B8A8Unorm);
			textureMap[e.filepaths[1]] = tModV->specularTexture;
		}
		else
		{
			tModV->specularTexture = textureMap.at(e.filepaths[1]);
		}
		
		// normal texture
		if (textureMap.find(e.filepaths[2]) == textureMap.end())
		{
			SetupTextureImage(stageBuffer, deviceVulkan->device, e.filepaths[2].c_str(), tModV->normalTexture.image, tModV->normalTexture.allocation, stagingBuffers);
			tModV->normalTexture.view = CreateImageView(deviceVulkan->device, tModV->normalTexture.image, Format::eR8G8B8A8Unorm);
			textureMap[e.filepaths[2]] = tModV->normalTexture;
		}
		else
		{
			tModV->normalTexture = textureMap.at(e.filepaths[2]);
		}

		if (textureMap.find(e.filepaths[3]) == textureMap.end())
		{
			SetupTextureImage(stageBuffer, deviceVulkan->device, e.filepaths[3].c_str(), tModV->roughnessTexture.image, tModV->roughnessTexture.allocation, stagingBuffers);
			tModV->roughnessTexture.view = CreateImageView(deviceVulkan->device, tModV->roughnessTexture.image, Format::eR8G8B8A8Unorm);
			textureMap[e.filepaths[3]] = tModV->roughnessTexture;
		}

		else
		{
			tModV->roughnessTexture = textureMap.at(e.filepaths[3]);
		}




		models.push_back(std::move(tModV));
		iMeshes[i].vulkanModelID = models.size();
	}
	
	EndSingleTimeCommands(deviceVulkan->device, stageBuffer, cmdPool->GetPool(), deviceVulkan->graphicsQueue);

	for (auto& e : stagingBuffers)
	{
		vmaDestroyBuffer(allocator, e.buffer, e.allocation);
	}
	stagingBuffers.clear();
	stagingBuffers.resize(1);

	SetupPipeline();
	SetupSemaphores();

	CreateTextureSampler(deviceVulkan->device);

	SetupDescriptorSet();
	iMeshes.clear();
	iMeshes.resize(1);
}

void RendererVulkan::Destroy()
{

	deviceVulkan->presentQueue.waitIdle();
	
	fbVulkan->Destroy(allocator, deviceVulkan->device);

	for (auto& e : commandBuffers)
	{
		e.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	}

	descriptorPool->Destroy(deviceVulkan->device);

	deviceVulkan->device.destroySemaphore(rendererFinishedSemaphore);
	deviceVulkan->device.destroySemaphore(imageAcquiredSemaphore);

	deviceVulkan->device.destroySampler(testSampler);
	deviceVulkan->device.destroySampler(testImageSampler);


	deviceVulkan->device.destroyFence(graphicsQueueFinishedFence);

	for (auto& e : shaders)
	{
		deviceVulkan->device.destroyShaderModule(e.shaderModule);
	}

	cmdPool->Destroy(deviceVulkan->device);
	//device.destroyCommandPool(commandPool);
	//vice.destroySwapchainKHR(swapchain.swapchain);

	vmaDestroyImage(allocator, (VkImage)depthBuffer.image, depthBuffer.allocation);
	deviceVulkan->device.destroyImageView(depthBuffer.view);

	for (auto& e : desc_layout)
	{
		deviceVulkan->device.destroyDescriptorSetLayout(e);
	}

	for (auto& e : framebuffers)
	{
		deviceVulkan->device.destroyFramebuffer(e);
	}

	deviceVulkan->device.destroyPipeline(pipeline);
	deviceVulkan->device.destroyPipeline(pipelineRenderScene);
	deviceVulkan->device.destroyPipelineLayout(pipelineLayout);
	deviceVulkan->device.destroyRenderPass(renderPassPostProc);

	for (auto& e : uniformBufferMVP)
	{
		vmaDestroyBuffer(allocator, (VkBuffer)e.buffer, e.allocation);
	}

	for (auto& e : uniformBufferLights)
	{
		vmaDestroyBuffer(allocator, (VkBuffer)e.buffer, e.allocation);
	}

	for (auto& e : models)
	{
		vmaDestroyBuffer(allocator, e->vertexBuffer.buffer, e->vertexBuffer.allocation);
		vmaDestroyBuffer(allocator, e->indexBuffer.buffer, e->indexBuffer.allocation);
		//vmaDestroyImage(allocator, e->albedoTexture.image, e->albedoTexture.allocation);
		//vmaDestroyImage(allocator, e->normalTexture.image, e->normalTexture.allocation);
		//vmaDestroyImage(allocator, e->specularTexture.image, e->specularTexture.allocation);

		//deviceVulkan->device.destroyImageView(e->albedoTexture.view);
	}

	for (auto& e: textureMap)
	{
		vmaDestroyImage(allocator, e.second.image, e.second.allocation);
		deviceVulkan->device.destroyImageView(e.second.view);
	}

	vmaDestroyImage(allocator, postProcBuffer.image, postProcBuffer.allocation);
	deviceVulkan->device.destroyImageView(postProcBuffer.view);
	//device.destroyImage(depthImage);
	//device.destroyImage(testTexture.image);

	// Clean up.
	vmaDestroyAllocator(allocator);
	window->Destroy();
	deviceVulkan->device.waitIdle();
	deviceVulkan->device.destroy();
	instance.destroySurfaceKHR(deviceVulkan->swapchain.surface);
	instance.destroy();
}