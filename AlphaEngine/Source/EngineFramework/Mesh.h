#pragma once

#include <cstdint>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>


namespace AlphaEngine
{

	struct Vertex
	{
		glm::vec3 Position;
		//glm::vec3 Normal;
		//glm::vec2 TexCoords;
	};

	struct Texture
	{
		uint32_t id;
		std::string type;
	};

	class Mesh
	{
	public:
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
		{
			this->m_Vertices = std::move(vertices);
			this->m_Indices = std::move(indices);
			//this->m_Textures = std::move(textures);

			SetupMesh();
		}

		~Mesh() {
			glDeleteVertexArrays(1, &m_VAO);
			glDeleteBuffers(1, &m_VBO);
			glDeleteBuffers(1, &m_EBO);
		}

		//noexcept: I guarantee this function will never throw an exception (error).
		// This is a Move constructor making sure that the other Mesh is making VAO,VBO,EBO to zero
		// and transfering the vertices,indicies and textures to this one
		Mesh(Mesh&& other) noexcept
		{
			m_VAO = other.m_VAO;
			m_VBO = other.m_VBO;
			m_EBO = other.m_EBO;

			m_Vertices = std::move(other.m_Vertices);
			m_Indices = std::move(other.m_Indices);
			//m_Textures = std::move(other.m_Textures);

			other.m_VAO = 0;
			other.m_VBO = 0;
			other.m_EBO = 0;
		}

		// Move assigment
		Mesh& operator=(Mesh&& other) noexcept
		{
			if (this != &other)
			{
				glDeleteVertexArrays(1, &m_VAO);
				glDeleteBuffers(1, &m_VBO);
				glDeleteBuffers(1, &m_EBO);

				m_VAO = other.m_VAO;
				m_VBO = other.m_VBO;
				m_EBO = other.m_EBO;

				m_Vertices = std::move(other.m_Vertices);
				m_Indices = std::move(other.m_Indices);
				//m_Textures = std::move(other.m_Textures);

				other.m_VAO = 0;
				other.m_VBO = 0;
				other.m_EBO = 0;
			}

			return *this;
		}


		uint32_t GetMesh() const { return m_VAO; };

	private:
		uint32_t m_VAO = 0, m_VBO = 0, m_EBO = 0;

		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		//std::vector<Texture> m_Textures;


		void SetupMesh()
		{
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
			//glEnableVertexAttribArray(1);
			//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			// vertex texture coords
			//glEnableVertexAttribArray(2);
			//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

			glBindVertexArray(0);
		}

		// Disable copying, It would be very bad for our game
		// = Delete means do not allow anyone to copy this object
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
	};
}