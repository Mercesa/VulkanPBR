#pragma once

#include <cstdint>
#include "GraphicsStructures.h"

class iModel
{
public:
	iModel() = default;
	virtual ~iModel() = default;

	uint32_t GetIndiceCount() { return indiceCount; };
	uint32_t GetIndiceOffset() { return indiceOffset; };


	uint32_t indiceCount = 0;
	uint32_t indiceOffset = 0;


	RawMeshData data;
	bool isPrepared = false;

protected:

};