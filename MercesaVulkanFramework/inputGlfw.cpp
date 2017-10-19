#include "inputGlfw.h"

#include "GLFW/glfw3.h"
#include "easylogging++.h"

inputGlfw::inputGlfw()
{
}


inputGlfw::~inputGlfw()
{
}

void MousebuttonInput(int key, int scancode, int action, int mods)
{

}

void inputGlfw::Initialize()
{
	for (int i = 0; i < numOfKeys; ++i)
	{
		keyPress[i] = false;
		keyHeld[i] = false;
		keyUp[i] = false;
	}
}

void inputGlfw::Update()
{
	for (int i = 0; i < numOfKeys; ++i)
	{
		keyUp[i] = false;
		keyPress[i] = false;
	}

	relMousePos.x = 0.0f;
	relMousePos.y = 0.0f;

	firstFrame = false;
}

void inputGlfw::KeyboardInput(int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		keyPress[key] = true;
		keyHeld[key] = true;
		break;

	case GLFW_RELEASE:
		keyUp[key] = true;
		keyHeld[key] = false;
		break;

	case GLFW_REPEAT:
		keyHeld[key] = true;
		break;

	default:
		LOG(ERROR) << "InputGLFW::MouseButtonInput invalid operation detected";
		break;
	}
}

bool inputGlfw::GetKeyUp(int32_t key)
{
	return keyUp[key];
}

bool inputGlfw::GetKeyDown(int32_t key)
{
	return keyPress[key];
}

bool inputGlfw::GetKeyHeld(int32_t key)
{
	return keyHeld[key];
}

XYPair inputGlfw::GetCurrentMousePos()
{
	return lastMousePos;
}

XYPair inputGlfw::GetRelMousePos()
{
	return relMousePos;
}

void inputGlfw::MouseMoveInput(double xpos, double ypos)
{
	if (firstFrame)
	{
		lastMousePos.x = xpos;
		lastMousePos.y = ypos;
	}

	relMousePos.x = xpos - lastMousePos.x;
	relMousePos.y = lastMousePos.y - ypos;

	lastMousePos.x = xpos;
	lastMousePos.y = ypos;
}

