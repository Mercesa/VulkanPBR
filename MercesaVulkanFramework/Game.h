#pragma once

#include <vector>
#include <memory>

class Camera;
class Light;

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
};

