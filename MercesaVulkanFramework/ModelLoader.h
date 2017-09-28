#pragma once

#include <vector>

#include "GraphicsStructures.h"
#include "GenericMathValueStructs.h"

struct aiNode;
struct aiScene;
struct aiMesh;

struct RawMeshData;

struct ModelLoadData
{
	std::string filepath;
	bool convertToDDS;
	bool generateBoundingSphere;
	bool regenerateAssbin;
};

class ModelLoader
{
public:
	static std::vector<RawMeshData> LoadModel(const char* aFilePath, bool aGenerateAABB);
	

private:
	ModelLoader();
	~ModelLoader();

	static void ProcessNode(aiNode* const a_Node, const aiScene* const a_Scene, std::vector<RawMeshData>& aData);
	static RawMeshData ProcessMesh(aiMesh* const a_Mesh, const aiScene* const a_Scene);
	static VEC4f GenerateSphereBound(std::vector<VertexData>& aData);
	// has a vector of meshes that are done processing 
};
