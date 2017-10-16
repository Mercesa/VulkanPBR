#pragma once
#include <vector>

class NewCamera;
class Light;
struct RawMeshData;

struct Object;
class ResourceManager;
class RendererVulkan
{
public:
	RendererVulkan();
	~RendererVulkan();
	
	void Create(std::vector<Object>& iMeshes, ResourceManager* const iResourceManager);
	
	void BeginFrame(const NewCamera& iCamera, const std::vector<Light>& iLights);
	void Render(const std::vector<Object>& iObjects);
	void Destroy();


};

