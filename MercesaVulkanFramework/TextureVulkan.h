#pragma once
#include "iTexture.h"
#include "RenderingIncludes.h"
#include "vk_mem_alloc.h"

struct TextureData
{
	vk::Image image;
	vk::ImageView view;
	VmaAllocation allocation;
	vk::Format format;
};

class TextureVulkan : public iTexture
{
public:
	TextureVulkan(std::string iFilePath);
	virtual ~TextureVulkan();

	TextureData data;
};

