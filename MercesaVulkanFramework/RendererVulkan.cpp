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
#include "DescriptorPoolVulkan.h"
#include "GLFWLowLevelWindow.h"
#include "ConstantBuffers.h"
#include "NewCamera.h"
#include "FramebufferVulkan.h"


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

	shaderProgramBloomCompute = std::make_unique<ShaderProgramVulkan>();
	
	ShaderDataVulkan computeDataBloom;

	computeDataBloom.entryPointName = "main";
	computeDataBloom.shaderStage = ShaderStageFlagBits::eCompute;
	computeDataBloom.shaderFile = "Shaders/Bin/gaussianComputeShader.spv";


	std::vector<ShaderDataVulkan> shaderDataComputeBloom = { computeDataBloom };

	shaderProgramBloomCompute->LoadShaders(backend->context.device, shaderDataComputeBloom);
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


	vk::PipelineColorBlendStateCreateInfo cb = {};
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

	vk::GraphicsPipelineCreateInfo gfxPipeFirst = GraphicsPipelineCreateInfo()
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
		.setRenderPass(offscreenTest->renderpass)
		.setSubpass(0);


	pipelineRenderScenePBR = backend->context.device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipeFirst);

	// Create graphics pipeline for the second shader
	std::vector<vk::PipelineShaderStageCreateInfo> shaderPipelineInfoRed = shaderProgramRed->GetPipelineShaderInfo();


	vk::GraphicsPipelineCreateInfo gfxPipeSecond = GraphicsPipelineCreateInfo()
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
		.setRenderPass(offscreenTest->renderpass)
		.setSubpass(0);


	pipelineRenderSceneRed = backend->context.device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipeSecond);

	
}

void RendererVulkan::SetupPipelinePostProc()
{
	auto shaderLayout = shaderProgramPostProc->GetShaderProgramLayout();

	vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setPNext(NULL)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(NULL)
		.setSetLayoutCount(shaderLayout.size())
		.setPSetLayouts(shaderLayout.data());

	pipelineLayoutPostProc = backend->context.device.createPipelineLayout(pPipelineLayoutCreateInfo);

	std::vector<vk::VertexInputAttributeDescription> inputAttributes;
	vk::VertexInputBindingDescription inputDescription;


	inputDescription.binding = 0;
	inputDescription.inputRate = vk::VertexInputRate::eVertex;
	inputDescription.stride = sizeof(VertexData);


	vk::PipelineVertexInputStateCreateInfo vi = vk::PipelineVertexInputStateCreateInfo()
		.setFlags(PipelineVertexInputStateCreateFlagBits(0))
		.setPVertexBindingDescriptions(&inputDescription)
		.setPVertexAttributeDescriptions(inputAttributes.data())
		.setVertexAttributeDescriptionCount(inputAttributes.size())
		.setVertexBindingDescriptionCount(0);

	vk::PipelineInputAssemblyStateCreateInfo ia = vk::PipelineInputAssemblyStateCreateInfo()
		.setPrimitiveRestartEnable(VK_FALSE)
		.setTopology(vk::PrimitiveTopology::eTriangleList);


		vk::PipelineRasterizationStateCreateInfo rs = vk::PipelineRasterizationStateCreateInfo()
			.setPolygonMode(vk::PolygonMode::eFill)
			.setCullMode(vk::CullModeFlagBits::eFront)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthClampEnable(VK_FALSE)
			.setRasterizerDiscardEnable(VK_FALSE)
			.setDepthBiasEnable(VK_FALSE)
			.setDepthBiasConstantFactor(0)
			.setDepthBiasClamp(0)
			.setDepthBiasSlopeFactor(0)
			.setLineWidth(1.0f);




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

	std::vector<vk::PipelineShaderStageCreateInfo> shaderPipelineInfo = shaderProgramPostProc->GetPipelineShaderInfo();

	// Create graphics pipeline for the first shader
	vk::GraphicsPipelineCreateInfo gfxPipe = GraphicsPipelineCreateInfo()
		.setLayout(pipelineLayoutPostProc)
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

	pipelinePostProc = backend->context.device.createGraphicsPipeline(vk::PipelineCache(nullptr), gfxPipe);

}

void RendererVulkan::SetupDescriptorSet(const std::vector<Object>& iObjects)
{
	descriptorPool = std::make_unique<DescriptorPoolVulkan>();

	descriptorPool->Create(backend->context.device, PoolData(100, 10, 10, 400, 2, 100));

	auto shaderDescriptorLayoutPBR = shaderProgramPBR->GetShaderProgramLayout();
	auto shaderDescriptorLayoutPostProc = shaderProgramPostProc->GetShaderProgramLayout();
	auto shaderDescriptorLayoutBloomCompute = shaderProgramPostProc->GetShaderProgramLayout();

	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		contextResources[i]->descriptorSetPBRShader.samplerSet = descriptorPool->AllocateDescriptorSet(backend->context.device, 1, shaderDescriptorLayoutPBR[0], bindings)[0];
		contextResources[i]->descriptorSetPBRShader.perFrameUniformBufferSet = descriptorPool->AllocateDescriptorSet(backend->context.device, 1, shaderDescriptorLayoutPBR[1], uniformBinding)[0];

		// Setup descriptor set for this shader
		contextResources[i]->descriptorSetPostProc.inputTextureSet = descriptorPool->AllocateDescriptorSet(backend->context.device, 1, shaderDescriptorLayoutPostProc[0], postProcBinding)[0];

		contextResources[i]->descriptorSetBloomCompute.horizontalSet = descriptorPool->AllocateDescriptorSet(backend->context.device, 1, shaderDescriptorLayoutBloomCompute[0], bloomComputeBinding)[0];
		contextResources[i]->descriptorSetBloomCompute.verticalSet = descriptorPool->AllocateDescriptorSet(backend->context.device, 1, shaderDescriptorLayoutBloomCompute[0], bloomComputeBinding)[0];
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

		tModel->textureSet = descriptorPool->AllocateDescriptorSet(backend->context.device, 1, shaderDescriptorLayoutPBR[2], textureBinding)[0];
		tRenderingData->positionBufferSet = descriptorPool->AllocateDescriptorSet(backend->context.device, 1, shaderDescriptorLayoutPBR[3], uniformBinding)[0];


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

		backend->context.device.updateDescriptorSets(static_cast<uint32_t>(textureWrites.size()), textureWrites.data(), 0, NULL);


		uniformModelWrite[0] = {};
		uniformModelWrite[0].pNext = NULL;
		uniformModelWrite[0].dstSet = tRenderingData->positionBufferSet;
		uniformModelWrite[0].descriptorCount = 1;
		uniformModelWrite[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniformModelWrite[0].pBufferInfo = &tRenderingData->positionUniformBuffer.descriptorInfo;
		uniformModelWrite[0].dstArrayElement = 0;
		uniformModelWrite[0].dstBinding = 0;

		backend->context.device.updateDescriptorSets(static_cast<uint32_t>(uniformModelWrite.size()), uniformModelWrite.data(), 0, NULL);

	}

	// Setup descriptors for the render scene shaders
	for (int i = 0; i < contextResources.size(); ++i)
	{
		std::array<vk::WriteDescriptorSet, 1> writes = {};


		// Create image info for the image descriptor
		vk::DescriptorImageInfo pureSamplerInfo = {};

		pureSamplerInfo.imageView = vk::ImageView(nullptr);
		pureSamplerInfo.sampler = samplerLinearRepeat;

		writes[0] = {};
		writes[0].pNext = NULL;
		writes[0].dstSet = contextResources[i]->descriptorSetPBRShader.samplerSet;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = vk::DescriptorType::eSampler;
		writes[0].pImageInfo = &pureSamplerInfo;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = 0;


		backend->context.device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, NULL);

		std::array<vk::WriteDescriptorSet, 2> uniform_writes = {};

		uniform_writes[0] = {};
		uniform_writes[0].pNext = NULL;
		uniform_writes[0].dstSet = contextResources[i]->descriptorSetPBRShader.perFrameUniformBufferSet;
		uniform_writes[0].descriptorCount = 1;
		uniform_writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniform_writes[0].pBufferInfo = &contextResources[i]->uniformBufferMVP.descriptorInfo;
		uniform_writes[0].dstArrayElement = 0;
		uniform_writes[0].dstBinding = 0;

		uniform_writes[1] = {};
		uniform_writes[1].pNext = NULL;
		uniform_writes[1].dstSet = contextResources[i]->descriptorSetPBRShader.perFrameUniformBufferSet;
		uniform_writes[1].descriptorCount = 1;
		uniform_writes[1].descriptorType = vk::DescriptorType::eUniformBuffer;
		uniform_writes[1].pBufferInfo = &contextResources[i]->uniformBufferLights.descriptorInfo;
		uniform_writes[1].dstArrayElement = 0;
		uniform_writes[1].dstBinding = 1;


		backend->context.device.updateDescriptorSets(static_cast<uint32_t>(uniform_writes.size()), uniform_writes.data(), 0, NULL);
	}

	// Setup descriptors for the post proc scene
	for (int i = 0; i < contextResources.size(); ++i)
	{
		std::array<vk::WriteDescriptorSet, 2> writes = {};

		writes[0] = {};
		writes[0].pNext = NULL;
		writes[0].dstSet = contextResources[i]->descriptorSetPostProc.inputTextureSet;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		writes[0].pImageInfo = &offscreenTest->descriptorColorTexture;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = 0;

		writes[1] = {};
		writes[1].pNext = NULL;
		writes[1].dstSet = contextResources[i]->descriptorSetPostProc.inputTextureSet;
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		writes[1].pImageInfo = &bloomData->descriptorTexture1;
		writes[1].dstArrayElement = 0;
		writes[1].dstBinding = 1;

		backend->context.device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, NULL);

	}

	// Setup descriptors for the post proc scene
	for (int i = 0; i < contextResources.size(); ++i)
	{
		// First descriptor set
		std::array<vk::WriteDescriptorSet, 2> writes = {};

		writes[0] = {};
		writes[0].pNext = NULL;
		writes[0].dstSet = contextResources[i]->descriptorSetBloomCompute.horizontalSet;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		writes[0].pImageInfo = &bloomData->descriptorTexture1;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = 0;

		writes[1] = {};
		writes[1].pNext = NULL;
		writes[1].dstSet = contextResources[i]->descriptorSetBloomCompute.horizontalSet;
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		writes[1].pImageInfo = &bloomData->descriptorTexture2;
		writes[1].dstArrayElement = 0;
		writes[1].dstBinding = 1;

		backend->context.device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, NULL);

		// Prepare the second descriptor set
		std::array<vk::WriteDescriptorSet, 2> writes2 = {};

		writes2[0] = {};
		writes2[0].pNext = NULL;
		writes2[0].dstSet = contextResources[i]->descriptorSetBloomCompute.verticalSet;
		writes2[0].descriptorCount = 1;
		writes2[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		writes2[0].pImageInfo = &bloomData->descriptorTexture2;
		writes2[0].dstArrayElement = 0;
		writes2[0].dstBinding = 0;

		writes2[1] = {};
		writes2[1].pNext = NULL;
		writes2[1].dstSet = contextResources[i]->descriptorSetBloomCompute.verticalSet;
		writes2[1].descriptorCount = 1;
		writes2[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		writes2[1].pImageInfo = &bloomData->descriptorTexture1;
		writes2[1].dstArrayElement = 0;
		writes2[1].dstBinding = 1;

		backend->context.device.updateDescriptorSets(static_cast<uint32_t>(writes2.size()), writes2.data(), 0, NULL);

	}


}

void RendererVulkan::SetupCommandPoolAndBuffers()
{
	for (int i = 0; i < NUM_FRAMES; ++i)
	{
		ContextResources* const resources = contextResources[i].get();

		resources->cmdPoolGfx = std::make_unique<CommandpoolVulkan>();
		resources->cmdPoolGfx->Create(backend->context.device, backend->context.graphicsFamilyIndex, CommandPoolCreateFlagBits::eResetCommandBuffer);

		resources->baseBuffer = resources->cmdPoolGfx->AllocateBuffer(backend->context.device, CommandBufferLevel::ePrimary, 1)[0];
		resources->imguiBuffer = resources->cmdPoolGfx->AllocateBuffer(backend->context.device, CommandBufferLevel::ePrimary, 1)[0];


		resources->cmdPoolCompute = std::make_unique<CommandpoolVulkan>();
		resources->cmdPoolCompute->Create(backend->context.device, backend->context.computeFamilyIndex, CommandPoolCreateFlagBits::eResetCommandBuffer);

		resources->bloomBufferCompute = resources->cmdPoolCompute->AllocateBuffer(backend->context.device, CommandBufferLevel::ePrimary, 1)[0];


		vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo();
		resources->renderSceneFence = backend->context.device.createFence(fenceCreateInfo);
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
	vk::CommandBuffer cmdBufferResources = BeginSingleTimeCommands(backend->context.device, contextResources[backend->context.currentFrame]->cmdPoolGfx->GetPool());

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
	
	EndSingleTimeCommands(backend->context.device, cmdBufferResources, contextResources[backend->context.currentFrame]->cmdPoolGfx->GetPool(), backend->context.graphicsQueue);

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
		iObjsToPrepare.pop();

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
	
	SetupDescriptorSet(iObjects);
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

void RendererVulkan::RecordCommandBuffersImgui()
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

void RendererVulkan::RenderScene(const std::vector<Object>& iObjects)
{
	// Render to scene
	vk::CommandBuffer sceneRenderbuffer = this->contextResources[backend->context.currentFrame]->baseBuffer;

	// Clear our render targets at start frame
	vk::ClearValue clear_values[3] = {};
	clear_values[0].color.float32[0] = 1.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 1.0f;
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0.0f;

	clear_values[2].color.float32[0] = 0.0f;
	clear_values[2].color.float32[1] = 0.0f;
	clear_values[2].color.float32[2] = 0.0f;
	clear_values[2].color.float32[3] = 1.0f;

	vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(offscreenTest->renderpass)
		.setFramebuffer(offscreenTest->framebuffer)
		.setClearValueCount(3)
		.setPClearValues(clear_values);

	renderPassBeginInfo.renderArea.extent = vk::Extent2D(offscreenTest->width, offscreenTest->height);

	// begin buffer and render pass
	vk::CommandBufferBeginInfo cmdBufferBeginInfo = vk::CommandBufferBeginInfo();
	sceneRenderbuffer.begin(cmdBufferBeginInfo);
	sceneRenderbuffer.beginRenderPass(renderPassBeginInfo, SubpassContents::eInline);

	const vk::DeviceSize offsets[1] = { 0 };

	sceneRenderbuffer.bindPipeline(PipelineBindPoint::eGraphics, pipelineRenderScenePBR);

	InitViewports(sceneRenderbuffer);
	InitScissors(sceneRenderbuffer);

	std::vector<DescriptorSet> totalSet;
	// Add our descriptor data to this set, and use this set
	totalSet.resize(4);

	// render all the objects except the last first (multiple shader test)
	for (int j = 0; j < (iObjects.size() - 1); ++j)
	{
		ModelVulkan* model = dynamic_cast<ModelVulkan*>(iObjects[j].model);
		ObjectRenderingDataVulkan* renderingData = dynamic_cast<ObjectRenderingDataVulkan*>(iObjects[j].renderingData);

		contextResources[backend->context.currentFrame]->descriptorSetPBRShader.textureSet = model->textureSet;
		contextResources[backend->context.currentFrame]->descriptorSetPBRShader.perObjectUniformBufferSet = renderingData->positionBufferSet;


		totalSet[0] = (contextResources[backend->context.currentFrame]->descriptorSetPBRShader.samplerSet);
		totalSet[1] = (contextResources[backend->context.currentFrame]->descriptorSetPBRShader.perFrameUniformBufferSet);
		totalSet[2] = (contextResources[backend->context.currentFrame]->descriptorSetPBRShader.textureSet);
		totalSet[3] = (contextResources[backend->context.currentFrame]->descriptorSetPBRShader.perObjectUniformBufferSet);

		sceneRenderbuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayoutRenderScene, 0, totalSet.size(), totalSet.data(), 0, NULL);
		sceneRenderbuffer.bindVertexBuffers(0, 1, &model->vertexBuffer.buffer, offsets);
		sceneRenderbuffer.bindIndexBuffer(model->indexBuffer.buffer, 0, IndexType::eUint32);
		sceneRenderbuffer.drawIndexed(model->GetIndiceCount(), 1, 0, 0, 0);
	}

	// Render last object with different shader
	ModelVulkan* model = dynamic_cast<ModelVulkan*>(iObjects[iObjects.size() - 1].model);
	ObjectRenderingDataVulkan* renderingData = dynamic_cast<ObjectRenderingDataVulkan*>(iObjects[iObjects.size() - 1].renderingData);

	sceneRenderbuffer.bindPipeline(PipelineBindPoint::eGraphics, pipelineRenderSceneRed);
	InitViewports(sceneRenderbuffer);
	InitScissors(sceneRenderbuffer);

	contextResources[backend->context.currentFrame]->descriptorSetPBRShader.textureSet = model->textureSet;
	contextResources[backend->context.currentFrame]->descriptorSetPBRShader.perObjectUniformBufferSet = renderingData->positionBufferSet;

	totalSet[0] = (contextResources[backend->context.currentFrame]->descriptorSetPBRShader.samplerSet);
	totalSet[1] = (contextResources[backend->context.currentFrame]->descriptorSetPBRShader.perFrameUniformBufferSet);
	totalSet[2] = (contextResources[backend->context.currentFrame]->descriptorSetPBRShader.textureSet);
	totalSet[3] = (contextResources[backend->context.currentFrame]->descriptorSetPBRShader.perObjectUniformBufferSet);

	sceneRenderbuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayoutRenderScene, 0, totalSet.size(), totalSet.data(), 0, NULL);
	sceneRenderbuffer.bindVertexBuffers(0, 1, &model->vertexBuffer.buffer, offsets);
	sceneRenderbuffer.bindIndexBuffer(model->indexBuffer.buffer, 0, IndexType::eUint32);
	sceneRenderbuffer.drawIndexed(model->GetIndiceCount(), 1, 0, 0, 0);


	sceneRenderbuffer.endRenderPass();
	sceneRenderbuffer.end();
}

void RendererVulkan::Render(const std::vector<Object>& iObjects)
{
	// Wait until last frame is done rendering
	backend->BlockUntilGpuIdle();

	backend->AcquireImage();
	
	// prepare command buffer
	RenderScene(iObjects);

	// submit command buffer to graphics queue
	vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::CommandBuffer buffers[1]{ contextResources[backend->context.currentFrame]->baseBuffer };

	vk::SubmitInfo subInfo = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(&contextResources[backend->context.currentFrame]->baseBuffer)
		.setWaitSemaphoreCount(0)
		.setPWaitSemaphores(VK_NULL_HANDLE)
		.setSignalSemaphoreCount(0)
		.setPSignalSemaphores(VK_NULL_HANDLE)
		.setPWaitDstStageMask(&dstStageMask);



	// Very important! the imgui renderpass implicitly sets the swapchain view to present
	backend->context.graphicsQueue.submit(1, &subInfo, contextResources[backend->context.currentFrame]->renderSceneFence);

	// wait and reset fence
	backend->context.device.waitForFences(contextResources[backend->context.currentFrame]->renderSceneFence, VK_TRUE, UINT64_MAX);
	backend->context.device.resetFences(contextResources[backend->context.currentFrame]->renderSceneFence);



	vk::CommandBuffer& computeBuffer = contextResources[backend->context.currentFrame]->bloomBufferCompute;





	RecordCommandBuffersImgui();

	std::vector<DescriptorSet> postProcSets = { contextResources[backend->context.currentFrame]->descriptorSetPostProc.inputTextureSet };

	// begin rendering to our swapchain buffer
	backend->BeginFrame();

	InitViewports(backend->context.commandBuffer);
	InitScissors(backend->context.commandBuffer);
	backend->context.commandBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayoutPostProc, 0, postProcSets.size(), postProcSets.data(), 0, NULL);
	backend->context.commandBuffer.bindPipeline(PipelineBindPoint::eGraphics, pipelinePostProc);
	backend->context.commandBuffer.draw(3, 1, 0, 0);

	// Render objects here into the final pass
	backend->EndFrame(contextResources[backend->context.currentFrame]->baseBuffer,contextResources[backend->context.currentFrame]->imguiBuffer);
	backend->BlockSwapBuffers();
}

void RendererVulkan::Destroy()
{
	// Idle everything
	backend->context.device.waitIdle();
	backend->context.graphicsQueue.waitIdle();
	backend->context.presentQueue.waitIdle();

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

		e->cmdPoolGfx->Destroy(backend->context.device);
		e->cmdPoolCompute->Destroy(backend->context.device);
	}

	
	// Destroy the offscreen target
	
	vmaDestroyImage(backend->allocator, offscreenTest->colorTexture.image, offscreenTest->colorTexture.allocation);
	backend->context.device.destroyImageView(offscreenTest->colorTexture.view);

	vmaDestroyImage(backend->allocator, offscreenTest->depthTexture.image, offscreenTest->depthTexture.allocation);
	backend->context.device.destroyImageView(offscreenTest->depthTexture.view);

	backend->context.device.destroySampler(offscreenTest->sampler);
	backend->context.device.destroyFramebuffer(offscreenTest->framebuffer);
	backend->context.device.destroyRenderPass(offscreenTest->renderpass);

	// End destroy offscreen target

	descriptorPool->Destroy(backend->context.device);

	// Destroy imgui resources
	backend->context.device.destroyDescriptorPool(imguiDataObj.descriptorPool);
	backend->context.device.destroyRenderPass(imguiDataObj.renderpass);
	
	for (auto& e : imguiDataObj.framebuffer)
	{
		backend->context.device.destroyFramebuffer(e);
	}

	backend->context.device.destroyPipelineLayout(pipelineLayoutRenderScene);
	backend->context.device.destroyPipelineLayout(pipelineLayoutPostProc);

	backend->context.device.destroyPipeline(pipelineRenderScenePBR);
	backend->context.device.destroyPipeline(pipelineRenderSceneRed);
	backend->context.device.destroyPipeline(pipelinePostProc);


	backend->context.device.destroySampler(samplerLinearRepeat);

	backend->Shutdown();
}

void RendererVulkan::CreateOffscreenData()
{
	offscreenTest = std::make_unique<Offscreenpass>();

	offscreenTest->width = backend->context.currentParameters.width;
	offscreenTest->height = backend->context.currentParameters.height;

	vk::Format depthFormat = vk::Format::eD24UnormS8Uint;

	// Create image and view for colour buffer
	CreateSimpleImage(backend->allocator, 
		offscreenTest->colorTexture.allocation,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, 
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, 
		vk::Format::eR32G32B32A32Sfloat, ImageLayout::eUndefined, 
		offscreenTest->colorTexture.image, 
		offscreenTest->width, offscreenTest->height);

	vk::ImageViewCreateInfo colorImageView;
	colorImageView.format = vk::Format::eR32G32B32A32Sfloat;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;
	colorImageView.viewType = ImageViewType::e2D;
	colorImageView.image = offscreenTest->colorTexture.image;

	offscreenTest->colorTexture.view = backend->context.device.createImageView(colorImageView);

	// create image and view for depth buffer
	CreateSimpleImage(backend->allocator,
		offscreenTest->depthTexture.allocation,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY,
		vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::Format::eD24UnormS8Uint, ImageLayout::eUndefined,
		offscreenTest->depthTexture.image,
		offscreenTest->width, offscreenTest->height);


	vk::ImageViewCreateInfo depthImageView;
	depthImageView.format = vk::Format::eD24UnormS8Uint;
	depthImageView.subresourceRange = {};
	depthImageView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
	depthImageView.subresourceRange.baseMipLevel = 0;
	depthImageView.subresourceRange.levelCount = 1;
	depthImageView.subresourceRange.baseArrayLayer = 0;
	depthImageView.subresourceRange.layerCount = 1;
	depthImageView.image = offscreenTest->depthTexture.image;
	depthImageView.viewType = ImageViewType::e2D;

	offscreenTest->depthTexture.view = backend->context.device.createImageView(depthImageView);


	// Create bloom data
	bloomData = std::make_unique<BloomData>();

	// Create image and view for bloom textures
	CreateSimpleImage(backend->allocator,
		bloomData->texture1.allocation,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		vk::Format::eR32G32B32A32Sfloat, ImageLayout::eUndefined,
		bloomData->texture1.image,
		offscreenTest->width, offscreenTest->height);

	vk::ImageViewCreateInfo bloomImageView;
	bloomImageView.format = vk::Format::eR32G32B32A32Sfloat;
	bloomImageView.subresourceRange = {};
	bloomImageView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	bloomImageView.subresourceRange.baseMipLevel = 0;
	bloomImageView.subresourceRange.levelCount = 1;
	bloomImageView.subresourceRange.baseArrayLayer = 0;
	bloomImageView.subresourceRange.layerCount = 1;
	bloomImageView.viewType = ImageViewType::e2D;
	bloomImageView.image = bloomData->texture1.image;
	bloomData->texture1.view = backend->context.device.createImageView(bloomImageView);

	// Create second image
	CreateSimpleImage(backend->allocator,
		bloomData->texture2.allocation,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		vk::Format::eR32G32B32A32Sfloat, ImageLayout::eUndefined,
		bloomData->texture2.image,
		offscreenTest->width, offscreenTest->height);

	bloomData->texture2.view = backend->context.device.createImageView(bloomImageView);

	// End bloom data

	// Create renderpass
	LOG(INFO) << "Created offscreen data";


	vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo()
		.setMagFilter(Filter::eLinear)
		.setMinFilter(Filter::eLinear)
		.setMipmapMode(SamplerMipmapMode::eLinear)
		.setAddressModeU(SamplerAddressMode::eClampToEdge)
		.setAddressModeV(SamplerAddressMode::eClampToEdge)
		.setAddressModeW(SamplerAddressMode::eClampToEdge)
		.setMipLodBias(1.0f)
		.setMaxAnisotropy(1.0f)
		.setMinLod(0.0f)
		.setMaxLod(1.0f)
		.setBorderColor(BorderColor::eFloatOpaqueWhite);

	offscreenTest->sampler = backend->context.device.createSampler(samplerInfo);

	std::array<vk::AttachmentDescription, 3> attachmentDescriptions = {};

	attachmentDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
	attachmentDescriptions[0].samples = vk::SampleCountFlagBits::e1;
	attachmentDescriptions[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDescriptions[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDescriptions[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachmentDescriptions[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDescriptions[0].initialLayout = vk::ImageLayout::eUndefined;
	attachmentDescriptions[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	attachmentDescriptions[1].format = vk::Format::eD24UnormS8Uint;
	attachmentDescriptions[1].samples = vk::SampleCountFlagBits::e1;
	attachmentDescriptions[1].loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDescriptions[1].storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDescriptions[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachmentDescriptions[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDescriptions[1].initialLayout = vk::ImageLayout::eUndefined;
	attachmentDescriptions[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	attachmentDescriptions[2].format = vk::Format::eR32G32B32A32Sfloat;
	attachmentDescriptions[2].samples = vk::SampleCountFlagBits::e1;
	attachmentDescriptions[2].loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDescriptions[2].storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDescriptions[2].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachmentDescriptions[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDescriptions[2].initialLayout = vk::ImageLayout::eUndefined;
	attachmentDescriptions[2].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	std::array<vk::AttachmentReference, 2> colorReference = { vk::AttachmentReference(0, ImageLayout::eColorAttachmentOptimal), vk::AttachmentReference(2, ImageLayout::eColorAttachmentOptimal) };

	vk::AttachmentReference depthReference = { 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };

	vk::SubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpassDescription.colorAttachmentCount = 2;
	subpassDescription.pColorAttachments = colorReference.data();
	subpassDescription.pDepthStencilAttachment = &depthReference;

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

	vk::RenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	offscreenTest->renderpass = backend->context.device.createRenderPass(renderPassInfo);
	LOG(INFO) << "Created offscreen renderpass";

	vk::ImageView attachments[3];
	attachments[0] = offscreenTest->colorTexture.view;
	attachments[1] = offscreenTest->depthTexture.view;
	attachments[2] = bloomData->texture1.view;

	vk::FramebufferCreateInfo fbCreateInfo;
	fbCreateInfo.renderPass = offscreenTest->renderpass;
	fbCreateInfo.attachmentCount = 3;
	fbCreateInfo.pAttachments = attachments;
	fbCreateInfo.width = offscreenTest->width;
	fbCreateInfo.height = offscreenTest->height;
	fbCreateInfo.layers = 1;

	offscreenTest->framebuffer = backend->context.device.createFramebuffer(fbCreateInfo);
	LOG(INFO) << "Created offscreen framebuffer";

	offscreenTest->descriptorColorTexture.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	offscreenTest->descriptorColorTexture.imageView = offscreenTest->colorTexture.view;
	offscreenTest->descriptorColorTexture.sampler = offscreenTest->sampler;
	
	bloomData->descriptorTexture1.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	bloomData->descriptorTexture1.imageView = bloomData->texture1.view;
	bloomData->descriptorTexture1.sampler = offscreenTest->sampler;

	bloomData->descriptorTexture2.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	bloomData->descriptorTexture2.imageView = bloomData->texture2.view;
	bloomData->descriptorTexture2.sampler = offscreenTest->sampler;

}

void RendererVulkan::CreateBloomRenderData()
{


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

	SetupCommandPoolAndBuffers();

	// Setup and load the shaders and corresponding pipelines
	SetupShaders();
	CreateOffscreenData();
	SetupPipeline();
	SetupPipelinePostProc();
		
	// Setup samplers
	SetupSamplers();

	// Setup command pool and buffers, afterwards use a buffer to setup Imgui
	SetupIMGUI(iWindow);

	SetupUniformBuffers();

}
