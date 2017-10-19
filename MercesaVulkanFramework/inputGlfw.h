#pragma once

#include <array>

static const int32_t numOfKeys = 349;


struct XYPair
{
	double x = 0;
	double y = 0;
};

class inputGlfw
{
public:
	inputGlfw();
	~inputGlfw();

	void Initialize();
	void Update();
	void KeyboardInput(int key, int scancode, int action, int mods);
	void MouseMoveInput(double xpos, double ypos);

	bool GetKeyUp(int32_t key);
	bool GetKeyDown(int32_t key);
	bool GetKeyHeld(int32_t key);

	XYPair GetRelMousePos();
	XYPair GetCurrentMousePos();


private:
	bool firstFrame = true;

	XYPair relMousePos;
	XYPair lastMousePos;

	std::array<bool, numOfKeys> keyPress;
	std::array<bool, numOfKeys> keyHeld;
	std::array<bool, numOfKeys> keyUp;
};

