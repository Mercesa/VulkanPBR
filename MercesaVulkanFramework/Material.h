#pragma once

#include "iTexture.h"

struct Material
{
	iTexture* diffuseTexture;
	iTexture* specularTexture;
	iTexture* normalTexture;
	iTexture* aoTexture;
	iTexture* roughnessTexture;

};