#pragma once

#include <iostream>



enum class AssetTypes { eTexture, eModel, eSound, eUnkown };

struct AssetInformation
{
	std::string filePath;
	AssetTypes assetType;
};

class iResourceManager
{
public:
	virtual ~iResourceManager() = default;

	//virtual void LoadAsset(const AssetInformation& assetInfo) = 0;

	virtual int32_t LoadTexture(const std::string& iFilePath) = 0;
	virtual int32_t LoadModel(const std::string& iFilePath) = 0;

};

