#include "ResourceManager.h"

#include "TextureVulkan.h"
#include "ModelVulkan.h"

ResourceManager::ResourceManager()
{
}


ResourceManager::~ResourceManager()
{
}


iTexture* const ResourceManager::LoadTexture(const std::string& iFilepath)
{
	// If element already exists in map, return that texture
	auto it = textureMap.find(iFilepath);
	if (it != textureMap.end())
	{
		return (*it).second.get();
	}

	std::unique_ptr<iTexture> texture = std::make_unique<TextureVulkan>(iFilepath);
	iTexture* returnVal = texture.get();
	textureMap[iFilepath] = std::move(texture);

	return texture.get();
}

iModel* const ResourceManager::LoadModel(const std::string& iFilepath)
{
	return nullptr;
}