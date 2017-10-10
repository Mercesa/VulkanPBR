#include "Game.h"

#include <string>
#include <memory>

#include "GraphicsStructures.h"
#include "camera.h"
#include "ModelLoader.h"
#include "PhysicalResourceManager.h"

Game::Game()
{
}


Game::~Game()
{
}

void Game::Init()
{

	//std::unique_ptr<PhysicalResourceManager> resourceManager = std::make_unique<PhysicalResourceManager>();


	//for (int i = 0; i < 100; ++i)
	//{
	//	std::cout << resourceManager->LoadModel(std::to_string(i).append(".")) << std::endl;
	//	resourceManager->ProcessWork();
	//}
	camera = std::make_unique<Camera>();
	
	//for (int i = 0; i < 4; ++i)
	//{
	//	for (int j = 0; j < 4; ++j)
	//	{
	////		Light light;
	////		light.diffuseColor = glm::vec3(i%2, (i+j)%2, j%2);
	////		light.position = glm::vec3(i*8.0f-4, 2.0f, j*3.0f);
	////		lights.push_back(light);
	//	}
	//}

	Light light;
	light.diffuseColor = glm::vec3(10.0f, 10.0f, 10.0f);
	light.position = glm::vec3(0.0f, 0.0f, -1.0f);
	lights.push_back(light);

	this->modelsToBeLoaded = ModelLoader::LoadModel("Models/Sphere/Sphere.obj", false);
	modelsToBeLoaded[0].filepaths[0] = "Textures/rustediron2_basecolor.png";
	modelsToBeLoaded[0].filepaths[1] = "Textures/rustediron2_metallic.psd";
	modelsToBeLoaded[0].filepaths[2] = "Textures/rustediron2_normal.png";
	modelsToBeLoaded[0].filepaths[3] = "Textures/rustediron2_roughness.png";
	modelsToBeLoaded[0].filepaths[4] = "";

	//this->modelsToBeLoaded = ModelLoader::LoadModel("Models/Sphere/Sphere.obj", false);
	//modelsToBeLoaded[0].filepaths[0] = "Textures/CopperRock/copper-rock1-alb.png";
	//modelsToBeLoaded[0].filepaths[1] = "Textures/CopperRock/copper-rock1-metal.png";
	//modelsToBeLoaded[0].filepaths[2] = "Textures/CopperRock/copper-rock1-normal.png";
	//modelsToBeLoaded[0].filepaths[3] = "Textures/CopperRock/copper-rock1-rough.png";
	//modelsToBeLoaded[0].filepaths[4] = "Textures/CopperRock/copper-rock1-ao.png";


	// Oakfloor
	//this->modelsToBeLoaded = ModelLoader::LoadModel("Models/Sphere/Sphere.obj", false);
	//modelsToBeLoaded[0].filepaths[0] = "Textures/Floor/oakfloor_basecolor.png";
	//modelsToBeLoaded[0].filepaths[1] = "";
	//modelsToBeLoaded[0].filepaths[2] = "Textures/Floor/oakfloor_normal.png";
	//modelsToBeLoaded[0].filepaths[3] = "Textures/Floor/oakfloor_roughness.png";
	//modelsToBeLoaded[0].filepaths[4] = "Textures/Floor/oakfloor_AO.png";

	for (auto& e : modelsToBeLoaded)
	{
		Object obj;
		obj.rawMeshData = e;
		gameObjects.push_back(obj);

	}

}

void Game::Update()
{
	static float derp = 0.0f;
	derp += 0.1f;
	lights[0].position.y = sinf(derp);
}

void Game::Destroy()
{

}