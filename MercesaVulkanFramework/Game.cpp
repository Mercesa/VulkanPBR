#include "Game.h"

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
	light.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	light.position = glm::vec3(2.0f, 2.0f, 2.0f);
	lights.push_back(light);

	this->modelsToBeLoaded = ModelLoader::LoadModel("Models/Sphere/Sphere.obj", false);
	//modelsToBeLoaded[0].filepaths[0] = "Textures/rustediron2_basecolor.png";
	modelsToBeLoaded[0].filepaths[0] = "rustediron2_metallic";

}

void Game::Update()
{

}

void Game::Destroy()
{

}