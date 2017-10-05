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
	
	Light light;
	light.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	light.position = glm::vec3(0.0f, 2.0f, 0.0f);
	lights.push_back(light);

	this->modelsToBeLoaded = ModelLoader::LoadModel("Models/Sponza/Sponza.obj", false);

}

void Game::Update()
{

}

void Game::Destroy()
{

}