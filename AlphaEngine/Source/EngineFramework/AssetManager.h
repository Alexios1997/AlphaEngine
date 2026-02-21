#pragma once

#include <cstdint>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <unordered_map>
#include "EngineFrameWork/Logger.h"
#include "EngineFramework/ServiceLocator.h"
#include "EngineFramework/Shader.h"
#include "EngineFramework/Mesh.h"


namespace AlphaEngine
{

	// Asynchronous I / O: hARD DRIVE SHOULD NEVER STOP THE CPU

	class AssetManager;

	// <-------------------------- Asset Handles ----------------------------->

	// We need to compare two integers instead of two string obvisously
	using AssetID = uint32_t;

	// To have Asset handler is so important to make it Data Oriented Design and not passing Raw ints
	struct AssetHandler {
		AssetID id = 0;
		bool IsValid() const { return id != 0; }
	};

	// constexpr means that it can be calculated at compile-time
	// We hash the given path because we try to avoid having
	// std::string as Asset IDs because they are heavy for lookups ( They have O(N) time complexity for comparison and hashing)
	// Hash algo = FNV-1a
	constexpr AssetID HashGivenPath(const char* str, AssetID hash = 14695981039346656037ULL)
	{
		return (*str == '\0') ? hash : HashGivenPath(str + 1, (hash ^ (uint32_t)*str) * 1099511628211ULL);
	};

	// <--------------------------------------------------------------------->


	// <-------------------------- JOB SYSTEM ----------------------------->

	// We use structs because we need moslty because they are data things sent 
	// from one thread to another


	// Job are used because only the main thread sould touch the Graphics API
	// Also  OpenGL is not thread-safe. If the background thread calls glGenTextures, the game crashes. 
	// The Job is the "envelope" that carries the data back to the Main Thread so it can call OpenGL safely.
	struct IUploadJob {
		virtual ~IUploadJob() = default;
		virtual void Execute() = 0;
	};

	struct TextureJob : public IUploadJob {
		AssetID id;
		unsigned char* data;
		int width, height;
		AssetManager* manager;

		void Execute() override;
	};

	struct MeshJob : public IUploadJob {
		AssetID id;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		AssetManager* manager;

		void Execute() override;
	};

	struct ShaderJob : public IUploadJob {
		AssetID id;
		std::string vertexSource;
		std::string fragmentSource;
		std::string path;
		AssetManager* manager;

		void Execute() override;
	};

	struct CubeMapJob : public IUploadJob {
		AssetID id;
		unsigned char* faceData[6];
		int width, height;
		AssetManager* manager;

		void Execute() override;
	};

	// <--------------------------------------------------------------------->



	// <-------------------------- Assets data ----------------------------->

	// An atomic variable is a type that can be read or written by 
	// multiple threads without a mutex
	// In general, Mutexes are heavy and slow down the CPU. If we want just to check
	// a simple status (like is LOADED), an atomic is much faster
	// ALSO, Texture Asset holds the actual data.
	struct TextureAsset {
		unsigned int rendererID = 0;
		int width, height, channels;
		unsigned char* cpuData = nullptr;
		// Main Thread can check if Ready or loading
		std::atomic<bool> isReady{ false };
		std::atomic<bool> isLoading{ false };
	};

	struct MeshAsset {

		std::unique_ptr<Mesh> mesh;

		std::atomic<bool> isReady{ false };
		std::atomic<bool> isLoading{ false };
	};

	// <--------------------------------------------------------------------->



	// The Asset manager should just load data into CPU RAM from disk
	// Also systems should talk to each other through asset handlers and not directly Obvisouly!
	class AssetManager : public IService
	{
	private:

		// The Texture map
		std::unordered_map<AssetID, std::unique_ptr<TextureAsset>> m_TexturesLibrary;

		// The Shader Map
		std::unordered_map<AssetID, std::unique_ptr<Shader>> m_ShaderLibrary;

		// The Mesh Map
		std::unordered_map<AssetID, std::unique_ptr<MeshAsset>> m_MeshesLibrary;

		// A queue that have all of our JObs
		std::queue< std::unique_ptr<IUploadJob>> m_UploadQueueJobs;

		// A map with VAO ID TO INSTANCE VBO ID
		std::unordered_map<uint32_t, uint32_t> m_VAOToInstanceVBO;

		// Mutex (Mutual Exclusion): Think of it like a key for your data. If 2 threads try to change
		// the same std::queue for example at the exact same time, the program will crash.
		// UploadQueue is the vault and queueMutex is the key
		// Think of it like "No thread is allowed to touch the uploadQueue unless it is currently holding the queueMutex."
		std::mutex m_QueueMutex;

		// Default Loading look like texture Pinky
		uint32_t m_DefaultTextureID;

	public:
		AssetManager();
		virtual ~AssetManager() = default;
		virtual void InitService() override { Logger::Log("Initializing Service named : Asset Manager"); };

		// Load Async Texture
		AssetHandler LoadTexture(const std::string& path);
		// Load Shader
		AssetHandler LoadShader(const std::string& path);
		// Load Mesh
		AssetHandler LoadMesh(const std::string& path, bool keepDataToCpu);
		// Load CubeMap
		AssetHandler LoadCubeMap(const std::vector<std::string> skyboxFaces);

		// Every Frame check for the upload queue and make GL Uploads
		void UpdateAssetManager();

		// Gl Texture Upload
		void GLTextureUpload(AssetID id, unsigned char* data, int width, int height);
		// Gl Mesh Upload
		void GLMeshUpload(AssetID id, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		// Gl Shader Upload
		void GLShaderUpload(AssetID id, const std::string& vSrc, const std::string& fSrc, const std::string& path);
		// GL CubeMap Upload
		void GLCubeMapUpload(AssetID id, unsigned char** facesData, int width, int height);

		// Get Shader
		uint32_t GetShaderID(AssetHandler handle);
		// Get TextureId
		uint32_t GetTextureID(AssetHandler handle);
		// Get Mesh VAO
		uint32_t GetMeshVAO(AssetHandler handle);
		// Get Mesh Indices
		uint32_t GetMeshIndexCount(AssetHandler handle);
		// Get VBO
		uint32_t GetMeshInstanceVBO(uint32_t vaoID);
		// Get Mesh Radius
		float GetMeshRadius(AssetHandler handle);
		// Get All indices
		const std::vector<uint32_t>& GetMeshAllIndices(AssetHandler handle);
		// Get All indices
		const std::vector<Vertex>& GetMeshAllVertices(AssetHandler handle);
		// Is Given Mesh Loaded
		bool IsMeshLoaded(AssetHandler handle);

		Shader* GetShaderPtr(uint32_t shaderID);
	};
}