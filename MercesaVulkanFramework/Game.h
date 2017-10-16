#pragma once

#include <vector>
#include <memory>
#include "GraphicsStructures.h"

#include "glm/gtx/common.hpp"

#include "NewCamera.h"
#include "Object.h"
#include "ResourceManager.h"

struct Light;


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
	ResourceManager* resourceManager;
};

