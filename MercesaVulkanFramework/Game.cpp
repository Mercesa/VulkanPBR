#include "Game.h"

#include <string>
#include <memory>

#include "GraphicsStructures.h"
#include "camera.h"
#include "ModelLoader.h"
#include "GLFW/glfw3.h"

Game::Game(inputGlfw* const iInput, ResourceManager* const iResourceManager) :
	input(iInput), resourceManager(iResourceManager)
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
	camera = std::make_unique<NewCamera>();
	camera->setPerspective(45, (float)((float)1280.0f / (float)720.0f), 0.01f, 100.0f);
	camera->setPosition(glm::vec3(0.0f, 0.0f, 10.0));
	camera->rotation.y = 730.0f;


	Light light;
	light.diffuseColor = glm::vec3(10.0f, 10.0f, 10.0f);
	light.position = glm::vec3(0.0f, 0.0f, 4.0f);
	lights.push_back(light);


	// Load first sphere
	auto SphereModel = resourceManager->LoadModel("Models/Sphere/Sphere.obj");
	Material material;

	material.diffuseTexture = resourceManager->LoadTexture("Textures/rustediron2_basecolor.png");
	material.specularTexture = resourceManager->LoadTexture("Textures/rustediron2_metallic.png");
	material.normalTexture = resourceManager->LoadTexture("Textures/rustediron2_normal.png");
	material.roughnessTexture = resourceManager->LoadTexture("Textures/rustediron2_roughness.png");
	material.aoTexture = resourceManager->LoadTexture("");


	Object obj;
	obj.model = SphereModel[0];
	obj.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	obj.material = material;
	obj.renderingData = resourceManager->RegisterRenderObject();

	gameObjects.push_back(obj);

	auto GunModel = resourceManager->LoadModel("Models/Gun/Cerberus_LP.fbx");

	material.diffuseTexture = resourceManager->LoadTexture("Textures/Cerberus_A.tga");
	material.specularTexture = resourceManager->LoadTexture("Textures/Cerberus_M.tga");
	material.normalTexture = resourceManager->LoadTexture("Textures/Cerberus_N.tga");
	material.roughnessTexture = resourceManager->LoadTexture("Textures/Cerberus_R.tga");
	material.aoTexture = resourceManager->LoadTexture("Textures/Cerberus_AO.tga");


	obj.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.04f, 0.04f));


	obj.model = GunModel[0];
	obj.material = material;
	obj.renderingData = resourceManager->RegisterRenderObject();
	gameObjects.push_back(obj);


	// New model
	auto DragonModel = resourceManager->LoadModel("Models/Lucy/Lucy.obj")[0];
	material.diffuseTexture = resourceManager->LoadTexture("Textures/CopperRock/copper-rock1-alb.png");
	material.specularTexture = resourceManager->LoadTexture("Textures/CopperRock/copper-rock1-metal.png");
	material.normalTexture = resourceManager->LoadTexture( "Textures/CopperRock/copper-rock1-normal.png");
	material.roughnessTexture = resourceManager->LoadTexture( "Textures/CopperRock/copper-rock1-rough.png");
	material.aoTexture = resourceManager->LoadTexture("Textures/CopperRock/copper-rock1-ao.png");

	obj.model = DragonModel;
	obj.material = material;
	obj.renderingData = resourceManager->RegisterRenderObject();
	obj.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
	gameObjects.push_back(obj);

	//
	//
	//obj.rawMeshData = DragonModel;
	//gameObjects.push_back(obj);

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

}
#include <iostream>
#include <algorithm>

void Game::Update(float iDT)
{
	static float derp = 0.0f;
	derp += 1.0f * iDT;
	lights[0].position.x = sinf(derp) * 3.0f;

	camera->keys.up = false;
	camera->keys.down = false;
	camera->keys.left = false;
	camera->keys.right = false;

	if (input->GetKeyHeld(GLFW_KEY_W))
	{
		camera->keys.up = true;
	}

	if (input->GetKeyHeld(GLFW_KEY_A))
	{
		camera->keys.left = true;
	}

	if (input->GetKeyHeld(GLFW_KEY_S))
	{
		camera->keys.down = true;
	}

	if (input->GetKeyHeld(GLFW_KEY_D))
	{
		camera->keys.right = true;
	}
	

	camera->rotate(glm::vec3(-input->GetRelMousePos().y , input->GetRelMousePos().x, 0.0f));
	camera->rotationSpeed = 0.5f;
	camera->movementSpeed = 3.0f;
	camera->rotation.x = std::max(camera->rotation.x, 275.0f);
	camera->rotation.x = std::min(camera->rotation.x, 450.0f);

	camera->update(iDT);
	

	//std::cout << camera->rotation.y << std::endl;
	//std::cout << "Light pos: " << lights[0].position.x << std::endl;
}

void Game::Destroy()
{

}