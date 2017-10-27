#include "ShaderProgramVulkan.h"

#include "easylogging++.h"
#include "VulkanDataObjects.h"


#include "DescriptorLayoutHelper.h"


inline ShaderDataVulkan CreateShader(
	const vk::Device& iDevice,
	const std::string& iFilePath,
	const std::string& iEntryPoint,
	vk::ShaderStageFlagBits iShaderStage)
{
	auto code = readFile(iFilePath);

	// Ensure our code is not zero
	assert(code.size() > 0);

	ShaderDataVulkan tShader = ShaderDataVulkan();

	vk::ShaderModuleCreateInfo moduleInfo = vk::ShaderModuleCreateInfo()
		.setCodeSize(code.size())
		.setPCode(reinterpret_cast<const uint32_t*>(code.data()));

	tShader.shaderModule = iDevice.createShaderModule(moduleInfo);

	tShader.entryPointName = iEntryPoint;
	tShader.shaderFile = iFilePath;
	tShader.shaderStage = iShaderStage;

	return tShader;
}

ShaderProgramVulkan::ShaderProgramVulkan()
{
}

ShaderProgramVulkan::~ShaderProgramVulkan()
{
}

bool ShaderProgramVulkan::LoadShaders(const vk::Device& iDevice, std::vector<ShaderDataVulkan> iShadersToLoad)
{
	if (iShadersToLoad.size() == 0)
	{
		LOG(ERROR) << "ShaderProgramVulkan::LoadShaders Attempting to load 0 shaders";
		return false;
	}

	for (auto& e : iShadersToLoad)
	{
		shadersData.push_back(CreateShader(iDevice, e.shaderFile, e.entryPointName, e.shaderStage));
	}
	
	// For now I will only support vertex and fragment shader layout parsing, 
	shaderLayout = std::move(ShaderLayoutparser::CompileShadersIntoLayouts(shadersData[0].shaderFile, shadersData[1].shaderFile, iDevice));
	
	
	return true;
}

std::vector<vk::PipelineShaderStageCreateInfo> ShaderProgramVulkan::GetPipelineShaderInfo()
{
	// We need to output a warning when there are no shaders in the program yet
	if (shadersData.size() == 0)
	{
		LOG(WARNING) << "ShaderVulkan::GetPipelineShaderInfo no shaders present in the program yet! returning empty vector";
		return std::vector<vk::PipelineShaderStageCreateInfo>();
	}

	std::vector<vk::PipelineShaderStageCreateInfo> tPipelineShadersInfo;

	for (auto& e : shadersData)
	{
		vk::PipelineShaderStageCreateInfo tShaderPipelineInfo = vk::PipelineShaderStageCreateInfo()
			.setStage(e.shaderStage)
			.setPName(e.entryPointName.c_str())
			.setModule(e.shaderModule)
			.setPNext(nullptr);

		tPipelineShadersInfo.push_back(tShaderPipelineInfo);
	}

	return tPipelineShadersInfo;
}

std::vector<vk::DescriptorSetLayout> ShaderProgramVulkan::GetShaderProgramLayout()
{
	return shaderLayout;
}

void ShaderProgramVulkan::Destroy(const vk::Device& iDevice)
{
	// Destroy shader modules and set layouts
	for (auto& e: shadersData)
	{
		iDevice.destroyShaderModule(e.shaderModule);
	}

	for (auto& e : shaderLayout)
	{
		iDevice.destroyDescriptorSetLayout(e);
	}

	// Clear and reset our data
	shadersData.clear();
	shadersData.resize(0);

	shaderLayout.clear();
	shaderLayout.resize(0);
}