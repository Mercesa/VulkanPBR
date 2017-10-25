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


#include "GraphicsParameters.h"
#include "VulkanDataObjects.h"
#include "ConstantBuffers.h"

struct ContextResources
{
	std::unique_ptr<CommandpoolVulkan> pool;
	vk::CommandBuffer imguiBuffer;
	vk::CommandBuffer baseBuffer;

	UniformBufferVulkan uniformBufferMVP;
	UniformBufferVulkan uniformBufferModelMatrix;
	UniformBufferVulkan uniformBufferLights;
};

struct imguiData
{
	vk::RenderPass renderpass;
	vk::DescriptorPool descriptorPool;
	std::vector<vk::Framebuffer> framebuffer;
};

struct ShaderResourcesPBR
{
	vk::DescriptorSet samplerSet;
	vk::DescriptorSet perFrameUniformBufferSet;
	vk::DescriptorSet perObjectUniformBufferSet;
	vk::DescriptorSet textureSet;
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

	// Initialization functions
	void SetupShaders();
	void SetupPipeline();
	void SetupIMGUI(iLowLevelWindow* const iIlowLevelWindow);
	
	void SetupSamplers();
	void SetupCommandPoolAndBuffers();

	void InitViewports(const vk::CommandBuffer& iBuffer);
	void InitScissors(const vk::CommandBuffer& iBuffer);

	// Render functions
	void SetupCommandBuffersImgui();

	void UpdateUniformBufferFrame(const NewCamera& iCam, const std::vector<Light>& iLights);


	std::unique_ptr<BackendVulkan> backend;

	// Models, textures and object rendering data
	std::vector<ModelVulkan*> models;
	std::vector<TextureVulkan*> textures;
	std::vector<ObjectRenderingDataVulkan*> objRenderingData;
	
	// Resources for every frame
	std::vector<std::unique_ptr<ContextResources>> contextResources;

	// Pipeline layout and pipelines and the corresponding shader programs
	vk::PipelineLayout pipelineLayoutRenderScene;
	vk::Pipeline pipelinePBR;
	vk::Pipeline pipelineRed;

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
};

