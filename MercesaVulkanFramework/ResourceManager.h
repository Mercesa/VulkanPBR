#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

class iTexture;
class iModel;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();


	iTexture*const LoadTexture(const std::string& iFilepath);
	std::vector<iModel*> const LoadModel(const std::string& iFilepath);


	std::map<std::string, std::unique_ptr<iTexture>> textureMap;
	std::map<std::string, std::vector<std::unique_ptr<iModel>>> modelMap;

private:

};


