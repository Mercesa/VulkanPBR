#pragma once

#include "RenderingIncludes.h"
#include "GraphicsStructures.h"

// Create very standard rasterization state
inline vk::PipelineRasterizationStateCreateInfo CreateStandardRasterizerState()
{
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

	return rs;
}

// Create a generic vertex input state
// CURRENTLY BROKEN !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Should find a more elegant way since as of now the data references a vector which goes out of scope

//inline vk::PipelineVertexInputStateCreateInfo CreateStandardVertexInputState()
//{
//	std::vector<vk::VertexInputAttributeDescription> inputAttributes;
//	vk::VertexInputBindingDescription inputDescription;
//
//
//	inputDescription.binding = 0;
//	inputDescription.inputRate = vk::VertexInputRate::eVertex;
//	inputDescription.stride = sizeof(VertexData);
//
//	// 12 bits 
//	// 8  bits 
//	// 12 bits
//	// 12 bits
//	// 12 bits
//	vk::VertexInputAttributeDescription att1;
//	att1.binding = 0;
//	att1.location = 0;
//	att1.format = vk::Format::eR32G32B32Sfloat;
//
//	vk::VertexInputAttributeDescription att2;
//	att2.binding = 0;
//	att2.location = 1;
//	att2.format = vk::Format::eR32G32Sfloat;
//	att2.offset = 12;
//
//	vk::VertexInputAttributeDescription att3;
//	att3.binding = 0;
//	att3.location = 2;
//	att3.format = vk::Format::eR32G32B32Sfloat;
//	att3.offset = 20;
//
//	vk::VertexInputAttributeDescription att4;
//	att4.binding = 0;
//	att4.location = 3;
//	att4.format = vk::Format::eR32G32B32Sfloat;
//	att4.offset = 32;
//
//	vk::VertexInputAttributeDescription att5;
//	att5.binding = 0;
//	att5.location = 4;
//	att5.format = vk::Format::eR32G32B32Sfloat;
//	att5.offset = 44;
//
//
//	inputAttributes.push_back(att1);
//	inputAttributes.push_back(att2);
//	inputAttributes.push_back(att3);
//	inputAttributes.push_back(att4);
//	inputAttributes.push_back(att5);
//
//	vk::PipelineVertexInputStateCreateInfo vi = vk::PipelineVertexInputStateCreateInfo()
//		.setFlags(PipelineVertexInputStateCreateFlagBits(0))
//		.setPVertexBindingDescriptions(&inputDescription)
//		.setPVertexAttributeDescriptions(inputAttributes.data())
//		.setVertexAttributeDescriptionCount(5)
//		.setVertexBindingDescriptionCount(1);
//
//	return vi;
//}

