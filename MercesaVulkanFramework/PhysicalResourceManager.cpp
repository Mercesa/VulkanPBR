#include "PhysicalResourceManager.h"

#include "stb_image.h"
#include "easylogging++.h"

PhysicalResourceManager::PhysicalResourceManager()
{
}


PhysicalResourceManager::~PhysicalResourceManager()
{
}
std::pair<bool, uint64_t> PhysicalResourceManager::CheckIfTextureExists(const std::string& iFilePath)
{
	auto it = textureIdentifierDatabase.find(iFilePath);
	if (it != textureIdentifierDatabase.end())
	{
		return std::pair<bool, uint64_t>(true, (it->second));
	}

	return{ false, 0 };
}

std::pair<bool, uint64_t> PhysicalResourceManager::CheckIfModelExists(const std::string& iFilePath)
{
	auto it = modelIdentifierDatabase.find(iFilePath);
	if (it != modelIdentifierDatabase.end())
	{
		return std::pair<bool, uint64_t>(true, (it->second));
	}

	return{ false, 0 };
}

int32_t PhysicalResourceManager::LoadTexture(const std::string& iFilePath)
{
	// First perform a pass to see if texture exists
	auto returnVals = CheckIfTextureExists(iFilePath);

	if (returnVals.first == true)
	{
		return returnVals.second;
	}

	// Return an unique ID to the user
	static uint64_t uniqueTextureIdentifier = 0;

	textureIdentifierDatabase.insert({ iFilePath, uniqueTextureIdentifier });
	
	// Temp variable which we want to return
	uint64_t identifierToReturn = uniqueTextureIdentifier;

	// register texture to process on the graphics side of things
	texturesToLoad.push({ iFilePath, identifierToReturn });
	uniqueTextureIdentifier++;

	return identifierToReturn;
}

int32_t PhysicalResourceManager::LoadModel(const std::string& iFilePath)
{
	// First perform a pass to see if texture exists
	auto returnVals = CheckIfModelExists(iFilePath);

	if (returnVals.first == true)
	{
		return returnVals.second;
	}

	// Return an unique ID to the user
	static uint64_t uniqueModelidentifier = 0;

	modelIdentifierDatabase.insert({ iFilePath, uniqueModelidentifier });
	
	// Temp variable which we want to return
	uint64_t identifierToReturn = uniqueModelidentifier;
	
	// register model
	modelsToLoad.push({ iFilePath, identifierToReturn });
	uniqueModelidentifier++;

	return identifierToReturn;
}

TextureAsset PhysicalResourceManager::LoadPhysicalTexture(const std::string& iFilePath)
{
	//// Load texture with stbi
	TextureAsset asset;

	asset.pixels = stbi_load(iFilePath.c_str(), &asset.width, &asset.height, &asset.channels, STBI_rgb_alpha);
	
	asset.imageSizeInBytes = asset.width * asset.height * 4;
	
	if (!asset.pixels)
	{
		stbi_image_free(asset.pixels);
	
		LOG(ERROR) << "Load texture failed! Fallback to error texture..";
		asset.pixels = stbi_load("textures/ErrorTexture.png", &asset.width, &asset.height, &asset.channels, STBI_rgb_alpha);
		asset.imageSizeInBytes = asset.width * asset.height* 4;
		if (!asset.pixels)
		{
			LOG(FATAL) << "FAILED TO LOAD ERRORTEXTURE, THIS SHOULD NOT HAPPEN";
		}
	}
	
	return asset;
}

RawMeshData PhysicalResourceManager::LoadPhysicalModel(const std::string& iFilePath)
{
	return ModelLoader::LoadModel(iFilePath.c_str(), false)[0];
}

void PhysicalResourceManager::ProcessWork()
{
	// Go through all the data
	for (int i = 0; i < modelsToLoad.size(); ++i)
	{
		auto& model = modelsToLoad.front();
		RawMeshData modeldata = LoadPhysicalModel(model.first);
		modelsToLoad.pop();

		loadedModels.push({ model.second, modeldata });
	}

	for (int i = 0; i < texturesToLoad.size(); ++i)
	{
		auto& textures = texturesToLoad.front();
		TextureAsset textureData = LoadPhysicalTexture(textures.first);
		texturesToLoad.pop();

		loadedTextures.push({ textures.second, textureData });
	}
}