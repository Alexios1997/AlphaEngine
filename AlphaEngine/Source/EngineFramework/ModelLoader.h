#pragma once


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include "EngineFrameWork/Mesh.h"

namespace AlphaEngine
{

	struct ModelData {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	class ModelLoader 
	{
	public:
		static ModelData LoadModelFromDisk(const std::string& path);
	};
}