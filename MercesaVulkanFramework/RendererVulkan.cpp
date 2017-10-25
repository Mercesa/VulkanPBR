//#include "RendererVulkan.h"
//
//
//#include <stdio.h>
//#include <stdlib.h>
//// Enable the WSI extensions
//#if defined(__ANDROID__)
//#define VK_USE_PLATFORM_ANDROID_KHR
//#elif defined(__linux__)
//#define VK_USE_PLATFORM_XLIB_KHR
//#elif defined(_WIN32)
//#define VK_USE_PLATFORM_WIN32_KHR
//#endif
//
//#define NOMINMAX
////#define WIN32_MEAN_AND_LEAN
//
//#define VMA_IMPLEMENTATION
//#include "vk_mem_alloc.h"
//
//#include "RenderingIncludes.h"
//
//#include <iostream>
//#include <math.h>
//#include <algorithm>
//#include <utility>
//#include <future>
//#include <map>
//
//#include "VulkanDataObjects.h"
//#include "GraphicsStructures.h"
//#include "DescriptorPoolVulkan.h"
//
//#include "VulkanHelper.h"
//#include "Helper.h"
//#include "ModelLoader.h"
//
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
//
//#include "Camera.h"
//
//#include <ostream>
//#include <sstream>
//#include <string.h>
//#include <iterator>
//#include "ConstantBuffers.h"
//
//#include "DescriptorPoolVulkan.h"
//#include "CommandpoolVulkan.h"
//#include "DeviceVulkan.h"
//
//#include "easylogging++.h"
//#include "SDLLowLevelWindow.h"
//
//#include "libs/Spir-v cross/spirv_glsl.hpp"
//#include "DescriptorLayoutHelper.h"
//#include "NewCamera.h"
//#include "Helper.h"
//#include "Game.h"
//#include "FramebufferVulkan.h"
//#include "ShaderProgramVulkan.h"
//#include "ModelVulkan.h"
//#include "TextureVulkan.h"
//#include "ObjectRenderingDataVulkan.h"
//#include "GLFWLowLevelWindow.h"
//#include "PipelineCreationDump.h"
//
//#include "RenderScenePass.h"
//#include <imgui.h>
//
//#include <imgui_impl_glfw_vulkan.h>
//
//
//
//
//
//
//#define NUM_SAMPLES vk::SampleCountFlagBits::e1
//#define NUM_DESCRIPTOR_SETS 4
//#define FENCE_TIMEOUT 100000000
//#define NUM_FRAMES 2
//
//vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, iLowLevelWindow* window);
//
//static const int screenWidth = 1280;
//static const int screenHeight = 720;
//
//using namespace vk;
//
//
//struct ShaderResourcesPBR
//{
//	vk::DescriptorSet samplerSet;
//	vk::DescriptorSet perFrameUniformBufferSet;
//	vk::DescriptorSet perObjectUniformBufferSet;
//	vk::DescriptorSet textureSet;
//};
//
//struct ShaderResourcesPostProc
//{
//	vk::DescriptorSet inputAttachmentSet;
//} shaderResourcesPostProc;
//
//struct RenderingContextResources
//{
//	ShaderResourcesPBR descriptorSetPBRShader;
//
//	UniformBufferVulkan uniformBufferMVP;
//	UniformBufferVulkan uniformBufferModelMatrix;
//	UniformBufferVulkan uniformBufferLights;
//
//	vk::QueryPool queryPool;
//	vk::CommandBuffer commandBuffer;
//	vk::CommandBuffer commandBufferIMGUI;
//};
//
//
//std::vector<vk::Framebuffer> framebuffers;
//std::vector<vk::Framebuffer> framebuffersImgui;
//
//std::unique_ptr<FramebufferVulkan> framebufferRenderScene;
//
//
//TextureData depthBuffer;
//
//
//std::vector<std::unique_ptr<RenderingContextResources>> renderingContextResources;
//
//
//// Device and instance
//vk::Instance instance;
//
//std::unique_ptr<CommandpoolVulkan> cmdPool;
//
//// Information about our device its memory and family properties
//
//std::unique_ptr<DescriptorPoolVulkan> descriptorPool;
//
//std::vector<ModelVulkan*> models;
//std::vector<TextureVulkan*> textures;
//std::vector<ObjectRenderingDataVulkan*> objRenderingData;
//
//VmaAllocator allocator;
//
//
//vk::Viewport viewPort;
//vk::Rect2D scissor;
//
//
//vk::Sampler testImageSampler;
//vk::Sampler testSampler;
//
//vk::Semaphore imageAcquiredSemaphore;
//vk::Semaphore rendererFinishedSemaphore;
//
//std::vector<vk::DescriptorSetLayoutBinding> bindings;
//std::vector<vk::DescriptorSetLayoutBinding> uniformBinding;
//std::vector<vk::DescriptorSetLayoutBinding> textureBinding;
//std::vector<vk::DescriptorSetLayoutBinding> postProcBinding;
//
//vk::Fence graphicsQueueFinishedFence;
//
//std::unique_ptr<DeviceVulkan> deviceVulkan;
//
//// All we need to render seperate shaders
//std::unique_ptr<ShaderProgramVulkan> shaderProgramPBR;
//std::unique_ptr<ShaderProgramVulkan> shaderProgramRed;
//vk::Pipeline pipelinePBR;
//vk::Pipeline pipelineRed;
//vk::PipelineLayout pipelineLayoutRenderScene;
//
//
//std::unique_ptr<ShaderProgramVulkan> shaderProgramPostProc;
//vk::PipelineLayout pipelineLayoutPostProc;
//vk::Pipeline pipelinePostProc;
//
//std::unique_ptr<RenderScenePass> renderPassScene;
//
//
//
//
//
//
//


//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//	
//	
//
//	CopyDataToBuffer(VkDevice(deviceVulkan->device), renderingContextResources[iCurrentBuff]->uniformBufferLights.allocation, (void*)&lightConstantBufferData, sizeof(lightConstantBufferData));
//}
//
//
//void SetupDescriptorSet(const std::vector<Object>& iObjects)
//{
//	descriptorPool = std::make_unique<DescriptorPoolVulkan>();
//
//	descriptorPool->Create(deviceVulkan->device, PoolData(10, 10, 10, 400, 2, 100));
//
//	auto shaderDescriptorLayoutPBR = shaderProgramPBR->GetShaderProgramLayout();
//
//	for (int i = 0; i < 2; ++i)
//	{
//		renderingContextResources[i]->descriptorSetPBRShader.samplerSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, shaderDescriptorLayoutPBR[0], bindings)[0];
//		renderingContextResources[i]->descriptorSetPBRShader.perFrameUniformBufferSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, shaderDescriptorLayoutPBR[1], uniformBinding)[0];
//	}
//
//	std::array<vk::WriteDescriptorSet, 5> textureWrites = {};
//	std::array<vk::WriteDescriptorSet, 1> uniformModelWrite = {};
//
//	for (auto& e : iObjects)
//	{
//		
//		ModelVulkan* tModel = dynamic_cast<ModelVulkan*>(e.model);
//		ObjectRenderingDataVulkan* tRenderingData = dynamic_cast<ObjectRenderingDataVulkan*>(e.renderingData);
//
//		TextureVulkan* albedoTexture = dynamic_cast<TextureVulkan*>(e.material.diffuseTexture);
//		TextureVulkan* specularTexture = dynamic_cast<TextureVulkan*>(e.material.specularTexture);
//		TextureVulkan* normalTexture = dynamic_cast<TextureVulkan*>(e.material.normalTexture);
//		TextureVulkan* roughnessTexture = dynamic_cast<TextureVulkan*>(e.material.roughnessTexture);
//		TextureVulkan* aoTexture = dynamic_cast<TextureVulkan*>(e.material.aoTexture);
//
//		tModel->textureSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, shaderDescriptorLayoutPBR[2], textureBinding)[0];
//		tRenderingData->positionBufferSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, shaderDescriptorLayoutPBR[3], uniformBinding)[0];
//
//
//		vk::DescriptorImageInfo albedoImageInfo = {};
//		albedoImageInfo.imageView = albedoTexture->data.view;
//
//		textureWrites[0] = {};
//		textureWrites[0].pNext = NULL;
//		textureWrites[0].dstSet = tModel->textureSet;
//		textureWrites[0].descriptorCount = 1;
//		textureWrites[0].descriptorType = vk::DescriptorType::eSampledImage;
//		textureWrites[0].pImageInfo = &albedoImageInfo;
//		textureWrites[0].dstArrayElement = 0;
//		textureWrites[0].dstBinding = 0;
//
//		vk::DescriptorImageInfo specularImageInfo = {};
//		specularImageInfo.imageView = specularTexture->data.view;
//
//		textureWrites[1] = {};
//		textureWrites[1].pNext = NULL;
//		textureWrites[1].dstSet = tModel->textureSet;
//		textureWrites[1].descriptorCount = 1;
//		textureWrites[1].descriptorType = vk::DescriptorType::eSampledImage;
//		textureWrites[1].pImageInfo = &specularImageInfo;
//		textureWrites[1].dstArrayElement = 0;
//		textureWrites[1].dstBinding = 1;
//
//		vk::DescriptorImageInfo normalmapImageInfo = {};
//		normalmapImageInfo.imageView = normalTexture->data.view;
//
//		textureWrites[2] = {};
//		textureWrites[2].pNext = NULL;
//		textureWrites[2].dstSet = tModel->textureSet;
//		textureWrites[2].descriptorCount = 1;
//		textureWrites[2].descriptorType = vk::DescriptorType::eSampledImage;
//		textureWrites[2].pImageInfo = &normalmapImageInfo;
//		textureWrites[2].dstArrayElement = 0;
//		textureWrites[2].dstBinding = 2;
//
//		vk::DescriptorImageInfo roughnessInfo = {};
//		roughnessInfo.imageView = roughnessTexture->data.view;
//
//		textureWrites[3] = {};
//		textureWrites[3].pNext = NULL;
//		textureWrites[3].dstSet = tModel->textureSet;
//		textureWrites[3].descriptorCount = 1;
//		textureWrites[3].descriptorType = vk::DescriptorType::eSampledImage;
//		textureWrites[3].pImageInfo = &roughnessInfo;
//		textureWrites[3].dstArrayElement = 0;
//		textureWrites[3].dstBinding = 3;
//
//		vk::DescriptorImageInfo AOInfo = {};
//		AOInfo.imageView = aoTexture->data.view;
//
//		textureWrites[4] = {};
//		textureWrites[4].pNext = NULL;
//		textureWrites[4].dstSet = tModel->textureSet;
//		textureWrites[4].descriptorCount = 1;
//		textureWrites[4].descriptorType = vk::DescriptorType::eSampledImage;
//		textureWrites[4].pImageInfo = &AOInfo;
//		textureWrites[4].dstArrayElement = 0;
//		textureWrites[4].dstBinding = 4;
//
//		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(textureWrites.size()), textureWrites.data(), 0, NULL);
//
//
//		uniformModelWrite[0] = {};
//		uniformModelWrite[0].pNext = NULL;
//		uniformModelWrite[0].dstSet = tRenderingData->positionBufferSet;
//		uniformModelWrite[0].descriptorCount = 1;
//		uniformModelWrite[0].descriptorType = vk::DescriptorType::eUniformBuffer;
//		uniformModelWrite[0].pBufferInfo = &tRenderingData->positionUniformBuffer.descriptorInfo;
//		uniformModelWrite[0].dstArrayElement = 0;
//		uniformModelWrite[0].dstBinding = 0;
//
//		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(uniformModelWrite.size()), uniformModelWrite.data(), 0, NULL);
//
//	}
//
//	for (int i = 0; i < renderingContextResources.size(); ++i)
//	{
//		std::array<vk::WriteDescriptorSet, 1> writes = {};
//
//
//		// Create image info for the image descriptor
//		vk::DescriptorImageInfo pureSamplerInfo = {};
//
//		pureSamplerInfo.imageView = vk::ImageView(nullptr);
//		pureSamplerInfo.sampler = testSampler;
//
//		writes[0] = {};
//		writes[0].pNext = NULL;
//		writes[0].dstSet = renderingContextResources[i]->descriptorSetPBRShader.samplerSet;
//		writes[0].descriptorCount = 1;
//		writes[0].descriptorType = vk::DescriptorType::eSampler;
//		writes[0].pImageInfo = &pureSamplerInfo;
//		writes[0].dstArrayElement = 0;
//		writes[0].dstBinding = 0;
//
//
//		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, NULL);
//
//		std::array<vk::WriteDescriptorSet, 2> uniform_writes = {};
//
//		uniform_writes[0] = {};
//		uniform_writes[0].pNext = NULL;
//		uniform_writes[0].dstSet = renderingContextResources[i]->descriptorSetPBRShader.perFrameUniformBufferSet;
//		uniform_writes[0].descriptorCount = 1;
//		uniform_writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
//		uniform_writes[0].pBufferInfo = &renderingContextResources[i]->uniformBufferMVP.descriptorInfo;
//		uniform_writes[0].dstArrayElement = 0;
//		uniform_writes[0].dstBinding = 0;
//
//		uniform_writes[1] = {};
//		uniform_writes[1].pNext = NULL;
//		uniform_writes[1].dstSet = renderingContextResources[i]->descriptorSetPBRShader.perFrameUniformBufferSet;
//		uniform_writes[1].descriptorCount = 1;
//		uniform_writes[1].descriptorType = vk::DescriptorType::eUniformBuffer;
//		uniform_writes[1].pBufferInfo = &renderingContextResources[i]->uniformBufferLights.descriptorInfo;
//		uniform_writes[1].dstArrayElement = 0;
//		uniform_writes[1].dstBinding = 1;
//
//
//		deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(uniform_writes.size()), uniform_writes.data(), 0, NULL);
//	}
//
//	auto shaderDescriptorLayoutPostProc = shaderProgramPostProc->GetShaderProgramLayout();
//
//	shaderResourcesPostProc.inputAttachmentSet = descriptorPool->AllocateDescriptorSet(deviceVulkan->device, 1, shaderDescriptorLayoutPostProc[0], postProcBinding)[0];
//
//	vk::DescriptorImageInfo inputView = {};
//	inputView.imageView = framebufferRenderScene->attachments[0].view;
//
//	std::array<vk::WriteDescriptorSet, 1> input_writes = {};
//
//	input_writes[0] = {};
//	input_writes[0].pNext = NULL;
//	input_writes[0].dstSet = shaderResourcesPostProc.inputAttachmentSet;
//	input_writes[0].descriptorCount = 1;
//	input_writes[0].descriptorType = vk::DescriptorType::eSampledImage;
//	input_writes[0].pImageInfo = &inputView;
//	input_writes[0].dstArrayElement = 0;
//	input_writes[0].dstBinding = 0;
//
//	deviceVulkan->device.updateDescriptorSets(static_cast<uint32_t>(input_writes.size()), input_writes.data(), 0, NULL);
//
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//void SetupCommandBuffers(const vk::CommandBuffer& iBuffer, uint32_t index, const std::vector<Object>& iObjects)
//{
//	vk::CommandBufferBeginInfo cmd_begin = vk::CommandBufferBeginInfo();
//
//	cmd_begin.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
//	iBuffer.begin(cmd_begin);
//
//	vk::ClearValue clear_values[2] = {};
//	clear_values[0].color.float32[0] = 0.2f;
//	clear_values[0].color.float32[1] = 0.2f;
//	clear_values[0].color.float32[2] = 0.2f;
//	clear_values[0].color.float32[3] = 0.2f;
//	clear_values[1].depthStencil.depth = 1.0f;
//	clear_values[1].depthStencil.stencil = 0;
//
//
//	const vk::DeviceSize offsets[1] = { 0 };
//
//	vk::RenderPassBeginInfo rp_begin = vk::RenderPassBeginInfo()
//		.setRenderPass(renderPassScene->renderpass)
//		.setFramebuffer(framebuffers[index])
//		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(screenWidth, screenHeight)))
//		.setClearValueCount(2)
//		.setPClearValues(clear_values);
//
//	iBuffer.beginRenderPass(&rp_begin, SubpassContents::eInline);
//	iBuffer.bindPipeline(PipelineBindPoint::eGraphics, pipelinePBR);
//
//	
//	InitViewports(iBuffer);
//	InitScissors(iBuffer);
//
//
//	std::vector<DescriptorSet> totalSet;
//	// Add our descriptor data to this set, and use this set
//	totalSet.resize(4);
//	for (int j = 0; j < (iObjects.size() - 1); ++j)
//	{
//		ModelVulkan* model = dynamic_cast<ModelVulkan*>(iObjects[j].model);
//		ObjectRenderingDataVulkan* renderingData = dynamic_cast<ObjectRenderingDataVulkan*>(iObjects[j].renderingData);
//
//		renderingContextResources[index]->descriptorSetPBRShader.textureSet = model->textureSet;
//		renderingContextResources[index]->descriptorSetPBRShader.perObjectUniformBufferSet = renderingData->positionBufferSet;
//
//
//		totalSet[0] = (renderingContextResources[index]->descriptorSetPBRShader.samplerSet);
//		totalSet[1] = (renderingContextResources[index]->descriptorSetPBRShader.perFrameUniformBufferSet);
//		totalSet[2] = (renderingContextResources[index]->descriptorSetPBRShader.textureSet);
//		totalSet[3] = (renderingContextResources[index]->descriptorSetPBRShader.perObjectUniformBufferSet);
//
//		iBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayoutRenderScene, 0, totalSet.size(), totalSet.data(), 0, NULL);
//		iBuffer.bindVertexBuffers(0, 1, &model->vertexBuffer.buffer, offsets);
//		iBuffer.bindIndexBuffer(model->indexBuffer.buffer, 0, IndexType::eUint32);
//		iBuffer.drawIndexed(model->GetIndiceCount(), 1, 0, 0, 0);
//	}
//
//	ModelVulkan* model = dynamic_cast<ModelVulkan*>(iObjects[iObjects.size() - 1].model);
//	ObjectRenderingDataVulkan* renderingData = dynamic_cast<ObjectRenderingDataVulkan*>(iObjects[iObjects.size() - 1].renderingData);
//
//	iBuffer.bindPipeline(PipelineBindPoint::eGraphics, pipelineRed);
//	InitViewports(iBuffer);
//	InitScissors(iBuffer);
//
//	renderingContextResources[index]->descriptorSetPBRShader.textureSet = model->textureSet;
//	renderingContextResources[index]->descriptorSetPBRShader.perObjectUniformBufferSet = renderingData->positionBufferSet;
//
//
//	totalSet[0] = (renderingContextResources[index]->descriptorSetPBRShader.samplerSet);
//	totalSet[1] = (renderingContextResources[index]->descriptorSetPBRShader.perFrameUniformBufferSet);
//	totalSet[2] = (renderingContextResources[index]->descriptorSetPBRShader.textureSet);
//	totalSet[3] = (renderingContextResources[index]->descriptorSetPBRShader.perObjectUniformBufferSet);
//
//	iBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayoutRenderScene, 0, totalSet.size(), totalSet.data(), 0, NULL);
//	iBuffer.bindVertexBuffers(0, 1, &model->vertexBuffer.buffer, offsets);
//	iBuffer.bindIndexBuffer(model->indexBuffer.buffer, 0, IndexType::eUint32);
//	iBuffer.drawIndexed(model->GetIndiceCount(), 1, 0, 0, 0);
//
//	iBuffer.endRenderPass();
//
//	std::vector<DescriptorSet> inputSet;
//	inputSet.resize(1);
//
//	inputSet[0] = shaderResourcesPostProc.inputAttachmentSet;
//
//
//	iBuffer.end();
//
//}
//void SetupCommandBuffersImgui(const vk::CommandBuffer& iBuffer, uint32_t index)
//{
//
//	vk::ClearValue clear_values[1] = {};
//	clear_values[0].color.float32[0] = 0.0f;
//	clear_values[0].color.float32[1] = 0.0f;
//	clear_values[0].color.float32[2] = 0.0f;
//	clear_values[0].color.float32[3] = 0.0f;
//
//	vk::CommandBufferBeginInfo begin_info = vk::CommandBufferBeginInfo()
//		.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
//
//	iBuffer.begin(begin_info);
//	vk::RenderPassBeginInfo rp_begin = vk::RenderPassBeginInfo()
//		.setRenderPass(imguiDataObj.renderpass)
//		.setFramebuffer(framebuffersImgui[index])
//		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(screenWidth, screenHeight)))
//		.setClearValueCount(3)
//		.setPClearValues(clear_values);
//
//	iBuffer.beginRenderPass(&rp_begin, SubpassContents::eInline);
//
//	ImGui_ImplGlfwVulkan_Render(iBuffer);
//	iBuffer.endRenderPass();
//
//	iBuffer.end();
//}
//
//
//bool firstFrame = true;
//void RendererVulkan::Render(const std::vector<Object>& iObjects)
//{
//	if (firstFrame)
//	{
//		SetupCommandBuffers(renderingContextResources[currentBuffer]->commandBuffer, currentBuffer, iObjects);
//		SetupCommandBuffersImgui(renderingContextResources[currentBuffer]->commandBufferIMGUI, currentBuffer);
//		firstFrame = false;
//	}
//
//	else
//	{
//		//std::thread CommandSetup = std::thread(SetupCommandBuffers, commandBuffers[(currentBuffer + 1) % 2], (currentBuffer + 1) % 2);
//		int32_t nextBuffer = (currentBuffer + 1) % 2;
//		SetupCommandBuffers(renderingContextResources[nextBuffer]->commandBuffer, nextBuffer, iObjects);
//		SetupCommandBuffersImgui(renderingContextResources[nextBuffer]->commandBufferIMGUI, nextBuffer);
//
//		deviceVulkan->device.waitForFences(1, &graphicsQueueFinishedFence, vk::Bool32(true), FENCE_TIMEOUT);
//		deviceVulkan->device.resetFences(graphicsQueueFinishedFence);
//		//CommandSetup.join();
//	}
//
//	deviceVulkan->device.acquireNextImageKHR(deviceVulkan->swapchain.swapchain, UINT64_MAX, imageAcquiredSemaphore, vk::Fence(nullptr), &currentBuffer);
//	vmaSetCurrentFrameIndex(allocator, currentBuffer);
//
//
//	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
//
//	vk::SubmitInfo submit_info[1] = {};
//	vk::CommandBuffer commandBuffers[2] = { renderingContextResources[currentBuffer]->commandBuffer, renderingContextResources[currentBuffer]->commandBufferIMGUI };
//
//
//	submit_info[0].waitSemaphoreCount = 1;
//	submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
//	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
//	submit_info[0].commandBufferCount = 2;
//	submit_info[0].pCommandBuffers = commandBuffers;
//	submit_info[0].signalSemaphoreCount = 1;
//	submit_info[0].pSignalSemaphores = &rendererFinishedSemaphore;
//
//	deviceVulkan->graphicsQueue.submit(1, submit_info, graphicsQueueFinishedFence);
//
//	vk::PresentInfoKHR present;
//	present.swapchainCount = 1;
//	present.pSwapchains = &deviceVulkan->swapchain.swapchain;
//	present.pImageIndices = &currentBuffer;
//	present.pWaitSemaphores = &rendererFinishedSemaphore;	//
//	present.waitSemaphoreCount = 1;
//	present.pResults = 0;
//
//	deviceVulkan->presentQueue.presentKHR(&present);
//
//}
//
//void RendererVulkan::BeginFrame(const NewCamera& iCamera, const std::vector<Light>& lights)
//{
//	UpdateUniformbufferFrame((currentBuffer + 1) % 2, iCamera, lights);
//}
//
//void SetupTexturesForObject(Material& material, std::vector<BufferVulkan>& iStagingBuffers, vk::CommandBuffer iBuffer);
//
//
//
//
//// Will screw up if the GPU is already running, should fence it

//
//	EndSingleTimeCommands(deviceVulkan->device, cmdBufferResources, cmdPool->GetPool(), deviceVulkan->graphicsQueue);
//
//	
//	
//	
//	
//
//	stagingBuffers.clear();
//	stagingBuffers.resize(0);
//	renderPassScene = std::make_unique<RenderScenePass>();
//	renderPassScene->CreateRenderpass(deviceVulkan->device, deviceVulkan->swapchain.format, NUM_SAMPLES);
//
//	SetupUniformbuffer();
//	SetupShaders();
//
//	SetupDepthbuffer();
//
//	SetupScenePassData();
//
//	SetupFramebuffers();
//	SetupQuerypool(deviceVulkan->device);
//
//
//	vk::CommandBuffer cmdBufferTextures = BeginSingleTimeCommands(deviceVulkan->device, cmdPool->GetPool());
//	TransitionImageLayout(cmdBufferTextures, depthBuffer.image, depthBuffer.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
//
//	
//	
//	
//	
//
//	
//	
//	
//
//	
//
//	
//	
//	
//	
//	
//	
//
//
//
//	
//	
//
//
//	
//	
//	
//
//	
//	
//	
//
//	
//	
//
//	EndSingleTimeCommands(deviceVulkan->device, cmdBufferTextures, cmdPool->GetPool(), deviceVulkan->graphicsQueue);
//	for (auto& e : stagingBuffers)
//	{
//		vmaDestroyBuffer(allocator, e.buffer, e.allocation);
//	}
//	stagingBuffers.clear();
//	stagingBuffers.resize(0);
//
//	SetupPipeline();
//	SetupPostProcPipeline();
//
//	SetupSemaphores();
//
//	CreateTextureSampler(deviceVulkan->device);
//
//	SetupDescriptorSet(iObjects);
//}
//
//
//void RendererVulkan::Create(std::vector<Object>& iObjects, 
//	ResourceManager* const iResourceManager,
//	iLowLevelWindow* const iIlowLevelWindow)
//{
//
//	SetupApplication(iIlowLevelWindow);
//
//	SetupDevice();
//
//	for (int i = 0; i < NUM_FRAMES; ++i)
//	{
//		auto contextResources = std::make_unique<RenderingContextResources>();
//		renderingContextResources.push_back(std::move(contextResources));
//	}
//
//	SetupSwapchain();
//	deviceVulkan->SetupDeviceQueue();
//	SetupCommandBuffer();
//
//	SetupIMGUI(iIlowLevelWindow);
//
//}
//


#define NUM_DESCRIPTOR_SETS 4

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <imgui.h>
#include <imgui_impl_glfw_vulkan.h>

#include "RendererVulkan.h"
#include "BackendVulkan.h"
#include "Object.h"
#include "VulkanHelper.h"
#include "TextureVulkan.h"
#include "ModelVulkan.h"
#include "ShaderProgramVulkan.h"
#include "ObjectRenderingDataVulkan.h"
#include "PipelineCreationDump.h"
#include "CommandpoolVulkan.h"

#include "GLFWLowLevelWindow.h"
#include "ConstantBuffers.h"
#include "NewCamera.h"
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
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
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
	vkCreateDescriptorPool((VkDevice)backend->context.device, &pool_info, nullptr, &tPool);
	imguiDataObj.descriptorPool = tPool;

	VkAttachmentDescription attachment = {};
	attachment.format = VkFormat(backend->swapchainFormat);
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
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
	vkCreateRenderPass(backend->context.device, &info, nullptr, &tRP);
	imguiDataObj.renderpass = tRP;

	{
		vk::ImageView attachment[1];
		vk::FramebufferCreateInfo fbInfo = {};
		fbInfo.renderPass = imguiDataObj.renderpass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = attachment;
		fbInfo.width = backend->context.currentParameters.width;
		fbInfo.height = backend->context.currentParameters.height;
		fbInfo.layers = 1;
		for (uint32_t i = 0; i <  backend->swapchainViews.size(); i++)
		{
			attachment[0] = backend->swapchainViews[i];
			imguiDataObj.framebuffer.push_back(backend->context.device.createFramebuffer(fbInfo, nullptr));
		}

	}

	ImGui_ImplGlfwVulkan_Init_Data imgui_init_data;
	imgui_init_data.allocator = VK_NULL_HANDLE;
	imgui_init_data.device = backend->context.device;
	imgui_init_data.gpu = backend->context.gpu->device;
	imgui_init_data.pipeline_cache = VK_NULL_HANDLE;
	imgui_init_data.render_pass = tRP;
	imgui_init_data.descriptor_pool = tPool;
	imgui_init_data.check_vk_result = check_vk_result;

	GLFWLowLevelWindow* glfwWindow = dynamic_cast<GLFWLowLevelWindow*>(iIlowLevelWindow);
	ImGui_ImplGlfwVulkan_Init(glfwWindow->window, false, &imgui_init_data);

	{
		vk::CommandBufferBeginInfo begin_info = vk::CommandBufferBeginInfo();

		contextResources[backend->context.currentFrame]->imguiBuffer.begin(begin_info);
		ImGui_ImplGlfwVulkan_CreateFontsTexture(contextResources[backend->context.currentFrame]->imguiBuffer);
		contextResources[backend->context.currentFrame]->imguiBuffer.end();

		vk::SubmitInfo end_info = vk::SubmitInfo()
			.setCommandBufferCount(1)
			.setPCommandBuffers(&contextResources[backend->context.currentFrame]->imguiBuffer);
		backend->context.graphicsQueue.submit(end_info, vk::Fence(nullptr));
		backend->context.device.waitIdle();
		ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();
	}
}

void RendererVulkan::InitViewports(const vk::CommandBuffer& aBuffer)
{
	viewPort.height = (float)backend->context.currentParameters.height;
	viewPort.width = (float)backend->context.currentParameters.width;
	viewPort.minDepth = (float)0.0f;
	viewPort.maxDepth = (float)1.0f;
	viewPort.x = 0;
	viewPort.y = 0;

	aBuffer.setViewport(0, 1, &viewPort);
}

void RendererVulkan::InitScissors(const vk::CommandBuffer& aBuffer)
{
	scissor.extent = vk::Extent2D(backend->context.currentParameters.width, backend->context.currentParameters.height);
	scissor.offset = vk::Offset2D(0, 0);

	aBuffer.setScissor(0, 1, &scissor);
}

void RendererVulkan::SetupSamplers()
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

	samplerLinearRepeat = backend->context.device.createSampler(samplerInfo);

}

void RendererVulkan::SetupShaders()
{
	// Create shader program, load the shaders and then the layout and pipeline will be setup

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

	shaderProgramPBR->LoadShaders(backend->context.device, shaderDataPBR);


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

	shaderProgramRed->LoadShaders(backend->context.device, shaderDataRed);


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

	shaderProgramPostProc->LoadShaders(backend->context.device, shaderDataPostProc);

}

void RendererVulkan::SetupPipeline()
{
	auto shaderLayoutPBR = shaderProgramPBR->GetShaderProgramLayout();

	vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setPNext(NULL)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(NULL)
		.setSetLayoutCount(NUM_DESCRIPTOR_SETS)
		.setPSetLayouts(shaderLayoutPBR.data());

	pipelineLayoutRenderScene = backend->context.device.createPipelineLayout(pPipelineLayoutCreateInfo);

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
		.setRasterizationSamples(NUM_MULTISAMPLES)
		.setSampleShadingEnable(VK_FALSE)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE)
		.setMinSampleShading(0.0f)
		.setPSampleMask(VK_NULL_HANDLE);

	std::vector<vk::PipelineShaderStageCreateInfo> shaderPipelineInfo = shaderProgramPBR->GetPipelineShaderInfo();

	// Create graphics pipeline for the first shader
	vk::GraphicsPipelineCreateInfo gfxPipe = GraphicsPipelineCreateInfo()
		.setLayout(pipelineLayoutRenderScene)
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
		.setRenderPass(backend->context.renderpass)
		.setSubpass(0);

	pipelinePBR = backend->context.device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipe);


	// Create graphics pipeline for the second shader
	std::vector<vk::PipelineShaderStageCreateInfo> shaderPipelineInfoRed = shaderProgramRed->GetPipelineShaderInfo();

	vk::GraphicsPipelineCreateInfo gfxPipe2 = GraphicsPipelineCreateInfo()
		.setLayout(pipelineLayoutRenderScene)
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
		.setRenderPass(backend->context.renderpass)
		.setSubpass(0);


	pipelineRed = backend->context.device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipe2);
}

void RendererVulkan::SetupCommandPoolAndBuffers()
{
	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		ContextResources* const resources = contextResources[i].get();

		resources->pool = std::make_unique<CommandpoolVulkan>();
		resources->pool->Create(backend->context.device, backend->context.graphicsFamilyIndex, CommandPoolCreateFlagBits::eResetCommandBuffer);

		resources->baseBuffer = resources->pool->AllocateBuffer(backend->context.device, CommandBufferLevel::ePrimary, 1)[0];
		resources->imguiBuffer = resources->pool->AllocateBuffer(backend->context.device, CommandBufferLevel::ePrimary, 1)[0];
	}
}

void RendererVulkan::Resize(const GFXParams& iParams)
{

}

void RendererVulkan::PrepareResources(
	std::queue<iTexture*> iTexturesToPrepare,
	std::queue<iModel*> iModelsToPrepare,
	std::queue<iObjectRenderingData*> iObjsToPrepare,
	const std::vector<Object>& iObjects)
{
	std::vector<BufferVulkan> stagingBuffers;
	vk::CommandBuffer cmdBufferResources = BeginSingleTimeCommands(backend->context.device, contextResources[backend->context.currentFrame]->pool->GetPool());

	// Prepare the texture resources
	while (!iTexturesToPrepare.empty())
	{
		iTexture* textureToWorkOn = iTexturesToPrepare.front();
		iTexturesToPrepare.pop();
		TextureVulkan* diffuseTexture = dynamic_cast<TextureVulkan*>(textureToWorkOn);

		SetupTextureImage(cmdBufferResources, backend->context.device, diffuseTexture->GetFilepath(), diffuseTexture->data.image, backend->allocator, diffuseTexture->data.allocation, stagingBuffers);
		diffuseTexture->data.view = CreateImageView(backend->context.device, diffuseTexture->data.image, Format::eR8G8B8A8Unorm);
		diffuseTexture->isPrepared = true;
		textures.push_back(diffuseTexture);
	}

	// Prepare the models
	while (!iModelsToPrepare.empty())
	{
		iModel* modelToWorkOn = iModelsToPrepare.front();
		iModelsToPrepare.pop();

		if (!modelToWorkOn->isPrepared)
		{
			auto e = dynamic_cast<ModelVulkan*>(modelToWorkOn);

			SetupVertexBuffer(backend->context.device, cmdBufferResources, backend->allocator, e->vertexBuffer, e->data, stagingBuffers);
			SetupIndexBuffer(backend->context.device, cmdBufferResources, backend->allocator, e->indexBuffer, e->data, stagingBuffers);


			e->indiceCount = e->data.indices.size();

			e->isPrepared = true;
			models.push_back(e);
		}
	}
	
	EndSingleTimeCommands(backend->context.device, cmdBufferResources, contextResources[backend->context.currentFrame]->pool->GetPool(), backend->context.graphicsQueue);

	for (auto& e : stagingBuffers)
	{
		vmaDestroyBuffer(backend->allocator, e.buffer, e.allocation);
	}

	stagingBuffers.clear();
	stagingBuffers.resize(0);

	// Setup the uniform buffers for the objects
	for (int i = 0; i < iObjects.size(); ++i)
	{
		iObjectRenderingData* objectToWorkOn = iObjects[i].renderingData;
		//iObjsToPrepare.pop();

		if (!objectToWorkOn->isPrepared)
		{
			auto e = dynamic_cast<ObjectRenderingDataVulkan*>(objectToWorkOn);

			UniformBufferVulkan tUniformBuff;

			CreateSimpleBuffer(backend->allocator,
				tUniformBuff.allocation,
				VMA_MEMORY_USAGE_CPU_TO_GPU,
				tUniformBuff.buffer,
				vk::BufferUsageFlagBits::eUniformBuffer,
				sizeof(CBMatrix));

			matrixSingleData.model = iObjects[i].modelMatrix;
			CopyDataToBuffer(VkDevice(backend->context.device), tUniformBuff.allocation, (void*)&matrixSingleData, sizeof(matrixSingleData));

			tUniformBuff.descriptorInfo.buffer = tUniformBuff.buffer;
			tUniformBuff.descriptorInfo.offset = 0;
			tUniformBuff.descriptorInfo.range = sizeof(CBModelMatrixSingle);

			e->positionUniformBuffer = tUniformBuff;
			e->isPrepared = true;
			objRenderingData.push_back(e);

		}
	}
}

void RendererVulkan::SetupUniformBuffers()
{
	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		UniformBufferVulkan tUniformBuff;

		CreateSimpleBuffer(backend->allocator,
			tUniformBuff.allocation,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			tUniformBuff.buffer,
			vk::BufferUsageFlagBits::eUniformBuffer,
			sizeof(CBMatrix));

		CopyDataToBuffer(VkDevice(backend->context.device), tUniformBuff.allocation, (void*)&matrixConstantBufferData, sizeof(matrixConstantBufferData));


		tUniformBuff.descriptorInfo.buffer = tUniformBuff.buffer;
		tUniformBuff.descriptorInfo.offset = 0;
		tUniformBuff.descriptorInfo.range = sizeof(matrixConstantBufferData);

		contextResources[i]->uniformBufferMVP = tUniformBuff;
	}

	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		UniformBufferVulkan tUniformBuff;

		CreateSimpleBuffer(backend->allocator,
			tUniformBuff.allocation,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			tUniformBuff.buffer,
			vk::BufferUsageFlagBits::eUniformBuffer,
			sizeof(CBLights));

		CopyDataToBuffer(VkDevice(backend->context.device), tUniformBuff.allocation, (void*)&lightConstantBufferData, sizeof(CBLights));


		tUniformBuff.descriptorInfo.buffer = tUniformBuff.buffer;
		tUniformBuff.descriptorInfo.offset = 0;
		tUniformBuff.descriptorInfo.range = sizeof(CBLights);

		contextResources[i]->uniformBufferLights = tUniformBuff;
	}
}

void RendererVulkan::UpdateUniformBufferFrame(const NewCamera& iCam, const std::vector<Light>& iLights)
{

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

	CopyDataToBuffer(VkDevice(backend->context.device), contextResources[backend->context.currentFrame]->uniformBufferMVP.allocation, (void*)&matrixConstantBufferData, sizeof(matrixConstantBufferData));

	lightConstantBufferData.currAmountOfLights = std::min(static_cast<uint32_t>(iLights.size()), (uint32_t)16);

	for (int i = 0; i < lightConstantBufferData.currAmountOfLights; ++i)
	{
		lightConstantBufferData.lights[i] = iLights[i];
	}
	
	CopyDataToBuffer(VkDevice(backend->context.device), contextResources[backend->context.currentFrame]->uniformBufferLights.allocation, (void*)&lightConstantBufferData, sizeof(lightConstantBufferData));

}

void RendererVulkan::BeginFrame(const NewCamera& iCamera, const std::vector<Light>& iLights)
{
	UpdateUniformBufferFrame(iCamera, iLights);
}

void RendererVulkan::SetupCommandBuffersImgui()
{

	vk::ClearValue clear_values[1] = {};
	clear_values[0].color.float32[0] = 1.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 0.0f;

	vk::CommandBufferBeginInfo begin_info = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	contextResources[backend->context.currentFrame]->imguiBuffer.begin(begin_info);

	vk::RenderPassBeginInfo rp_begin = vk::RenderPassBeginInfo()
		.setRenderPass(imguiDataObj.renderpass)
		.setFramebuffer(imguiDataObj.framebuffer[backend->currentSwapIndex])
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(backend->context.currentParameters.width, backend->context.currentParameters.height)))
		.setClearValueCount(3)
		.setPClearValues(clear_values);

	contextResources[backend->context.currentFrame]->imguiBuffer.beginRenderPass(&rp_begin, SubpassContents::eInline);

	ImGui_ImplGlfwVulkan_Render(contextResources[backend->context.currentFrame]->imguiBuffer);
	contextResources[backend->context.currentFrame]->imguiBuffer.endRenderPass();

	contextResources[backend->context.currentFrame]->imguiBuffer.end();
}

void RendererVulkan::Render(const std::vector<Object>& iObjects)
{
	backend->AcquireImage();
	SetupCommandBuffersImgui();
	backend->BeginFrame();
	backend->EndFrame(contextResources[backend->context.currentFrame]->imguiBuffer);
	backend->BlockSwapBuffers();
}

void RendererVulkan::Destroy()
{
	shaderProgramPBR->Destroy(backend->context.device);
	shaderProgramRed->Destroy(backend->context.device);
	shaderProgramPostProc->Destroy(backend->context.device);

	for (auto& e : models)
	{
		vmaDestroyBuffer(backend->allocator, e->vertexBuffer.buffer, e->vertexBuffer.allocation);
		vmaDestroyBuffer(backend->allocator, e->indexBuffer.buffer, e->indexBuffer.allocation);
	}

	for (auto& e : textures)
	{
		vmaDestroyImage(backend->allocator, e->data.image, e->data.allocation);
		backend->context.device.destroyImageView(e->data.view);
	}

	for (auto& e : objRenderingData)
	{
		vmaDestroyBuffer(backend->allocator, e->positionUniformBuffer.buffer, e->positionUniformBuffer.allocation);
	}


	for (auto& e : contextResources)
	{
		vmaDestroyBuffer(backend->allocator, e->uniformBufferLights.buffer, e->uniformBufferLights.allocation);
		vmaDestroyBuffer(backend->allocator, e->uniformBufferModelMatrix.buffer, e->uniformBufferModelMatrix.allocation);
		vmaDestroyBuffer(backend->allocator, e->uniformBufferMVP.buffer, e->uniformBufferMVP.allocation);

		e->pool->Destroy(backend->context.device);
	}
	backend->Shutdown();
}

void RendererVulkan::Initialize(const GFXParams& iParams, iLowLevelWindow* const iWindow)
{
	backend = std::make_unique<BackendVulkan>();
	backend->Init(iParams, iWindow);

	// Ready our context resources for filling in
	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		auto tContextResources = std::make_unique<ContextResources>();

		contextResources.push_back(std::move(tContextResources));
	}

	// Setup and load the shaders and corresponding pipelines
	SetupShaders();
	SetupPipeline();

	// Setup samplers
	SetupSamplers();

	// Setup command pool and buffers, afterwards use a buffer to setup Imgui
	SetupCommandPoolAndBuffers();
	SetupIMGUI(iWindow);

	SetupUniformBuffers();
}
