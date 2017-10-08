#pragma once
#include "iResourceManager.h"

#include <vector>
#include <queue>
#include <map>
#include <memory>

#include "GraphicsStructures.h"
#include "ModelLoader.h"

#include "stb_image.h"

struct TextureAsset
{
	stbi_uc* pixels;
	int width, height;
	int channels;
	int imageSizeInBytes;

	~TextureAsset()
	{
		stbi_image_free(pixels);
	}
};

class PhysicalResourceManager :
	public iResourceManager
{
public:
	PhysicalResourceManager();
	virtual ~PhysicalResourceManager();

	virtual int32_t LoadTexture(const std::string& iFilePath);
	virtual int32_t LoadModel(const std::string& iFilePath);

	
	// This function will be done on a thread, it will constantly load physical resources
	// As long as there are resources, this thread will become active(
	void ProcessWork();

private:

	// How we implement physical texture loading or model loading 
	// Is not of interest for our Vulkan resource manager
	// This allows for a lot of different ways to process different resources
	TextureAsset LoadPhysicalTexture(const std::string& iFilePath);
	RawMeshData LoadPhysicalModel(const std::string& iFilePath);
	

	std::pair<bool, uint64_t> CheckIfTextureExists(const std::string& iFilePath);
	std::pair<bool, uint64_t> CheckIfModelExists(const std::string& iFilePath);


	std::map<std::string, uint64_t> modelIdentifierDatabase;
	std::map<std::string, uint64_t> textureIdentifierDatabase;

	
	std::queue<std::pair<std::string, uint64_t>> modelsToLoad;
	std::queue<std::pair<std::string, uint64_t>> texturesToLoad;

protected:
	// The resources our renderer resource manager will use and consume
	std::queue <std::pair<uint64_t, RawMeshData>> loadedModels;
	std::queue <std::pair<uint64_t, TextureAsset>> loadedTextures;
};

