#pragma once

#include "RenderingIncludes.h"
#include "GraphicsStructures.h"

struct CBMatrix
{
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	glm::mat4 viewProjectMatrix;
	glm::mat4 mvpMatrix;

	glm::vec3 viewPos;
};

struct CBLights
{
	Light lights[16];
	uint32_t currAmountOfLights;
};

struct CBModelMatrix {
	glm::mat4 *model = nullptr;
};

struct CBMaterialPBR{
	float roughness = 0.0f;
	float metallicness = 0.0f;
	int use = 0;
};

struct CBModelMatrixSingle {
	glm::mat4 model;
};