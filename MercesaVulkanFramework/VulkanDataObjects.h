#pragma once

#include "RenderingIncludes.h"
#include "vk_mem_alloc.h"
// A vertex buffer is 
// - A buffer
// - Memory associated with the buffer
// - Input description for the shader
// - The attributes linked with that description


struct BufferVulkan
{
	vk::Buffer buffer;
	VmaAllocation allocation;
};

struct ImageVulkan
{
	vk::Image image;
	VmaAllocation allocation;
};

struct VertexBufferVulkan
{
	std::vector<vk::VertexInputAttributeDescription> inputAttributes;
	vk::VertexInputBindingDescription inputDescription;
	vk::Buffer buffer;
	VmaAllocation allocation;
};

struct ShaderVulkan
{
	std::string entryPointName = "";
	std::string shaderFile = "";
	vk::ShaderStageFlagBits shaderStage;
	vk::ShaderModule shaderModule;

	ShaderVulkan() = default;
	~ShaderVulkan() = default;

};

// A texture in vulkan is made out of..
// - An image (physical object)
// - A view ( GPU based )
// - A chunk of memory allocated from the device
// - Format denoting what kind of texture it is
struct TextureVulkan
{
	vk::Image image;
	vk::ImageView view;
	VmaAllocation allocation;
	vk::Format format;
};

struct ModelVulkan
{
	BufferVulkan indexBuffer;
	VertexBufferVulkan vertexBuffer;
	

	TextureVulkan normalTexture;
	TextureVulkan specularTexture;
	TextureVulkan albedoTexture;
	vk::DescriptorSet albedoTextureSet;


	uint32_t indiceCount = 0;
	uint32_t indiceOffset = 0;

	ModelVulkan() = default;
	~ModelVulkan() = default;
	
};

// A uniform buffer is..
// - A buffer object
// - Memory handle
// - Info for the descriptor
// - Requirements of this uniform buffer
struct UniformBufferVulkan
{
	vk::Buffer buffer;
	vk::DescriptorBufferInfo descriptorInfo;
	VmaAllocation allocation;
};



struct SwapchainVulkan
{
	vk::Format format;
	vk::SurfaceKHR surface;
	vk::SwapchainKHR swapchain;

	std::vector<vk::Image> images;
	std::vector<vk::ImageView> views;
};

struct QueueFamilyIndicesVulkan
{
	
};