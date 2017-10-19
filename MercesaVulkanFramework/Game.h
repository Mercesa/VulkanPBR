#pragma once

#include <vector>
#include <memory>
#include "GraphicsStructures.h"

#include "glm/gtx/common.hpp"

#include "NewCamera.h"
#include "Object.h"
#include "ResourceManager.h"
#include "inputGlfw.h"

struct Light;


class Game
{
public:
	Game(inputGlfw* const iInput, ResourceManager* const iResourceManager);
	~Game();
	
	void Init();
	void Update(float iDT);
	void Destroy();

	std::vector<Light> lights;
	std::unique_ptr<NewCamera> camera;
	std::vector<Object> gameObjects;
	ResourceManager* const resourceManager;
	inputGlfw* const input;
	
};

