#pragma once


#include <vector>

#include "GenericMathValueStructs.h"
#include <glm/common.hpp>

enum eLightTypes
{
	ePOINT, eDIRECTIONAL, eSPOT
};

struct Light
{
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	float pad0;

	glm::vec3 diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	float pad1;

	glm::vec3 specularColor = glm::vec3(1.0f, 0.0f, 1.0f);
	int typeOfLight;
};


struct VertexData
{
	glm::vec3 position;
	glm::vec2 texCoords;
	glm::vec3 normal;
	glm::vec3 bitangent;
	glm::vec3 tangent;
};


struct RawTextureData
{
	std::string filepath;
	bool isValid;
};


struct RawMeshData
{
	// Vertices, indices
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;


	enum FilepathIndices { eAlbedo = 0, eSpecularMetal = 1, eNormal = 2, eRough = 3, eAO = 4 };
	std::string filepaths[5] = {""};
	bool isValids[5] = { false };


	VEC4f sphericalCollider;
};

