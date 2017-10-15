#pragma once

#include <string>
#include <map>
#include <memory>

class iTexture;
class iModel;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();


	iTexture*const LoadTexture(const std::string& iFilepath);
	iModel*const LoadModel(const std::string& iFilepath);


private:
	std::map<std::string, std::unique_ptr<iTexture>> textureMap;
	std::map<std::string, std::unique_ptr<iModel>> modelMap;

};


