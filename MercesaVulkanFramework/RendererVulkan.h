#pragma once
#include <vector>
#include <queue>

#include "RenderingIncludes.h"

#include <stdio.h>
 #include <stdlib.h>
// Enable the WSI extensions
 #if defined(__ANDROID__)
 #define VK_USE_PLATFORM_ANDROID_KHR
 #elif defined(__linux__)
 #define VK_USE_PLATFORM_XLIB_KHR
 #elif defined(_WIN32)
 #define VK_USE_PLATFORM_WIN32_KHR
 #endif

 #define NOMINMAX
//#define WIN32_MEAN_AND_LEAN


class NewCamera;
class Light;
struct RawMeshData;

class iModel;
class iTexture;
class iObjectRenderingData;

class Object;
class ResourceManager;
class iLowLevelWindow;
class BackendVulkan;

class ModelVulkan;
class TextureVulkan;
class ObjectRenderingDataVulkan;

#include "GraphicsParameters.h"

class RendererVulkan
{
public:
	RendererVulkan();
	~RendererVulkan();
	
	void Initialize(const GFXParams& iParams, iLowLevelWindow* const iWindow);
	void Resize(const GFXParams& iParams);
	void SetupIMGUI(iLowLevelWindow* const iIlowLevelWindow);
	
	void PrepareResources(
		std::queue<iTexture*> iTexturesToPrepare,
		std::queue<iModel*> iModelsToPrepare,
		std::queue<iObjectRenderingData*> iObjsToPrepare,
		const std::vector<Object>& iObjects);

	void BeginFrame(const NewCamera& iCamera, const std::vector<Light>& iLights);
	void Render(const std::vector<Object>& iObjects);
	void Destroy();

	uint32_t currentBuffer = 0;

private:
	std::unique_ptr<BackendVulkan> backend;

	// Models, textures and object rendering data
	std::vector<ModelVulkan*> models;
	std::vector<TextureVulkan*> textures;
	std::vector<ObjectRenderingDataVulkan*> objRenderingData;
};

