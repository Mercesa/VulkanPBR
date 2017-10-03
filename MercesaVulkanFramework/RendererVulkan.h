#pragma once
#include <vector>

class Camera;
class Light;
struct RawMeshData;

class RendererVulkan
{
public:
	RendererVulkan();
	~RendererVulkan();
	
	void Create(std::vector<RawMeshData>& iMeshes);
	void BeginFrame(const Camera& iCamera, const std::vector<Light>& iLights);
	void Render();
	void Destroy();


};

