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

	vk::Buffer buffer;
	VmaAllocation allocation;
};

struct ShaderDataVulkan
{
	std::string entryPointName = "";
	std::string shaderFile = "";
	vk::ShaderStageFlagBits shaderStage;
	vk::ShaderModule shaderModule;

	ShaderDataVulkan() = default;
	~ShaderDataVulkan() = default;

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