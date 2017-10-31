#pragma once

#include "iTexture.h"

struct Material
{
	iTexture* diffuseTexture;
	iTexture* specularTexture;
	iTexture* normalTexture;
	iTexture* aoTexture;
	iTexture* roughnessTexture;

	float roughness = 0.0f;
	float metalness = 0.0f;
	int useTexturesForReflectivity = 0.0f;
};