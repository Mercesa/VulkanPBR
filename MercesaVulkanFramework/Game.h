#pragma once

#include <vector>
#include <memory>
#include "GraphicsStructures.h"

class Camera;
class Light;

class Game
{
public:
	Game();
	~Game();
	
	void Init();
	void Update();
	void Destroy();

	std::vector<Light> lights;
	std::unique_ptr<Camera> camera;
	std::vector<RawMeshData> modelsToBeLoaded;
};

