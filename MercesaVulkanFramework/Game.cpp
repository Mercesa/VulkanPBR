#include "Game.h"

#include <string>
#include <memory>

#include "GraphicsStructures.h"
#include "camera.h"
#include "ModelLoader.h"

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
	camera = std::make_unique<NewCamera>();
	camera->setPerspective(45, (float)((float)1280.0f / (float)720.0f), 0.01f, 100.0f);
	camera->setPosition(glm::vec3(0.0f, 0.0f, -10.0));



	Light light;
	light.diffuseColor = glm::vec3(10.0f, 10.0f, 10.0f);
	light.position = glm::vec3(0.0f, 0.0f, 4.0f);
	lights.push_back(light);


	// Load first sphere
	auto SphereModel = resourceManager->LoadModel("Models/Sphere/Sphere.obj");
	Material material;

	material.diffuseTexture = resourceManager->LoadTexture("Textures/Cerberus_A.tga");
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
	//auto DragonModel = ModelLoader::LoadModel("Models/Lucy/Lucy.obj", false)[0];
	//DragonModel.filepaths[RawMeshData::eAlbedo] = "Textures/CopperRock/copper-rock1-alb.png";
	//DragonModel.filepaths[RawMeshData::eSpecularMetal] = "Textures/CopperRock/copper-rock1-metal.png";
	//DragonModel.filepaths[RawMeshData::eNormal] = "Textures/CopperRock/copper-rock1-normal.png";
	//DragonModel.filepaths[RawMeshData::eRough] = "Textures/CopperRock/copper-rock1-rough.png";
	//DragonModel.filepaths[RawMeshData::eAO] = "Textures/CopperRock/copper-rock1-ao.png";
	//
	//obj.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
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

void Game::Update(float iDT)
{
	static float derp = 0.0f;
	derp += 1.0f * iDT;
	lights[0].position.x = sinf(derp) * 3.0f;

	//std::cout << "Light pos: " << lights[0].position.x << std::endl;
}

void Game::Destroy()
{

}