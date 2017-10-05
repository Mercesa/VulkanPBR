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
	
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; i < 4; ++j)
		{
			Light light;
			light.diffuseColor = glm::vec3(i/4.0f, 1.0f, j/4.0f);
			light.position = glm::vec3(i*2.0f, j*2.0f, 0.0f);
			lights.push_back(light);
		}
	}


	this->modelsToBeLoaded = ModelLoader::LoadModel("Models/Sponza/Sponza.obj", false);

}

void Game::Update()
{

}

void Game::Destroy()
{

}