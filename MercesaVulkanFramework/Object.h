#pragma once

#include "RenderingIncludes.h"
#include "GraphicsStructures.h"

#include "IModel.h"

class Object
{
public:
	Object();
	~Object();

	glm::mat4 modelMatrix;
	uint32_t vulkanModelID = 0;
	RawMeshData rawMeshData;
	bool hasBeenPrepared = false;
	
	iModel* model;
};



