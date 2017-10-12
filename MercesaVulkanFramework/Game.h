#pragma once

#include <vector>
#include <memory>
#include "GraphicsStructures.h"

#include "glm/gtx/common.hpp"

#include "NewCamera.h"

struct Light;

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
	void Update(float iDT);
	void Destroy();

	std::vector<Light> lights;
	std::unique_ptr<NewCamera> camera;
	std::vector<Object> gameObjects;
};

