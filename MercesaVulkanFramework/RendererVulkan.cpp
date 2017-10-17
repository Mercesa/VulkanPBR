#include "RendererVulkan.h"


#include <stdio.h>
#include <stdlib.h>
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
#include "ShaderProgramVulkan.h"
#include "ModelVulkan.h"
#include "TextureVulkan.h"
#include "ObjectRenderingDataVulkan.h"
#include "GLFWLowLevelWindow.h"

#include <imgui.h>

#include <imgui_impl_glfw_vulkan.h>

CBModelMatrixSingle singleModelmatrixData;
CBMatrix matrixConstantBufferData;
CBLights lightConstantBufferData;


#define NUM_SAMPLES vk::SampleCountFlagBits::e1
#define NUM_DESCRIPTOR_SETS 4
#define FENCE_TIMEOUT 100000000
#define NUM_FRAMES 2

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, iLowLevelWindow* window);

static const int screenWidth = 1280;
static const int screenHeight = 720;

using namespace vk;


struct ShaderResourcesPBR
{
	vk::DescriptorSet samplerSet;
	vk::DescriptorSet perFrameUniformBufferSet;
	vk::DescriptorSet perObjectUniformBufferSet;
	vk::DescriptorSet textureSet;
};

struct RenderingContextResources
{
	ShaderResourcesPBR descriptorSetPBRShader;

	UniformBufferVulkan uniformBufferMVP;
	UniformBufferVulkan uniformBufferModelMatrix;
	UniformBufferVulkan uniformBufferLights;

	vk::QueryPool queryPool;
	vk::CommandBuffer commandBuffer;
	vk::CommandBuffer commandBufferIMGUI;
};


std::vector<vk::Framebuffer> framebuffers;
std::vector<vk::Framebuffer> framebuffersImgui;

std::unique_ptr<FramebufferVulkan> framebufferRenderScene;

vk::PipelineLayout pipelineLayout;
vk::PipelineLayout pipelineLayoutRenderScene
;

TextureData depthBuffer;
TextureData postProcBuffer;


std::vector<std::unique_ptr<RenderingContextResources>> renderingContextResources;

vk::RenderPass renderPassPostProc;

// Device and instance
vk::Instance instance;

std::unique_ptr<CommandpoolVulkan> cmdPool;

// Information about our device its memory and family properties

std::unique_ptr<DescriptorPoolVulkan> descriptorPool;

std::vector<ModelVulkan*> models;
std::vector<TextureVulkan*> textures;
std::vector<ObjectRenderingDataVulkan*> objRenderingData;

VmaAllocator allocator;

vk::Pipeline pipelinePBR;
vk::Pipeline pipelineRed;

vk::Viewport viewPort;
vk::Rect2D scissor;


vk::Sampler testImageSampler;
vk::Sampler testSampler;

vk::Semaphore imageAcquiredSemaphore;
vk::Semaphore rendererFinishedSemaphore;

std::vector<vk::DescriptorSetLayoutBinding> bindings;
std::vector<vk::DescriptorSetLayoutBinding> uniformBinding;
std::vector<vk::DescriptorSetLayoutBinding> textureBinding;

vk::Fence graphicsQueueFinishedFence;

std::unique_ptr<DeviceVulkan> deviceVulkan;

std::unique_ptr<ShaderProgramVulkan> shaderProgramPBR;
std::unique_ptr<ShaderProgramVulkan> shaderProgramRed;
std::unique_ptr<ShaderProgramVulkan> shaderProgramPostProc;


struct imguiData
{
	vk::RenderPass renderpass;
	vk::DescriptorPool descriptorPool;

} imguiDataObj;

RendererVulkan::RendererVulkan()
{
}


RendererVulkan::~RendererVulkan()
{
}

static void check_vk_result(VkResult err)
{
	if (err == 0) return;
	printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}


void RendererVulkan::SetupIMGUI(iLowLevelWindow* const iIlowLevelWindow)
{

	// Create special pool for imgui
	VkDescriptorPoolSize pool_size[11] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100},
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 100 * 11;
	pool_info.poolSizeCount = 11;
	pool_info.pPoolSizes = pool_size;

	VkDescriptorPool tPool;
	vkCreateDescriptorPool((VkDevice)deviceVulkan->device, &pool_info, nullptr, &tPool);
	imguiDataObj.descriptorPool = tPool;
	
	VkAttachmentDescription attachment = {};
	attachment.format = VkFormat(deviceVulkan->swapchain.format);
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference color_attachment = {};
	color_attachment.attachment = 0;
	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;
	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &attachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;

	VkRenderPass tRP = VK_NULL_HANDLE;
	vkCreateRenderPass(deviceVulkan->device, &info, nullptr, &tRP);
	imguiDataObj.renderpass = tRP;

	{
		vk::ImageView attachment[1];
		vk::FramebufferCreateInfo fbInfo = {};
		fbInfo.renderPass = imguiDataObj.renderpass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = attachment;
		fbInfo.width = screenWidth;
		fbInfo.height = screenHeight;
		fbInfo.layers = 1;
		for (uint32_t i = 0; i < deviceVulkan->swapchain.views.size(); i++)
		{
			attachment[0] = deviceVulkan->swapchain.views[i];
			framebuffersImgui.push_back(deviceVulkan->device.createFramebuffer(fbInfo, nullptr));
		}

	}

	ImGui_ImplGlfwVulkan_Init_Data imgui_init_data;
	imgui_init_data.allocator = VK_NULL_HANDLE;
	imgui_init_data.device = deviceVulkan->device;
	imgui_init_data.gpu = deviceVulkan->physicalDevice;
	imgui_init_data.pipeline_cache = VK_NULL_HANDLE;
	imgui_init_data.render_pass = tRP;
	imgui_init_data.descriptor_pool = tPool;
	imgui_init_data.check_vk_result = check_vk_result;

	GLFWLowLevelWindow* glfwWindow = dynamic_cast<GLFWLowLevelWindow*>(iIlowLevelWindow);
	ImGui_ImplGlfwVulkan_Init(glfwWindow->window, false, &imgui_init_data);

	{
		vk::CommandBufferBeginInfo begin_info = vk::CommandBufferBeginInfo();

		renderingContextResources[currentBuffer]->commandBufferIMGUI.begin(begin_info);
		ImGui_ImplGlfwVulkan_CreateFontsTexture(renderingContextResources[currentBuffer]->commandBufferIMGUI);
		renderingContextResources[currentBuffer]->commandBufferIMGUI.end();

		vk::SubmitInfo end_info = vk::SubmitInfo()
			.setCommandBufferCount(1)
			.setPCommandBuffers(&renderingContextResources[currentBuffer]->commandBufferIMGUI);
		deviceVulkan->graphicsQueue.submit(end_info, vk::Fence(nullptr));
		deviceVulkan->device.waitIdle();
		ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();
	}
}

void SetupApplication(iLowLevelWindow* const iLowLevelWindow)
{

	deviceVulkan = std::make_unique<DeviceVulkan>();
	deviceVulkan->CreateInstance("PBR Vulkan", 1, "Engine", 1, VK_API_VERSION_1_0, iLowLevelWindow->GetRequiredExtensions());
	deviceVulkan->CreateDebugCallbacks();
	instance = deviceVulkan->instance;

	GLFWLowLevelWindow* tempWindow = dynamic_cast<GLFWLowLevelWindow*>(iLowLevelWindow);

	// Create a Vulkan surface for rendering
	try {
		VkSurfaceKHR surface;

		glfwCreateWindowSurface(
			deviceVulkan->instance, 
			tempWindow->window, nullptr, &surface);

		deviceVulkan->swapchain.surface = surface;
	}
	catch (const std::exception& e) {
		std::cout << "Failed to create Vulkan surface: " << e.what() << std::endl;
		instance.destroy();
		return;
	}
	// vk::ApplicationInfo allows the programmer to specifiy some basic information about the
	// program, which can be useful for layers and tools to provide more debug information.
	
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


void SetupCommandBuffer()
{
	cmdPool = std::make_unique<CommandpoolVulkan>();
	cmdPool->Create(deviceVulkan->device, deviceVulkan->familyIndexGraphics, CommandPoolCreateFlagBits::eResetCommandBuffer);

	for (int i = 0; i < renderingContextResources.size(); ++i)
	{
		renderingContextResources[i]->commandBuffer = cmdPool->AllocateBuffer(deviceVulkan->device, CommandBufferLevel::ePrimary, 1)[0];
		renderingContextResources[i]->commandBufferIMGUI = cmdPool->AllocateBuffer(deviceVulkan->device, CommandBufferLevel::ePrimary, 1)[0];
	}
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

		renderingContextResources[i]->uniformBufferMVP = tUniformBuff;
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

		renderingContextResources[i]->uniformBufferLights = tUniformBuff;
	}
}

void UpdateUniformbufferFrame(int32_t iCurrentBuff, const NewCamera& iCam, const std::vector<Light>& iLights)
{
	static float derp = 1.0f;
	derp += 0.01f;
	float derp2 = (sinf(derp) + 1.0f) / 2.0f;

	glm::mat4 projectionMatrix = iCam.matrices.perspective;
	glm::mat4 viewMatrix = iCam.matrices.view;//glm::lookAt(glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, 2.0f));

	
	glm::mat4 clipMatrix = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);


	matrixConstantBufferData.modelMatrix = modelMatrix;
	matrixConstantBufferData.viewMatrix = viewMatrix;
	matrixConstantBufferData.projectionMatrix = projectionMatrix;
	matrixConstantBufferData.viewProjectMatrix = clipMatrix * projectionMatrix * viewMatrix;
	matrixConstantBufferData.mvpMatrix = clipMatrix * projectionMatrix * viewMatrix * modelMatrix;

	matrixConstantBufferData.viewPos = iCam.position;

	CopyDataToBuffer(VkDevice(deviceVulkan->device), renderingContextResources[iCurrentBuff]->uniformBufferMVP.allocation, (void*)&matrixConstantBufferData, sizeof(matrixConstantBufferData));

	lightConstantBufferData.currAmountOfLights = std::min(static_cast<uint32_t>(iLights.size()), (uint32_t)16);

	for (int i = 0; i < lightConstantBufferData.currAmountOfLights; ++i)
	{
		lightConstantBufferData.lights[i] = iLights[i];
	}

	CopyDataToBuffer(VkDevice(deviceVulkan->device), renderingContextResources[iCurrentBuff]->uniformBufferLights.allocation, (void*)&lightConstantBufferData, sizeof(lightConstantBufferData));
}


void SetupDescriptorSet(const std::vector<Object>& iObjects)
{
	descriptorPool = std::make_unique<DescriptorPoolVulkan>();

	descriptorPool->Create(deviceVulkan->device, PoolData(10, 10, 10, 400, 2, 100));

	auto shaderDescriptorLayoutPBR = shaderProgramPBR->GetShaderProgramLayout();

	for (int i = 0; i < 2; ++i)
	{
		renderingContextResources[i]->descriptorSetPBRShader.samplerSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, shaderDescriptorLayoutPBR[0], bindings)[0];
		renderingContextResources[i]->descriptorSetPBRShader.perFrameUniformBufferSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, shaderDescriptorLayoutPBR[1], uniformBinding)[0];
	}

	std::array<vk::WriteDescriptorSet, 5> textureWrites = {};
	std::array<vk::WriteDescriptorSet, 1> uniformModelWrite = {};

	for (auto& e : iObjects)
	{
		
		ModelVulkan* tModel = dynamic_cast<ModelVulkan*>(e.model);
		ObjectRenderingDataVulkan* tRenderingData = dynamic_cast<ObjectRenderingDataVulkan*>(e.renderingData);

		TextureVulkan* albedoTexture = dynamic_cast<TextureVulkan*>(e.material.diffuseTexture);
		TextureVulkan* specularTexture = dynamic_cast<TextureVulkan*>(e.material.specularTexture);
		TextureVulkan* normalTexture = dynamic_cast<TextureVulkan*>(e.material.normalTexture);
		TextureVulkan* roughnessTexture = dynamic_cast<TextureVulkan*>(e.material.roughnessTexture);
		TextureVulkan* aoTexture = dynamic_cast<TextureVulkan*>(e.material.aoTexture);

		tModel->textureSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, shaderDescriptorLayoutPBR[2], textureBinding)[0];
		tRenderingData->positionBufferSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, shaderDescriptorLayoutPBR[3], uniformBinding)[0];


		vk::DescriptorImageInfo albedoImageInfo = {};
		albedoImageInfo.imageView = albedoTexture->data.view;

		textureWrites[0] = {};
		textureWrites[0].pNext = NULL;
		textureWrites[0].dstSet = tModel->textureSet;
		textureWrites[0].descriptorCount = 1;
		textureWrites[0].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[0].pImageInfo = &albedoImageInfo;
		textureWrites[0].dstArrayElement = 0;
		textureWrites[0].dstBinding = 0;

		vk::DescriptorImageInfo specularImageInfo = {};
		specularImageInfo.imageView = specularTexture->data.view;

		textureWrites[1] = {};
		textureWrites[1].pNext = NULL;
		textureWrites[1].dstSet = tModel->textureSet;
		textureWrites[1].descriptorCount = 1;
		textureWrites[1].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[1].pImageInfo = &specularImageInfo;
		textureWrites[1].dstArrayElement = 0;
		textureWrites[1].dstBinding = 1;

		vk::DescriptorImageInfo normalmapImageInfo = {};
		normalmapImageInfo.imageView = normalTexture->data.view;

		textureWrites[2] = {};
		textureWrites[2].pNext = NULL;
		textureWrites[2].dstSet = tModel->textureSet;
		textureWrites[2].descriptorCount = 1;
		textureWrites[2].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[2].pImageInfo = &normalmapImageInfo;
		textureWrites[2].dstArrayElement = 0;
		textureWrites[2].dstBinding = 2;

		vk::DescriptorImageInfo roughnessInfo = {};
		roughnessInfo.imageView = roughnessTexture->data.view;

		textureWrites[3] = {};
		textureWrites[3].pNext = NULL;
		textureWrites[3].dstSet = tModel->textureSet;
		textureWrites[3].descriptorCount = 1;
		textureWrites[3].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[3].pImageInfo = &roughnessInfo;
		textureWrites[3].dstArrayElement = 0;
		textureWrites[3].dstBinding = 3;

		vk::DescriptorImageInfo AOInfo = {};
		AOInfo.imageView = aoTexture->data.view;

		textureWrites[4] = {};
		textureWrites[4].pNext = NULL;
		textureWrites[4].dstSet = tModel->textureSet;
		textureWrites[4].descriptorCount = 1;
		textureWrites[4].descriptorType = vk::DescriptorType::eSampledImage;
		textureWrites[4].pImageInfo = &AOInfo;
		textureWrites[4].dstArrayElement = 0;
		textureWrites[4].dstBinding = 4;

		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(textureWrites.size()), textureWrites.data(), 0, NULL);


		uniformModelWrite[0] = {};
		uniformModelWrite[0].pNext = NULL;
		uniformModelWrite[0].dstSet = tRenderingData->positionBufferSet;
		uniformModelWrite[0].descriptorCount = 1;
		uniformModelWrite[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniformModelWrite[0].pBufferInfo = &tRenderingData->positionUniformBuffer.descriptorInfo;
		uniformModelWrite[0].dstArrayElement = 0;
		uniformModelWrite[0].dstBinding = 0;

		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(uniformModelWrite.size()), uniformModelWrite.data(), 0, NULL);

	}

	for (int i = 0; i < renderingContextResources.size(); ++i)
	{
		std::array<vk::WriteDescriptorSet, 1> writes = {};


		// Create image info for the image descriptor
		vk::DescriptorImageInfo pureSamplerInfo = {};

		pureSamplerInfo.imageView = vk::ImageView(nullptr);
		pureSamplerInfo.sampler = testSampler;

		writes[0] = {};
		writes[0].pNext = NULL;
		writes[0].dstSet = renderingContextResources[i]->descriptorSetPBRShader.samplerSet;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = vk::DescriptorType::eSampler;
		writes[0].pImageInfo = &pureSamplerInfo;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = 0;


		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, NULL);

		std::array<vk::WriteDescriptorSet, 2> uniform_writes = {};

		uniform_writes[0] = {};
		uniform_writes[0].pNext = NULL;
		uniform_writes[0].dstSet = renderingContextResources[i]->descriptorSetPBRShader.perFrameUniformBufferSet;
		uniform_writes[0].descriptorCount = 1;
		uniform_writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniform_writes[0].pBufferInfo = &renderingContextResources[i]->uniformBufferMVP.descriptorInfo;
		uniform_writes[0].dstArrayElement = 0;
		uniform_writes[0].dstBinding = 0;

		uniform_writes[1] = {};
		uniform_writes[1].pNext = NULL;
		uniform_writes[1].dstSet = renderingContextResources[i]->descriptorSetPBRShader.perFrameUniformBufferSet;
		uniform_writes[1].descriptorCount = 1;
		uniform_writes[1].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniform_writes[1].pBufferInfo = &renderingContextResources[i]->uniformBufferLights.descriptorInfo;
		uniform_writes[1].dstArrayElement = 0;
		uniform_writes[1].dstBinding = 1;


		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(uniform_writes.size()), uniform_writes.data(), 0, NULL);
	}
}

void SetupRenderPass()
{
	vk::AttachmentDescription attachments[2] = {};
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

	
	renderPassPostProc = deviceVulkan->device.createRenderPass(rp_info);
}

void SetupScenePassData()
{
	framebufferRenderScene = std::make_unique<FramebufferVulkan>(screenWidth, screenHeight);
	AttachmentCreateInfo attchCinfo;
	attchCinfo.format = Format::eR8G8B8A8Unorm;
	attchCinfo.height = screenHeight;
	attchCinfo.width = screenWidth;
	attchCinfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
	attchCinfo.layerCount = 1;

	framebufferRenderScene = std::make_unique<FramebufferVulkan>(screenWidth, screenHeight);
	AttachmentCreateInfo attchCinfoDepth;
	attchCinfoDepth.format = Format::eD24UnormS8Uint;
	attchCinfoDepth.height = screenHeight;
	attchCinfoDepth.width = screenWidth;
	attchCinfoDepth.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	attchCinfoDepth.layerCount = 1;

	framebufferRenderScene->AddAttachment(deviceVulkan->device, attchCinfo, allocator);
	framebufferRenderScene->AddAttachment(deviceVulkan->device, attchCinfoDepth, allocator);
	framebufferRenderScene->CreateRenderpass(deviceVulkan->device);


}

void SetupPostProcPass()
{
	vk::AttachmentDescription attachments[2] = {};
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


	renderPassPostProc = deviceVulkan->device.createRenderPass(rp_info);
}



void SetupShaders()
{
	// Create shader program, load the shaders and then the layout and pipeline will be setup

	// 
	shaderProgramPBR = std::make_unique<ShaderProgramVulkan>();
	
	ShaderDataVulkan vertexDataPBR;
	ShaderDataVulkan fragmentDataPBR;

	vertexDataPBR.entryPointName = "main";
	vertexDataPBR.shaderStage = ShaderStageFlagBits::eVertex;
	vertexDataPBR.shaderFile = "Shaders/Bin/basicVertexShader.spv";

	fragmentDataPBR.entryPointName = "main";
	fragmentDataPBR.shaderStage = ShaderStageFlagBits::eFragment;
	fragmentDataPBR.shaderFile = "Shaders/Bin/basicFragmentShader.spv";

	std::vector<ShaderDataVulkan> shaderDataPBR = { vertexDataPBR, fragmentDataPBR };

	shaderProgramPBR->LoadShaders(deviceVulkan->device, shaderDataPBR);


	// Load second shader
	shaderProgramRed = std::make_unique<ShaderProgramVulkan>();

	ShaderDataVulkan vertexDataRed;
	ShaderDataVulkan fragmentDataRed;

	vertexDataRed.entryPointName = "main";
	vertexDataRed.shaderStage = ShaderStageFlagBits::eVertex;
	vertexDataRed.shaderFile = "Shaders/Bin/redVertexShader.spv";

	fragmentDataRed.entryPointName = "main";
	fragmentDataRed.shaderStage = ShaderStageFlagBits::eFragment;
	fragmentDataRed.shaderFile = "Shaders/Bin/redFragmentShader.spv";

	std::vector<ShaderDataVulkan> shaderDataRed = { vertexDataRed, fragmentDataRed };

	shaderProgramRed->LoadShaders(deviceVulkan->device, shaderDataRed);


	shaderProgramPostProc = std::make_unique<ShaderProgramVulkan>();

	ShaderDataVulkan vertexDataPostProc;
	ShaderDataVulkan fragmentDataPostProc;

	vertexDataPostProc.entryPointName = "main";
	vertexDataPostProc.shaderStage = ShaderStageFlagBits::eVertex;
	vertexDataPostProc.shaderFile = "Shaders/Bin/postProcVertexShader.spv";

	fragmentDataPostProc.entryPointName = "main";
	fragmentDataPostProc.shaderStage = ShaderStageFlagBits::eFragment;
	fragmentDataPostProc.shaderFile = "Shaders/Bin/postProcFragmentShader.spv";

	std::vector<ShaderDataVulkan> shaderDataPostProc = { vertexDataPostProc, fragmentDataPostProc };

	shaderProgramPostProc->LoadShaders(deviceVulkan->device, shaderDataPostProc);

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


	std::vector<vk::ImageView> attachments;

	attachments.push_back(vk::ImageView(nullptr));
	attachments.push_back(depthBuffer.view);

	for (int i = 0; i < deviceVulkan->swapchain.images.size(); ++i)
	{
		attachments[0] = deviceVulkan->swapchain.views[i];
		framebuffers.push_back(CreateFrameBuffer(deviceVulkan->device, attachments, screenWidth, screenHeight, renderPassPostProc));
	}
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


	// Create staging buffer

	CopyBufferMemory(stagingT.buffer, oVertexBuffer.buffer, oVertexBuffer.allocation->GetSize());

	vmaDestroyBuffer(allocator, stagingT.buffer, stagingT.allocation);
	
}

#include "PipelineCreationDump.h"

void SetupPipeline()
{
	auto shaderLayoutPBR = shaderProgramPBR->GetShaderProgramLayout();

	vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setPNext(NULL)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(NULL)
		.setSetLayoutCount(NUM_DESCRIPTOR_SETS)
		.setPSetLayouts(shaderLayoutPBR.data());

	pipelineLayout = deviceVulkan->device.createPipelineLayout(pPipelineLayoutCreateInfo);

	std::vector<vk::VertexInputAttributeDescription> inputAttributes;
	vk::VertexInputBindingDescription inputDescription;


	inputDescription.binding = 0;
	inputDescription.inputRate = vk::VertexInputRate::eVertex;
	inputDescription.stride = sizeof(VertexData);

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


	inputAttributes.push_back(att1);
	inputAttributes.push_back(att2);
	inputAttributes.push_back(att3);
	inputAttributes.push_back(att4);
	inputAttributes.push_back(att5);

	vk::PipelineVertexInputStateCreateInfo vi = vk::PipelineVertexInputStateCreateInfo()
		.setFlags(PipelineVertexInputStateCreateFlagBits(0))
		.setPVertexBindingDescriptions(&inputDescription)
		.setPVertexAttributeDescriptions(inputAttributes.data())
		.setVertexAttributeDescriptionCount(5)
		.setVertexBindingDescriptionCount(1);

	vk::PipelineInputAssemblyStateCreateInfo ia = vk::PipelineInputAssemblyStateCreateInfo()
		.setPrimitiveRestartEnable(VK_FALSE)
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	vk::PipelineRasterizationStateCreateInfo rs = CreateStandardRasterizerState();


	vk::PipelineColorBlendAttachmentState att_state[1] = {};
	att_state[0].colorWriteMask = vk::ColorComponentFlagBits(0xF);
	att_state[0].blendEnable = VK_FALSE;
	att_state[0].alphaBlendOp = vk::BlendOp::eAdd;
	att_state[0].colorBlendOp = vk::BlendOp::eAdd;
	att_state[0].srcColorBlendFactor = vk::BlendFactor::eZero;
	att_state[0].dstColorBlendFactor = vk::BlendFactor::eZero;
	att_state[0].srcAlphaBlendFactor = vk::BlendFactor::eZero;
	att_state[0].dstAlphaBlendFactor = vk::BlendFactor::eZero;

	vk::PipelineColorBlendStateCreateInfo cb = {};
	cb.attachmentCount = 1;
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


	std::vector<vk::DynamicState> dynamicStateEnables;
	dynamicStateEnables.resize(VK_DYNAMIC_STATE_RANGE_SIZE); //[VK_DYNAMIC_STATE_RANGE_SIZE];

	vk::PipelineDynamicStateCreateInfo dynamicState = PipelineDynamicStateCreateInfo()
		.setPDynamicStates(dynamicStateEnables.data())
		.setDynamicStateCount(1);

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

	vk::PipelineMultisampleStateCreateInfo ms = PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(NUM_SAMPLES)
		.setSampleShadingEnable(VK_FALSE)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE)
		.setMinSampleShading(0.0f)
		.setPSampleMask(VK_NULL_HANDLE);

	std::vector<vk::PipelineShaderStageCreateInfo> shaderPipelineInfo = shaderProgramPBR->GetPipelineShaderInfo();

	// Create graphics pipeline for the first shader
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
		.setPStages(shaderPipelineInfo.data())
		.setStageCount(shaderPipelineInfo.size())
		.setRenderPass(renderPassPostProc)
		.setSubpass(0);

	pipelinePBR = deviceVulkan->device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipe);


	// Create graphics pipeline for the second shader
	std::vector<vk::PipelineShaderStageCreateInfo> shaderPipelineInfoRed = shaderProgramRed->GetPipelineShaderInfo();

	vk::GraphicsPipelineCreateInfo gfxPipe2 = GraphicsPipelineCreateInfo()
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
		.setPStages(shaderPipelineInfoRed.data())
		.setStageCount(shaderPipelineInfoRed.size())
		.setRenderPass(renderPassPostProc)
		.setSubpass(0);


	pipelineRed = deviceVulkan->device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipe2);


	/* CREATE PIPELINE FOR */


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

	cmd_begin.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
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
	iBuffer.bindPipeline(PipelineBindPoint::eGraphics, pipelinePBR);

	
	InitViewports(iBuffer);
	InitScissors(iBuffer);


	std::vector<DescriptorSet> totalSet;
	// Add our descriptor data to this set, and use this set
	totalSet.resize(4);
	for (int j = 0; j < (iObjects.size() - 1); ++j)
	{
		ModelVulkan* model = dynamic_cast<ModelVulkan*>(iObjects[j].model);
		ObjectRenderingDataVulkan* renderingData = dynamic_cast<ObjectRenderingDataVulkan*>(iObjects[j].renderingData);

		renderingContextResources[index]->descriptorSetPBRShader.textureSet = model->textureSet;
		renderingContextResources[index]->descriptorSetPBRShader.perObjectUniformBufferSet = renderingData->positionBufferSet;


		totalSet[0] = (renderingContextResources[index]->descriptorSetPBRShader.samplerSet);
		totalSet[1] = (renderingContextResources[index]->descriptorSetPBRShader.perFrameUniformBufferSet);
		totalSet[2] = (renderingContextResources[index]->descriptorSetPBRShader.textureSet);
		totalSet[3] = (renderingContextResources[index]->descriptorSetPBRShader.perObjectUniformBufferSet);



		iBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, totalSet.size(), totalSet.data(), 0, NULL);
		iBuffer.bindVertexBuffers(0, 1, &model->vertexBuffer.buffer, offsets);
		iBuffer.bindIndexBuffer(model->indexBuffer.buffer, 0, IndexType::eUint32);
		iBuffer.drawIndexed(model->GetIndiceCount(), 1, 0, 0, 0);
	}

	ModelVulkan* model = dynamic_cast<ModelVulkan*>(iObjects[iObjects.size() - 1].model);
	ObjectRenderingDataVulkan* renderingData = dynamic_cast<ObjectRenderingDataVulkan*>(iObjects[iObjects.size() - 1].renderingData);

	iBuffer.bindPipeline(PipelineBindPoint::eGraphics, pipelineRed);
	InitViewports(iBuffer);
	InitScissors(iBuffer);

	renderingContextResources[index]->descriptorSetPBRShader.textureSet = model->textureSet;
	renderingContextResources[index]->descriptorSetPBRShader.perObjectUniformBufferSet = renderingData->positionBufferSet;


	totalSet[0] = (renderingContextResources[index]->descriptorSetPBRShader.samplerSet);
	totalSet[1] = (renderingContextResources[index]->descriptorSetPBRShader.perFrameUniformBufferSet);
	totalSet[2] = (renderingContextResources[index]->descriptorSetPBRShader.textureSet);
	totalSet[3] = (renderingContextResources[index]->descriptorSetPBRShader.perObjectUniformBufferSet);

	iBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, totalSet.size(), totalSet.data(), 0, NULL);
	iBuffer.bindVertexBuffers(0, 1, &model->vertexBuffer.buffer, offsets);
	iBuffer.bindIndexBuffer(model->indexBuffer.buffer, 0, IndexType::eUint32);
	iBuffer.drawIndexed(model->GetIndiceCount(), 1, 0, 0, 0);

	iBuffer.endRenderPass();

	iBuffer.end();

}
void SetupCommandBuffersImgui(const vk::CommandBuffer& iBuffer, uint32_t index)
{

	vk::ClearValue clear_values[1] = {};
	clear_values[0].color.float32[0] = 0.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 0.0f;

	vk::CommandBufferBeginInfo begin_info = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	iBuffer.begin(begin_info);
	vk::RenderPassBeginInfo rp_begin = vk::RenderPassBeginInfo()
		.setRenderPass(imguiDataObj.renderpass)
		.setFramebuffer(framebuffersImgui[index])
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(screenWidth, screenHeight)))
		.setClearValueCount(3)
		.setPClearValues(clear_values);

	iBuffer.beginRenderPass(&rp_begin, SubpassContents::eInline);

	ImGui_ImplGlfwVulkan_Render(iBuffer);
	iBuffer.endRenderPass();

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
		SetupCommandBuffers(renderingContextResources[currentBuffer]->commandBuffer, currentBuffer, iObjects);
		SetupCommandBuffersImgui(renderingContextResources[currentBuffer]->commandBufferIMGUI, currentBuffer);
		firstFrame = false;
	}

	else
	{
		//std::thread CommandSetup = std::thread(SetupCommandBuffers, commandBuffers[(currentBuffer + 1) % 2], (currentBuffer + 1) % 2);
		int32_t nextBuffer = (currentBuffer + 1) % 2;
		SetupCommandBuffers(renderingContextResources[nextBuffer]->commandBuffer, nextBuffer, iObjects);
		SetupCommandBuffersImgui(renderingContextResources[nextBuffer]->commandBufferIMGUI, nextBuffer);

		deviceVulkan->device.waitForFences(1, &graphicsQueueFinishedFence, vk::Bool32(true), FENCE_TIMEOUT);
		deviceVulkan->device.resetFences(graphicsQueueFinishedFence);
		//CommandSetup.join();
	}

	deviceVulkan->device.acquireNextImageKHR(deviceVulkan->swapchain.swapchain, UINT64_MAX, imageAcquiredSemaphore, vk::Fence(nullptr), &currentBuffer);
	vmaSetCurrentFrameIndex(allocator, currentBuffer);


	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::SubmitInfo submit_info[1] = {};
	vk::CommandBuffer commandBuffers[2] = { renderingContextResources[currentBuffer]->commandBuffer, renderingContextResources[currentBuffer]->commandBufferIMGUI };


	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 2;
	submit_info[0].pCommandBuffers = commandBuffers;
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
	UpdateUniformbufferFrame((currentBuffer + 1) % 2, iCamera, lights);
}

void SetupTexturesForObject(Material& material, std::vector<BufferVulkan>& iStagingBuffers, vk::CommandBuffer iBuffer);

void SetupQuerypool(const vk::Device& iDevice)
{
	for (auto& e : renderingContextResources)
	{
		vk::QueryPoolCreateInfo poolCI = vk::QueryPoolCreateInfo()
			.setQueryType(QueryType::eTimestamp)
			.setQueryCount(2);

		e->queryPool = iDevice.createQueryPool(poolCI);
	}
}

void RendererVulkan::Create(std::vector<Object>& iObjects, 
	ResourceManager* const iResourceManager,
	iLowLevelWindow* const iIlowLevelWindow)
{

	SetupApplication(iIlowLevelWindow);

	SetupDevice();

	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		auto contextResources = std::make_unique<RenderingContextResources>();
		renderingContextResources.push_back(std::move(contextResources));
	}

	SetupSwapchain();
	deviceVulkan->SetupDeviceQueue();
	SetupCommandBuffer();

	SetupIMGUI(iIlowLevelWindow);


	SetupUniformbuffer();
	SetupShaders();

	SetupDepthbuffer();
	SetupRTTexture(deviceVulkan->device, screenWidth, screenHeight, postProcBuffer.image, postProcBuffer.allocation, postProcBuffer.view);
	postProcBuffer.format = Format::eR8G8B8A8Unorm;

	SetupRenderPass();
	SetupScenePassData();

	SetupFramebuffers();
	SetupQuerypool(deviceVulkan->device);
	
	
	vk::CommandBuffer cmdBufferTextures = BeginSingleTimeCommands(deviceVulkan->device, cmdPool->GetPool());
	TransitionImageLayout(cmdBufferTextures, depthBuffer.image, depthBuffer.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	// Staging buffers we will need to delete
	std::vector<BufferVulkan> stagingBuffers;

	// load in albedo textures in this loop
	for (int i = 0; i < iObjects.size(); ++i)
	{

		if (!iObjects[i].model->isPrepared)
		{
			auto e = dynamic_cast<ModelVulkan*>(iObjects[i].model);


			SetupVertexBuffer(e->vertexBuffer, e->data);
			SetupIndexBuffer(e->indexBuffer, e->data);


			e->indiceCount = e->data.indices.size();


			e->isPrepared = true;
			models.push_back(e);
		}

		SetupTexturesForObject(iObjects[i].material, stagingBuffers, cmdBufferTextures);

		
		if (!iObjects[i].renderingData->isPrepared)
		{
			auto e = dynamic_cast<ObjectRenderingDataVulkan*>(iObjects[i].renderingData);

			UniformBufferVulkan tUniformBuff;

			CreateSimpleBuffer(allocator,
				tUniformBuff.allocation,
				VMA_MEMORY_USAGE_CPU_TO_GPU,
				tUniformBuff.buffer,
				vk::BufferUsageFlagBits::eUniformBuffer,
				sizeof(CBMatrix));

			singleModelmatrixData.model = iObjects[i].modelMatrix;
			CopyDataToBuffer(VkDevice(deviceVulkan->device), tUniformBuff.allocation, (void*)&singleModelmatrixData, sizeof(singleModelmatrixData));


			tUniformBuff.descriptorInfo.buffer = tUniformBuff.buffer;
			tUniformBuff.descriptorInfo.offset = 0;
			tUniformBuff.descriptorInfo.range = sizeof(CBModelMatrixSingle);

			e->positionUniformBuffer = tUniformBuff;
			e->isPrepared = true;
			objRenderingData.push_back(e);

		}
	}
	
	EndSingleTimeCommands(deviceVulkan->device, cmdBufferTextures, cmdPool->GetPool(), deviceVulkan->graphicsQueue);

	for (auto& e : stagingBuffers)
	{
		vmaDestroyBuffer(allocator, e.buffer, e.allocation);
	}
	stagingBuffers.clear();
	stagingBuffers.resize(1);

	SetupPipeline();
	SetupSemaphores();

	CreateTextureSampler(deviceVulkan->device);

	SetupDescriptorSet(iObjects);
	//iObjects.clear();
	//iObjects.resize(1);
}

void RendererVulkan::Destroy()
{
	deviceVulkan->presentQueue.waitIdle();
	
	// free our dynamic uniform buffer thing
	framebufferRenderScene->Destroy(allocator, deviceVulkan->device);

	for (auto& e : renderingContextResources)
	{
		e->commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		e->commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	}

	descriptorPool->Destroy(deviceVulkan->device);

	deviceVulkan->device.destroySemaphore(rendererFinishedSemaphore);
	deviceVulkan->device.destroySemaphore(imageAcquiredSemaphore);

	deviceVulkan->device.destroySampler(testSampler);
	deviceVulkan->device.destroySampler(testImageSampler);


	deviceVulkan->device.destroyFence(graphicsQueueFinishedFence);

	shaderProgramPBR->Destroy(deviceVulkan->device);
	shaderProgramRed->Destroy(deviceVulkan->device);

	cmdPool->Destroy(deviceVulkan->device);

	vmaDestroyImage(allocator, (VkImage)depthBuffer.image, depthBuffer.allocation);
	deviceVulkan->device.destroyImageView(depthBuffer.view);


	for (auto& e : framebuffers)
	{
		deviceVulkan->device.destroyFramebuffer(e);
	}

	deviceVulkan->device.destroyPipeline(pipelinePBR);
	deviceVulkan->device.destroyPipeline(pipelineRed);
	deviceVulkan->device.destroyPipelineLayout(pipelineLayout);
	deviceVulkan->device.destroyRenderPass(renderPassPostProc);


	for (auto& e : renderingContextResources)
	{
		vmaDestroyBuffer(allocator, (VkBuffer)(*e).uniformBufferLights.buffer, (*e).uniformBufferLights.allocation);
		vmaDestroyBuffer(allocator, (VkBuffer)(*e).uniformBufferModelMatrix.buffer, (*e).uniformBufferModelMatrix.allocation);
		vmaDestroyBuffer(allocator, (VkBuffer)(*e).uniformBufferMVP.buffer, (*e).uniformBufferMVP.allocation);

		deviceVulkan->device.destroyQueryPool(e->queryPool);
	}

	for (auto& e : models)
	{
		vmaDestroyBuffer(allocator, e->vertexBuffer.buffer, e->vertexBuffer.allocation);
		vmaDestroyBuffer(allocator, e->indexBuffer.buffer, e->indexBuffer.allocation);
	}

	for (auto& e : objRenderingData)
	{
		vmaDestroyBuffer(allocator, e->positionUniformBuffer.buffer, e->positionUniformBuffer.allocation);
	}

	for (auto& e : textures)
	{
		vmaDestroyImage(allocator, e->data.image, e->data.allocation);
		deviceVulkan->device.destroyImageView(e->data.view);
	}

	for (auto& e : deviceVulkan->swapchain.views)
	{
		deviceVulkan->device.destroyImageView(e);
	}

	for (auto& e : deviceVulkan->swapchain.images)
	{
		//deviceVulkan->device.destroyImage(e);
	}


	vmaDestroyImage(allocator, postProcBuffer.image, postProcBuffer.allocation);
	deviceVulkan->device.destroyImageView(postProcBuffer.view);
	instance.destroySurfaceKHR(deviceVulkan->swapchain.surface);
	
	deviceVulkan->device.destroySwapchainKHR(deviceVulkan->swapchain.swapchain);


	//device.destroyImage(depthImage);
	//device.destroyImage(testTexture.image);

	// Clean up.
	vmaDestroyAllocator(allocator);
	deviceVulkan->device.waitIdle();
	deviceVulkan->device.destroy();
	instance.destroy();
}


void SetupTexturesForObject(Material& material, std::vector<BufferVulkan>& iStagingBuffers, vk::CommandBuffer iBuffer)
{

	// if texture is already prepared, then just ignore it.
	if(!material.diffuseTexture->isPrepared)
	{
		TextureVulkan* diffuseTexture = dynamic_cast<TextureVulkan*>(material.diffuseTexture);

		SetupTextureImage(iBuffer, deviceVulkan->device, diffuseTexture->GetFilepath(), diffuseTexture->data.image, diffuseTexture->data.allocation, iStagingBuffers);
		diffuseTexture->data.view = CreateImageView(deviceVulkan->device, diffuseTexture->data.image, Format::eR8G8B8A8Unorm);
		diffuseTexture->isPrepared = true;
		textures.push_back(diffuseTexture);
	}

	if (!material.specularTexture->isPrepared)
	{
		TextureVulkan* specularTexture = dynamic_cast<TextureVulkan*>(material.specularTexture);

		SetupTextureImage(iBuffer, deviceVulkan->device, specularTexture->GetFilepath(), specularTexture->data.image, specularTexture->data.allocation, iStagingBuffers);
		specularTexture->data.view = CreateImageView(deviceVulkan->device, specularTexture->data.image, Format::eR8G8B8A8Unorm);
		specularTexture->isPrepared = true;
		textures.push_back(specularTexture);
	}

	if (!material.roughnessTexture->isPrepared)
	{
		TextureVulkan* roughnessTexture = dynamic_cast<TextureVulkan*>(material.roughnessTexture);

		SetupTextureImage(iBuffer, deviceVulkan->device, roughnessTexture->GetFilepath(), roughnessTexture->data.image, roughnessTexture->data.allocation, iStagingBuffers);
		roughnessTexture->data.view = CreateImageView(deviceVulkan->device, roughnessTexture->data.image, Format::eR8G8B8A8Unorm);
		roughnessTexture->isPrepared = true;
		textures.push_back(roughnessTexture);
	}

	if (!material.aoTexture->isPrepared)
	{
		TextureVulkan* aoTexture = dynamic_cast<TextureVulkan*>(material.aoTexture);

		SetupTextureImage(iBuffer, deviceVulkan->device, aoTexture->GetFilepath(), aoTexture->data.image, aoTexture->data.allocation, iStagingBuffers);
		aoTexture->data.view = CreateImageView(deviceVulkan->device, aoTexture->data.image, Format::eR8G8B8A8Unorm);
		aoTexture->isPrepared = true;
		textures.push_back(aoTexture);
	}

	if (!material.normalTexture->isPrepared)
	{
		TextureVulkan* normalTexture = dynamic_cast<TextureVulkan*>(material.normalTexture);

		SetupTextureImage(iBuffer, deviceVulkan->device, normalTexture->GetFilepath(), normalTexture->data.image, normalTexture->data.allocation, iStagingBuffers);
		normalTexture->data.view = CreateImageView(deviceVulkan->device, normalTexture->data.image, Format::eR8G8B8A8Unorm);
		normalTexture->isPrepared = true;
		textures.push_back(normalTexture);
	}
	
}