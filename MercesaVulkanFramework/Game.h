#pragma once

#include <vector>
#include <memory>
#include "GraphicsStructures.h"

#include "glm/gtx/common.hpp"
class Camera;
class Light;

struct Object
{
	glm::mat4 modelMatrix;
	uint32_t vulkanModelID = 0;
	RawMeshData rawMeshData;
};

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
	std::vector<Object> gameObjects;
};

