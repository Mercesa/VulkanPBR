#pragma once

#include "RenderingIncludes.h"

struct CBMatrix
{
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	glm::mat4 viewProjectMatrix;
	glm::mat4 mvpMatrix;
};