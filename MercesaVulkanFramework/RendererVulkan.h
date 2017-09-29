#pragma once
#include <vector>
class Camera;
class Light;

class RendererVulkan
{
public:
	RendererVulkan();
	~RendererVulkan();
	
	void Create();
	void BeginFrame(const Camera& iCamera, const std::vector<Light>& iLights);
	void Render();
	void Destroy();


};

