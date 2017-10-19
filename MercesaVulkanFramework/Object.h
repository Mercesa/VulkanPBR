#pragma once

#include "RenderingIncludes.h"
#include "GraphicsStructures.h"

#include "IModel.h"
#include "material.h"
#include "iObjectRenderingData.h"

class Object
{
public:
	Object();
	~Object();

	glm::mat4 modelMatrix;
	
	iModel* model;
	iObjectRenderingData* renderingData;
	Material material;
};



