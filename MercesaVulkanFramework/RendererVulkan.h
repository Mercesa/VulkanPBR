#pragma once
#include <vector>

class NewCamera;
class Light;
struct RawMeshData;

class RendererVulkan
{
public:
	RendererVulkan();
	~RendererVulkan();
	
	void Create(std::vector<RawMeshData>& iMeshes);
	void BeginFrame(const NewCamera& iCamera, const std::vector<Light>& iLights);
	void Render();
	void Destroy();


};

