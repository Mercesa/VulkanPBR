
#include <string.h>
#include <vector>
#include <cinttypes>
#include <map>
#include <future>
#include <thread>

#include "easylogging++.h"
#include "libs/Spir-v cross/spirv_glsl.hpp"
#include "RenderingIncludes.h"
#include "Helper.h"

using namespace vk;

struct descriptorLayoutIntermediate
{
	std::string name = "";
	uint32_t binding = 0;
	uint32_t set = 0;
	uint32_t count = 0;
	vk::DescriptorType descType;
	vk::ShaderStageFlags shaderStage;
};

class ShaderLayoutparser
{
public:
	// Parse the resources
	static std::vector<descriptorLayoutIntermediate> ParseResources(
		const std::vector<spirv_cross::Resource>& iResources,
		const spirv_cross::Compiler& iCompiler,
		DescriptorType iDescriptorType,
		ShaderStageFlagBits iTypeOfShader)
	{
		std::vector<descriptorLayoutIntermediate> tIntermediates;
		for (auto& e : iResources)
		{
			descriptorLayoutIntermediate tIntermediate;

			tIntermediate.binding = iCompiler.get_decoration(e.id, spv::DecorationBinding);
			tIntermediate.set = iCompiler.get_decoration(e.id, spv::DecorationDescriptorSet);
			tIntermediate.name = e.name;
			tIntermediate.descType = iDescriptorType;
			tIntermediate.shaderStage = iTypeOfShader;

			// Find out if our descriptor set is an array or not
			const spirv_cross::SPIRType& type = iCompiler.get_type(e.type_id);

			unsigned tArraySize = 0;

			// If it is no array, only one descriptor present
			if (type.array.empty())
			{
				tArraySize = 1;
			}
			else
			{
				tArraySize = type.array[0];
			}
			tIntermediate.count = tArraySize;

			tIntermediates.push_back(tIntermediate);
		}
		return tIntermediates;
	}

	// Compile the shader layout per shader
	static std::vector<descriptorLayoutIntermediate> CompileShaderLayout(
		const std::string& iShader, ShaderStageFlagBits iTypeOfShader)
	{
		std::vector<uint32_t> byteCode = readFileInt(iShader);
		spirv_cross::Compiler compilerShader(std::move(byteCode));
		spirv_cross::ShaderResources resourcesShader = compilerShader.get_shader_resources();

		std::vector<descriptorLayoutIntermediate> intermediateLayout;

		// Get all the resources
		std::vector<descriptorLayoutIntermediate> sampledImagesIntermediate
			= ParseResources(resourcesShader.separate_images, compilerShader, DescriptorType::eSampledImage, iTypeOfShader);

		std::vector<descriptorLayoutIntermediate> samplersIntermediate
			= ParseResources(resourcesShader.separate_samplers, compilerShader, DescriptorType::eSampler, iTypeOfShader);

		std::vector<descriptorLayoutIntermediate> uniformBuffersIntermediate
			= ParseResources(resourcesShader.uniform_buffers, compilerShader, DescriptorType::eUniformBuffer, iTypeOfShader);

		std::vector<descriptorLayoutIntermediate> combinedImageSamplersIntermediate
			= ParseResources(resourcesShader.sampled_images, compilerShader, DescriptorType::eCombinedImageSampler, iTypeOfShader);

		// Put resources in a single 
		intermediateLayout.insert(intermediateLayout.end(), sampledImagesIntermediate.begin(), sampledImagesIntermediate.end());
		intermediateLayout.insert(intermediateLayout.end(), samplersIntermediate.begin(), samplersIntermediate.end());
		intermediateLayout.insert(intermediateLayout.end(), uniformBuffersIntermediate.begin(), uniformBuffersIntermediate.end());
		intermediateLayout.insert(intermediateLayout.end(), combinedImageSamplersIntermediate.begin(), combinedImageSamplersIntermediate.end());


		return intermediateLayout;
	}

	//Merge the layouts we have into proper descriptor sets
	// This function will always return the layouts of the sets in ordered fashion
	// from set 0 to 1 to 2 etc
	static std::vector<DescriptorSetLayout> MergeLayouts(
		const std::vector<std::vector<descriptorLayoutIntermediate>> iLayouts,
		const vk::Device& iDevice)
	{
		std::vector<descriptorLayoutIntermediate> tIntermediates;

		// go through all elements in the layouts
		// We could potentially have 5/6 shaders, and they all need to be synced up
		for (int i = 0; i < iLayouts.size(); ++i)
		{
			for (auto& existingLayout : iLayouts[i])
			{
				bool unique = true;
				for (auto& finalLayouts : tIntermediates)
				{
					// If in the same set? That is fine, sets can be different types
					// if same bindings, types NEED to be the same, otherwise we have a mismatch
					// if same bindings, count NEEDS to be the same, otherwise we have mismatch
					// if same bindings, and if same count, add the shader stage flag
					if (existingLayout.set == finalLayouts.set)
					{
						// Check if the descriptors match, if they don't we have a mismatch between shaders
						// If there are different bindings, it is unique and we can skip this
						if (existingLayout.binding == finalLayouts.binding)
						{
							if (existingLayout.count != finalLayouts.count || existingLayout.descType != finalLayouts.descType)
							{
								LOG(FATAL) << "MergeLayouts() Sets and bindings of descriptor sets match but the type/count is/are different!";
								break;
							}
							unique = false;
							// Add our shader stage to it
							finalLayouts.shaderStage |= existingLayout.shaderStage;
						}

					}
				}

				if (unique == false)
				{
					continue;
				}
				tIntermediates.push_back(existingLayout);
			}
		}


		// Map our descriptor layouts based on their sets

		std::map<uint32_t, std::vector<descriptorLayoutIntermediate>> sets;
		std::vector<DescriptorSetLayout> finalLayouts;

		// Put our entries in the map by their set value
		for (auto& e : tIntermediates)
		{
			sets[e.set].push_back(e);
		}

		// Now create layouts for the pre-determined sets in the map
		for (auto& e : sets)
		{
			std::vector<DescriptorSetLayoutBinding> tLayoutsBindings;
			for (auto& e2 : e.second)
			{
				vk::DescriptorSetLayoutBinding layoutbinding = vk::DescriptorSetLayoutBinding()
					.setBinding(e2.binding)
					.setDescriptorCount(e2.count)
					.setDescriptorType(e2.descType)
					.setPImmutableSamplers(nullptr)
					.setStageFlags(e2.shaderStage);

				tLayoutsBindings.push_back(layoutbinding);
			}


			vk::DescriptorSetLayoutCreateInfo layout = vk::DescriptorSetLayoutCreateInfo()
				.setPNext(NULL)
				.setBindingCount(static_cast<uint32_t>(tLayoutsBindings.size()))
				.setPBindings(tLayoutsBindings.data());

			finalLayouts.push_back(iDevice.createDescriptorSetLayout(layout));
		}

		return finalLayouts;
	}


	// compile these shaders into one descriptor layout
	static std::vector<vk::DescriptorSetLayout> CompileShadersIntoLayouts(
		const std::string& iVertexShader, const std::string& iFragmentShader, 
		const vk::Device& iDevice)
	{
		std::future<std::vector<descriptorLayoutIntermediate>> futureVertex =
			std::async(CompileShaderLayout, iVertexShader, ShaderStageFlagBits::eVertex);

		std::future<std::vector<descriptorLayoutIntermediate>> futureFragment =
			std::async(CompileShaderLayout, iFragmentShader, ShaderStageFlagBits::eFragment);

		std::vector<descriptorLayoutIntermediate> vertexLayout;
		std::vector<descriptorLayoutIntermediate> fragmentLayout;

		vertexLayout = futureVertex.get();
		fragmentLayout = futureFragment.get();

		std::vector<std::vector<descriptorLayoutIntermediate>> layouts = { std::move(vertexLayout), std::move(fragmentLayout) };

		return MergeLayouts(layouts, iDevice);
	}
};

