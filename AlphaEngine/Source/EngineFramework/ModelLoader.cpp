#pragma once


#include "ModelLoader.h"
#include "EngineFramework/Logger.h"

namespace AlphaEngine
{
    ModelData AlphaEngine::ModelLoader::LoadModelFromDisk(const std::string& path)
    {
        ModelData data;
        Assimp::Importer importer;

        // Post-processing: 
        // Triangulate: Ensures all faces are triangles
        // FlipUVs: OpenGL expects UVs to start from the bottom-left
        // CalcTangentSpace: Useful if you want Normal Mapping later
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs);

        if (!scene || !scene->mRootNode) {
            Logger::Err("Assimp Error: " + std::string(importer.GetErrorString()));
            return data;
        }

        // Load First mesh 
        // TODO: We should later on have to loop thourgh all meshese
        // for example for a car might have -> A body mesh and 4 wheels or sth

        if (scene->mNumMeshes > 0) {
            aiMesh* mesh = scene->mMeshes[0];

            
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

                Vertex vertex;

                // Positions
                vertex.Position.x = mesh->mVertices[i].x;
                vertex.Position.y = mesh->mVertices[i].y;
                vertex.Position.z = mesh->mVertices[i].z;

                // Normals
                if (mesh->HasNormals()) {
                    vertex.Normal.x = mesh->mNormals[i].x;
                    vertex.Normal.y = mesh->mNormals[i].y;
                    vertex.Normal.z = mesh->mNormals[i].z;
                }
                else {
                    vertex.Normal = { 0.0f, 0.0f, 0.0f };
                }

                // Texctutre Coord
                if (mesh->mTextureCoords[0]) {
                    vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                    vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
                }
                else {
                    vertex.TexCoords = { 0.0f, 0.0f };
                }

                data.vertices.push_back(vertex);

            }

            // Indices
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    data.indices.push_back(face.mIndices[j]);
                }
            }
        }

        return data;
    }
}