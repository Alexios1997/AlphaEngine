#pragma once

#include "AssetManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "EngineFramework/FileSystem.h"
#include "EngineFramework/ModelLoader.h"
#include "EngineFramework/Logger.h"
#include <glad/gl.h>


namespace AlphaEngine
{

	void TextureJob::Execute() {
		// Now the compiler knows GLTextureUpload exists!
		manager->GLTextureUpload(id, data, width, height);
		stbi_image_free(data);
	}

	void MeshJob::Execute() {
		manager->GLMeshUpload(id, vertices, indices);
	}

	void ShaderJob::Execute() {
		manager->GLShaderUpload(id, vertexSource, fragmentSource, path);
	}

	void CubeMapJob::Execute()
	{
		manager->GLCubeMapUpload(id, faceData, width, height);
	}

	AssetManager::AssetManager()
	{
		// Inside AssetManager Constructor, we set the default loading texture
		unsigned char pixels[] = { 255, 0, 255, 255 }; // Pink
		glGenTextures(1, &m_DefaultTextureID);
		glBindTexture(GL_TEXTURE_2D, m_DefaultTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// Loads the texture from disk to CPU RAM through a thread obvisously
	AssetHandler AlphaEngine::AssetManager::LoadTexture(const std::string& path)
	{
		AssetID id = HashGivenPath(path.c_str());

		// Check if it is already in our Texture Lib
		if (m_TexturesLibrary.find(id) != m_TexturesLibrary.end()) {
			return { id };
		}

		m_TexturesLibrary[id] = std::make_unique<TextureAsset>();
		m_TexturesLibrary[id]->isLoading = true;

		// Starting a background thread that runs in parallel with our game
		// A lambda function
		// In general we want the Back Thread to get in and out of the queue as fast as possible
		// But safely always by using {} and locking it just for the duration of the push
		std::thread([this, id, path]() {
			int w, h, channels;

			std::string fullPath = FileSystem::GetPath(path);
			// stbi_load in general it is slow but running inside our thread is gonna be ok.
			unsigned char* data = stbi_load(fullPath.c_str(), &w, &h, &channels, 4);

			if (data) {
				auto job = std::make_unique<TextureJob>();
				job->id = id;
				job->data = data;
				job->width = w;
				job->height = h;
				job->manager = this;

				// We put the job in queue and we lock so that the main thread does not take the texture accidentlty 
				// Plius making sure that only one thread at a time is even looking at this queue So that the Main thread
				// wont read any broken pointers.
				// Lock and Push
				{
					// Thread "grabs the key"
					std::lock_guard<std::mutex> lock(m_QueueMutex);
					// Thread "opens the vault"
					m_UploadQueueJobs.push(std::move(job));
				} // Here it unlocks it agaain
			}
			else {
				Logger::Err("Failed to load texture file at: " + fullPath);
			}
			}).detach();
		// . detatch makes sure that the thread runs independently.

		return { id };
	}

	// Loads the Shader from Disk to CPU RAM
	AssetHandler AssetManager::LoadShader(const std::string& path)
	{
		AssetID id = HashGivenPath(path.c_str());

		if (m_ShaderLibrary.find(id) != m_ShaderLibrary.end()) {
			return { id };
		}

		m_ShaderLibrary[id] = nullptr;

		std::thread([this, id, path]() {

			std::string fullPath = FileSystem::GetPath(path);
			std::string source = Shader::ReadFile(fullPath);

			if (source.empty()) return;

			ShaderProgramSource shaderSource = Shader::ParseShader(source);

			// Create and Push the Job
			auto job = std::make_unique<ShaderJob>();
			job->id = id;
			job->vertexSource = std::move(shaderSource.VertexSource);
			job->fragmentSource = std::move(shaderSource.FragmentSource);
			job->path = path;
			job->manager = this;


			{
				std::lock_guard<std::mutex> lock(m_QueueMutex);
				m_UploadQueueJobs.push(std::move(job));
			}

			}).detach();

		return { id };
	}

	// Loads the Model from Disk to CPU RAM
	AssetHandler AssetManager::LoadMesh(const std::string& path, bool keepDataToCpu)
	{
		AssetID id = HashGivenPath(path.c_str());

		if (m_MeshesLibrary.find(id) != m_MeshesLibrary.end()) {
			return { id };
		}

		m_MeshesLibrary[id] = std::make_unique<MeshAsset>();
		m_MeshesLibrary[id]->isLoading = true;
		m_MeshesLibrary[id]->isReady = false;
		m_MeshesLibrary[id]->mesh = std::make_unique<Mesh>();
		m_MeshesLibrary[id]->mesh->KeepCpuData(keepDataToCpu);

		std::thread([this, id, path]() {

			// Using Model loader, in which Assimp loads the Given Model
			std::string fullPath = FileSystem::GetPath(path);
			ModelData importedModelData = ModelLoader::LoadModelFromDisk(fullPath);

			if (!importedModelData.vertices.empty()) {
				

				auto job = std::make_unique<MeshJob>();
				job->id = id;
				job->vertices = std::move(importedModelData.vertices);
				job->indices = std::move(importedModelData.indices);
				job->manager = this;

				// Lock and Push
				std::lock_guard<std::mutex> lock(m_QueueMutex);
				m_UploadQueueJobs.push(std::move(job));
			}
			else {
				Logger::Err("Mesh data was empty for path: " + path);
				m_MeshesLibrary[id]->isLoading = false;
			}
			}).detach();

		return { id };
	}

	// Loads the CubeMap Textures from Disk to CPU RAM
	AssetHandler AssetManager::LoadCubeMap(const std::vector<std::string> skyboxFacespath)
	{
		AssetID id = HashGivenPath(skyboxFacespath[0].c_str());

		if (m_TexturesLibrary.find(id) != m_TexturesLibrary.end()) return { id };;

		m_TexturesLibrary[id] = std::make_unique<TextureAsset>();
		m_TexturesLibrary[id]->isLoading = true;

		std::thread([this, id, skyboxFacespath]() {
			unsigned char* facesData[6];
			int w, h, channels;
			bool success = true;

			// Loading all faces(textures)
			for (int i = 0; i < 6; i++) {
				std::string fullPath = FileSystem::GetPath(skyboxFacespath[i]);
				facesData[i] = stbi_load(fullPath.c_str(), &w, &h, &channels, 4);
				if (!facesData[i]) {
					Logger::Err("Failed to load cubemap face: " + fullPath);
					success = false;
				}
			}

			if (success) {

				auto job = std::make_unique<CubeMapJob>();

				job->id = id;
				job->manager = this;
				job->width = w;
				job->height = h;

				for (int i = 0; i < 6; i++) {
					job->faceData[i] = facesData[i];
				}

				{
					std::lock_guard<std::mutex> lock(m_QueueMutex);
					m_UploadQueueJobs.push(std::move(job));
				}
			}
			}).detach();

		return { id };
	}

	void AssetManager::UpdateAssetManager()
	{
		
		// The way to grab the key (mutex) safely is : std::lock_guard<std::mutex> lock(queueMutex)
		// If another thread now tries to lock it, it must wait.
		// When {} finish that means that the lock_guard is automatically destroyed and releases the key.
		// Be careful of DEADLOCK ( locks the engine forever ).
		
		// Also we need to be careful because we do the following:
		// Lock the vault
		// While vault is open take out one asset and upload it to the GPU
		// Unlock the vault
		// This is actually bad -> Because if you have 50 textures to upload , the vault stays locked for a long time
		// Solution -> Queue Swap
		// 
		
	

		// Create a local, temp queue
		std::queue<std::unique_ptr<IUploadJob>> jobsToProcess;

		{
			// Here now we try to lock the queueMutex the Key to check if the back thread has finished anything.
			// Lock the mutex
			std::lock_guard<std::mutex> lock(m_QueueMutex);

			// SWAP: Move everything from the shared queue to our local one.
			// This is extremely fast (O(1)).
			std::swap(jobsToProcess, m_UploadQueueJobs);

		} // Lock released here

		// Process the jobs on the Main Thread without blocking anyone
		while (!jobsToProcess.empty()) {
			auto job = std::move(jobsToProcess.front());
			jobsToProcess.pop();

			// This calls your GL upload functions
			job->Execute();
		}
	}


	// GL Funcs for Texture needed, so it can be displayed
	void AssetManager::GLTextureUpload(AssetID id, unsigned char* data, int width, int height)
	{
		auto& tex = m_TexturesLibrary[id];

		glGenTextures(1, &tex->rendererID);
		glBindTexture(GL_TEXTURE_2D, tex->rendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		tex->width = width;
		tex->height = height;
		tex->isLoading = false;
		tex->isReady = true;

		Logger::Log("Asset " + std::to_string(id) + " is now ready on GPU!");
	}

	// GL Funcs for Mesh needed, so it can be displayed
	void AssetManager::GLMeshUpload(AssetID id, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		// Getting the Mesh Obj
		auto& assetMesh = m_MeshesLibrary[id];

		// Move vertices and indices to Mesh
		assetMesh->mesh->SetData(std::move(vertices), std::move(indices));
		assetMesh->mesh->SetupMesh();

		uint32_t vao = assetMesh->mesh->GetMesh();
		uint32_t instanceVBO = assetMesh->mesh->GetInstanceVBO();
		m_VAOToInstanceVBO[vao] = instanceVBO;

		assetMesh->isLoading = false;
		assetMesh->isReady = true;
		std::cout << "Upload Mesh Addr: " << assetMesh->mesh.get() << std::endl;
		Logger::Log("Mesh Asset " + std::to_string(id) + " uploaded to GPU!");
	}

	// GL Funcs for Shader needed, so it can be displayed
	void AssetManager::GLShaderUpload(AssetID id, const std::string& vSrc, const std::string& fSrc, const std::string& path)
	{
		auto newShader = std::make_unique<Shader>(vSrc, fSrc, path);

		// Store it in our map
		m_ShaderLibrary[id] = std::move(newShader);
	}

	// GL Funcs for CubeMap needed, so it can be displayed
	void AssetManager::GLCubeMapUpload(AssetID id, unsigned char** facesData, int width, int height)
	{
		auto& tex = m_TexturesLibrary[id];

		glGenTextures(1, &tex->rendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex->rendererID);

		for (unsigned int i = 0; i < 6; i++)
		{
			// We use GL_TEXTURE_CUBE_MAP_POSITIVE_X + i to target each face in order
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
				width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, facesData[i]);

			// Clean up the CPU memory for this face immediately after uploading to GPU
			stbi_image_free(facesData[i]);
		}

		// Cubemaps need specific wrapping to avoid lines at the edges of the cube
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		tex->width = width;
		tex->height = height;
		tex->isLoading = false;
		tex->isReady = true;

		Logger::Log("Cubemap Asset " + std::to_string(id) + " is now ready on GPU!");
	}

	// Get Shader ID
	uint32_t AssetManager::GetShaderID(AssetHandler handle)
	{
		if (m_ShaderLibrary.count(handle.id)) {
			return m_ShaderLibrary[handle.id]->GetRendererID();
		}
		return 0;
	}

	// Get Texture ID
	uint32_t AssetManager::GetTextureID(AssetHandler handle)
	{
		auto it = m_TexturesLibrary.find(handle.id);
		if (it != m_TexturesLibrary.end()) {
			// Only return the ID if the background thread + GL upload is finished
			if (it->second->isReady) {
				return it->second->rendererID;
			}
		}
		// Fallback texture ID 
		return m_DefaultTextureID;
	
	}

	// Get Mesh VAO
	uint32_t AssetManager::GetMeshVAO(AssetHandler handle)
	{
		auto it = m_MeshesLibrary.find(handle.id);
		if (it != m_MeshesLibrary.end()) {
			if (it->second->isReady) { 
				return it->second->mesh->GetMesh();
			}
		}
		return 0;

	}

	// Get Mesh Index Count
	uint32_t AssetManager::GetMeshIndexCount(AssetHandler handle)
	{
		if (m_MeshesLibrary.count(handle.id)) {
			return m_MeshesLibrary[handle.id]->mesh->GetIndexCount();
		}
		return 0;
	}

	// Get Instance VBO
	uint32_t AssetManager::GetMeshInstanceVBO(uint32_t vaoID)
	{
		auto it = m_VAOToInstanceVBO.find(vaoID);
		if (it != m_VAOToInstanceVBO.end()) {
			return it->second;
		}
		return 0;
	}

	float AssetManager::GetMeshRadius(AssetHandler handle)
	{
		if (m_MeshesLibrary.count(handle.id)) {
			return m_MeshesLibrary[handle.id]->mesh->GetLocalSphere().radius;
		}
		return 0.0f;
	}

	// Get All Indices of the given handler
	const std::vector<uint32_t>& AssetManager::GetMeshAllIndices(AssetHandler handle)
	{
		static const std::vector<uint32_t> empty;

		auto it = m_MeshesLibrary.find(handle.id);
		if (it != m_MeshesLibrary.end()) {
			return it->second->mesh->GetMeshAllIndices();
		}

		return empty;
	}

	// Get All Vertices of the given handler
	const std::vector<Vertex>& AssetManager::GetMeshAllVertices(AssetHandler handle)
	{
		static const std::vector<Vertex> empty;

		auto it = m_MeshesLibrary.find(handle.id);
		if (it != m_MeshesLibrary.end()) {
			return it->second->mesh->GetMeshAllVertices();
		}

		return empty;
	}

	// Check if Mesh is loaded
	bool AssetManager::IsMeshLoaded(AssetHandler handle)
	{
		auto it = m_MeshesLibrary.find(handle.id);
		if (it != m_MeshesLibrary.end())
		{
			bool ready = it->second->isReady;

			bool hasData = !it->second->mesh->GetMeshAllVertices().empty();

			if (ready && hasData) {
				return true; 
			}
		}
		return false;
		
	}

	// Get the Shader needed through its shader id
	Shader* AssetManager::GetShaderPtr(uint32_t shaderID)
	{
		auto it = m_ShaderLibrary.find(shaderID);
		if (it != m_ShaderLibrary.end()) {
			// .get() returns the raw pointer from the unique_ptr
			return it->second.get();
		}
		return nullptr;
	}
}