#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>          
#include <glm/gtx/component_wise.hpp>
#include <cstdint>
#include <glad/gl.h>
#include <string>
#include "EngineFramework/Geometry.h"
#include <algorithm>
#include <iostream>
#include "EngineFramework/Logger.h"

namespace AlphaEngine
{

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	struct Texture
	{
		uint32_t id;
		std::string type;
	};

	class Mesh
	{
	public:
		Mesh() = default;

		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
		{
			this->m_Vertices = std::move(vertices);
			this->m_Indices = std::move(indices);
		}

		~Mesh() {
			glDeleteVertexArrays(1, &m_VAO);
			glDeleteBuffers(1, &m_VBO);
			glDeleteBuffers(1, &m_EBO);
			glDeleteBuffers(1, &m_InstanceVBO);
		}

		void KeepCpuData(bool keepData)
		{
			m_KeepMeshCPUData = keepData;
		}
		

		void SetupMesh()
		{
			m_IndexCount = static_cast<uint32_t>(m_Indices.size());

			glGenVertexArrays(1, &m_VAO);
			glGenBuffers(1, &m_VBO);
			glGenBuffers(1, &m_EBO);

			glBindVertexArray(m_VAO);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

			glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(uint32_t), m_Indices.data(), GL_STATIC_DRAW);

			// vertex positions
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			glEnableVertexAttribArray(0);
			// vertex normals
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			// vertex texture coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

			glGenBuffers(1, &m_InstanceVBO);
			glBindBuffer(GL_ARRAY_BUFFER, m_InstanceVBO);
			// Allocate space for up to 10,000 instances (640KB of VRAM)
			glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

			// A mat4 is 4 vec4s. We enable locations 3, 4, 5, and 6
			for (int i = 0; i < 4; i++) {
				glEnableVertexAttribArray(3 + i);
				glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
				// This makes it update per INSTANCE, not per vertex
				glVertexAttribDivisor(3 + i, 1);
			}


			CalculateBounds();

			if (!m_KeepMeshCPUData) 
			{
				m_Vertices.clear();
				m_Indices.clear();
				m_Vertices.shrink_to_fit();
				m_Indices.shrink_to_fit();
			}
			

			glBindVertexArray(0);
		}

		// Sets the data through RValues
		void SetData(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
		{
			// Move the data into our member variables
			// This is extremely fast because it doesn't copy the arrays
			m_Vertices = std::move(vertices);
			m_Indices = std::move(indices);
		}


		//noexcept: I guarantee this function will never throw an exception (error).
		// This is a Move constructor making sure that the other Mesh is making VAO,VBO,EBO to zero
		// and transfering the vertices,indicies and textures to this one
		Mesh(Mesh&& other) noexcept
		{
			m_VAO = other.m_VAO;
			m_VBO = other.m_VBO;
			m_EBO = other.m_EBO;
			m_IndexCount = other.m_IndexCount;

			m_Vertices = std::move(other.m_Vertices);
			m_Indices = std::move(other.m_Indices);

			other.m_VAO = 0;
			other.m_VBO = 0;
			other.m_EBO = 0;
			other.m_IndexCount = 0;
		}

		// Move assigment
		Mesh& operator=(Mesh&& other) noexcept
		{
			if (this != &other)
			{
				glDeleteVertexArrays(1, &m_VAO);
				glDeleteBuffers(1, &m_VBO);
				glDeleteBuffers(1, &m_EBO);
				glDeleteBuffers(1, &m_InstanceVBO);

				m_VAO = other.m_VAO;
				m_VBO = other.m_VBO;
				m_EBO = other.m_EBO;
				m_InstanceVBO = other.m_InstanceVBO;
				m_IndexCount = other.m_IndexCount;

				m_Vertices = std::move(other.m_Vertices);
				m_Indices = std::move(other.m_Indices);

				other.m_VAO = 0;
				other.m_VBO = 0;
				other.m_EBO = 0;
				other.m_IndexCount = 0;
				other.m_InstanceVBO = 0;
			}

			return *this;
		}


		inline const Sphere& GetLocalSphere() const { return m_LocalSphere; }
		inline const AABB& GetLocalAABB() const { return m_LocalAABB; }
		inline uint32_t GetMesh() const { return m_VAO; };
		inline uint32_t GetIndexCount() const { return m_IndexCount; };
		inline uint32_t GetInstanceVBO() const { return m_InstanceVBO; }
		inline const std::vector<Vertex>& GetMeshAllVertices() const { return m_Vertices; };
		inline const std::vector<uint32_t>& GetMeshAllIndices() const { return m_Indices; };
		

	private:

		bool m_KeepMeshCPUData;
		uint32_t m_VAO = 0, m_VBO = 0, m_EBO = 0;
		uint32_t m_IndexCount = 0;
		uint32_t m_InstanceVBO = 0;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Sphere m_LocalSphere;
		AABB m_LocalAABB;

		// Calculate the Bounding volume for a 3d mesh in local space! (AABB)
		void CalculateBounds()
		{
			glm::vec3 min(FLT_MAX), max(-FLT_MAX);
			float maxDistSq = 0.0f;

			for (const auto& v : m_Vertices) {
				min = glm::min(min, v.Position);
				max = glm::max(max, v.Position);
				// Find the vertex furthest from the center (0,0,0) for the sphere
				maxDistSq = std::max(maxDistSq, glm::length2(v.Position));
			}
			m_LocalAABB = { min, max };
			m_LocalSphere = { glm::vec3(0.0f), std::sqrt(maxDistSq) };
		}

		// Disable copying, It would be very bad for our game
		// = Delete means do not allow anyone to copy this object
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
	};
}