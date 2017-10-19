#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <queue>

class iTexture;
class iModel;
class iObjectRenderingData;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();


	iTexture*const LoadTexture(const std::string& iFilepath);
	std::vector<iModel*> const LoadModel(const std::string& iFilepath);
	iObjectRenderingData* const RegisterRenderObject();

	std::map<std::string, std::unique_ptr<iTexture>> textureMap;
	std::map<std::string, std::vector<std::unique_ptr<iModel>>> modelMap;
	std::vector<std::unique_ptr<iObjectRenderingData>> renderingData;

	// Queues for the renderer, the renderer can work through these and pop them out
	std::queue<iTexture*> texturesToPrepare;
	std::queue<iModel*> modelsToPrepare;
	std::queue<iObjectRenderingData*> objsToPrepare;

private:

};


