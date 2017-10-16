#include "ResourceManager.h"

#include "easylogging++.h"

#include "TextureVulkan.h"
#include "ModelVulkan.h"
#include "ModelLoader.h"

#include "iModel.h"

#include "ModelLoader.h"

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
		LOG(INFO) << "ResourceManager::LoadTexture Found match, returning copy";
		return (*it).second.get();
	}

	std::unique_ptr<iTexture> texture = std::make_unique<TextureVulkan>(iFilepath);
	iTexture* returnVal = texture.get();
	textureMap[iFilepath] = std::move(texture);

	return returnVal;
}

std::vector<iModel*> const ResourceManager::LoadModel(const std::string& iFilepath)
{
	auto it = modelMap.find(iFilepath);

	if (it != modelMap.end())
	{
		LOG(INFO) << "ResourceManager::LoadModel Found match, returning copy";
		
		std::vector<iModel*> modelsToReturn;

		modelsToReturn.reserve((*it).second.size());
		
		// Fill in the models to return with the models from the model vector
		for (auto& e : (*it).second)
		{
			modelsToReturn.push_back(e.get());
		}

		return modelsToReturn;
	}

	// Load model
	auto models = ModelLoader::LoadModel(iFilepath.c_str(), false);

	// Vectors for our models to store and return
	std::vector<std::unique_ptr<iModel>> tModelsToStore;
	std::vector<iModel*> tModelsToReturn;

	// Reserve exactly the data we need, as to prevent constant reallocation
	tModelsToStore.reserve(models.size());
	tModelsToReturn.reserve(models.size());
	
	for (auto& e : models)
	{
		std::unique_ptr<iModel> tModel = std::make_unique<ModelVulkan>();
		tModel->data = e;

		tModelsToReturn.push_back(tModel.get());
		tModelsToStore.push_back(std::move(tModel));

	}

	// Store our vector of unique ptrs to the resource map
	modelMap[iFilepath] = std::move(tModelsToStore);

	// return these models
	return tModelsToReturn;
}