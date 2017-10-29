#pragma once
#include <vector>
#include <queue>

#include "RenderingIncludes.h"

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


class NewCamera;
class Light;
struct RawMeshData;

class iModel;
class iTexture;
class iObjectRenderingData;

class Object;
class ResourceManager;
class iLowLevelWindow;
class BackendVulkan;

class ModelVulkan;
class TextureVulkan;
class ObjectRenderingDataVulkan;

class ShaderProgramVulkan;
class CommandpoolVulkan;
class DescriptorPoolVulkan;

class FramebufferVulkan;
class ModelVulkan;

#include "GraphicsParameters.h"
#include "VulkanDataObjects.h"
#include "ConstantBuffers.h"
#include "TextureVulkan.h"

struct ShaderResourcesPBR
{
	vk::DescriptorSet samplerSet;
	vk::DescriptorSet perFrameUniformBufferSet;
	vk::DescriptorSet perObjectUniformBufferSet;
	vk::DescriptorSet textureSet;
};


struct ShaderResourcesPostProc
{
	vk::DescriptorSet inputTextureSet;
};

struct ContextResources
{
	std::unique_ptr<CommandpoolVulkan> cmdPoolGfx;
	vk::CommandBuffer imguiBuffer;
	vk::CommandBuffer baseBuffer;

	std::unique_ptr<CommandpoolVulkan> cmdPoolCompute;
	vk::CommandBuffer bloomBufferCompute;


	UniformBufferVulkan uniformBufferMVP;
	UniformBufferVulkan uniformBufferModelMatrix;
	UniformBufferVulkan uniformBufferLights;

	ShaderResourcesPBR descriptorSetPBRShader;
	ShaderResourcesPostProc descriptorSetPostProc;

};

struct imguiData
{
	vk::RenderPass renderpass;
	vk::DescriptorPool descriptorPool;
	std::vector<vk::Framebuffer> framebuffer;
};

struct Offscreenpass
{
	int32_t width, height;
	vk::Framebuffer framebuffer;
	TextureData colorTexture, depthTexture;
	vk::RenderPass renderpass;

	vk::Sampler sampler;
	vk::DescriptorImageInfo descriptorColorTexture;

};

struct BloomData
{
	TextureData texture1, texture2;
	vk::RenderPass renderpass;
	vk::Sampler sampler;

	vk::Framebuffer texture1Fb;
	vk::Framebuffer texture2Fb;

	vk::DescriptorImageInfo descriptorTexture1;
	vk::DescriptorImageInfo descriptorTexture2;

};


class RendererVulkan
{
public:
	RendererVulkan();
	~RendererVulkan();
	
	void Initialize(const GFXParams& iParams, iLowLevelWindow* const iWindow);

	void Resize(const GFXParams& iParams);
	
	void PrepareResources(
		std::queue<iTexture*> iTexturesToPrepare,
		std::queue<iModel*> iModelsToPrepare,
		std::queue<iObjectRenderingData*> iObjsToPrepare,
		const std::vector<Object>& iObjects);

	void BeginFrame(const NewCamera& iCamera, const std::vector<Light>& iLights);
	void Render(const std::vector<Object>& iObjects);
	void Destroy();

private:
	void SetupUniformBuffers();

	// Setup functions
	void SetupShaders();
	void SetupPipeline();
	void SetupPipelinePostProc();
	void SetupIMGUI(iLowLevelWindow* const iIlowLevelWindow);
	void SetupSamplers();
	void SetupCommandPoolAndBuffers();
	void SetupDescriptorSet(const std::vector<Object>& iObjects);
	void CreateOffscreenData();
	void CreateBloomRenderData();

	void InitViewports(const vk::CommandBuffer& iBuffer);
	void InitScissors(const vk::CommandBuffer& iBuffer);

	// Render functions
	void RecordCommandBuffersImgui();

	void UpdateUniformBufferFrame(const NewCamera& iCam, const std::vector<Light>& iLights);

	void RenderObjsToBuffer(const vk::CommandBuffer& iBuffer, uint32_t index, const std::vector<Object>& iObjects);


private:

	std::unique_ptr<BackendVulkan> backend;

	// Models, textures and object rendering data
	std::vector<ModelVulkan*> models;
	std::vector<TextureVulkan*> textures;
	std::vector<ObjectRenderingDataVulkan*> objRenderingData;
	
	// Resources for every frame
	std::vector<std::unique_ptr<ContextResources>> contextResources;

	// Pipeline layout and pipelines and the corresponding shader programs
	vk::PipelineLayout pipelineLayoutRenderScene;
	vk::Pipeline pipelineRenderScenePBR;
	vk::Pipeline pipelineRenderSceneRed;

	vk::PipelineLayout pipelineLayoutPostProc;
	vk::Pipeline pipelinePostProc;
	

	std::unique_ptr<ShaderProgramVulkan> shaderProgramPBR;
	std::unique_ptr<ShaderProgramVulkan> shaderProgramRed;
	std::unique_ptr<ShaderProgramVulkan> shaderProgramPostProc;


	// Viewport and scissor rect
	vk::Viewport viewPort;
	vk::Rect2D scissor;

	vk::Sampler samplerLinearRepeat;

	imguiData imguiDataObj;

	// Data containers used to transfer data to
	CBMatrix matrixConstantBufferData;
	CBLights lightConstantBufferData;
	CBModelMatrixSingle matrixSingleData;


	//  these bindings will be useful in the future, they do nothing now
	// But they should be inputted to the descriptor pool for resource management
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	std::vector<vk::DescriptorSetLayoutBinding> uniformBinding;
	std::vector<vk::DescriptorSetLayoutBinding> textureBinding;
	std::vector<vk::DescriptorSetLayoutBinding> postProcBinding;

	// Descriptor resource management
	std::unique_ptr<DescriptorPoolVulkan> descriptorPool;

	std::unique_ptr<Offscreenpass> offscreenTest;
	std::unique_ptr<BloomData> bloomData;

	std::unique_ptr<ModelVulkan> quadModel;

};

