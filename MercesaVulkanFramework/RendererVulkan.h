#pragma once

class Camera;

class RendererVulkan
{
public:
	RendererVulkan();
	~RendererVulkan();
	
	void Create();
	void BeginFrame(const Camera& iCamera);
	void Render();
	void Destroy();
};

