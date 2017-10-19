#pragma once
#include <vector>
#include <queue>

class NewCamera;
class Light;
struct RawMeshData;

class iModel;
class iTexture;
class iObjectRenderingData;

class Object;
class ResourceManager;
class iLowLevelWindow;
class RendererVulkan
{
public:
	RendererVulkan();
	~RendererVulkan();
	

	void SetupIMGUI(iLowLevelWindow* const iIlowLevelWindow);
	void Create(
		std::vector<Object>& iMeshes, 
		ResourceManager* const iResourceManager,
		iLowLevelWindow* const iIlowLevelWindow);
	
	void PrepareResources(
		std::queue<iTexture*> iTexturesToPrepare,
		std::queue<iModel*> iModelsToPrepare,
		std::queue<iObjectRenderingData*> iObjsToPrepare,
		std::vector<Object> iObjects);

	void BeginFrame(const NewCamera& iCamera, const std::vector<Light>& iLights);
	void Render(const std::vector<Object>& iObjects);
	void Destroy();

	uint32_t currentBuffer = 0;

};

