#include "EngineFramework/Renderer/OpenGLRenderer.h"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

namespace AlphaEngine
{
	// use std::span because
	// 1. Flexibility -> A function taking const std::vector<GLfloat>& only works with vectors. 
	// If you later decide to use a std::array for static geometry or have a raw pointer from a C-style library, 
	// you would have to rewrite your function or copy data into a new vector just to call it.
	// 
	// 2. If you have a giant vector containing 1,000 vertices but only want to upload vertices 50 through 100 to a specific GPU buffer,
	// std::vector makes this difficult without copying.
	// With std::span, you can simply pass a subspan
	// 
	// 3. Safety and Expressiveness -> Non-Owning Intent: When you see std::span, you immediately know the function is just "looking" 
	// at the data and will not try to resize or manage the memory.


	OpenGLRenderer::OpenGLRenderer()
		: m_ActiveViewProj(1.0f), m_CameraUBO(0)
	{
		glGenBuffers(1, &m_CameraUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_CameraUBO);

		// Allocate 64 bytes (size of one mat4)
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

		// Bind to Slot 0. 
		// This MUST match "binding = 0" in your shader
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_CameraUBO);
	}

	OpenGLRenderer::~OpenGLRenderer()
	{
	}

	void AlphaEngine::OpenGLRenderer::BeginFrame()
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_DrawQueueRCs.clear();
	}

	// Decouple the logic from the rendering by fueling commands
	void OpenGLRenderer::FuelRenderCommands(const RenderCommand& command)
	{
		m_DrawQueueRCs.push_back(command);
	}

	void OpenGLRenderer::SetViewProjection(const glm::mat4& viewProj)
	{
		m_ActiveViewProj = viewProj;
	}

	void OpenGLRenderer::EndFrame()
	{
		// MVP - Model, View, Projection
		// Model Matrix: Moves the object from ((0,0,0) to its position in the world. (Local -> World).
		// View Matrix: Moves the world so the camera is at (0,0,0). (World -> View).
		// Projection Matrix: Squashes 3D coordinates into 2D screen space and handles perspective (making far things small). (View -> Clip)

		// Order matters: 
		// First, place the vertex in the world.
		// Then, move it relative to the camera.
		// Finally, project it onto the screen.
		// 
		// P * V * M * Vertex

		glBindBuffer(GL_UNIFORM_BUFFER, m_CameraUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(m_ActiveViewProj));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


		// Sort according to layers first and then by Shader and then Texture to minimize state changes.
		std::sort(m_DrawQueueRCs.begin(), m_DrawQueueRCs.end(), [](const RenderCommand& a, const RenderCommand& b) {
			if (a.layerID != b.layerID) return a.layerID < b.layerID;
			if (a.shaderID != b.shaderID) return a.shaderID < b.shaderID;
			return a.textureID < b.textureID;
			});

		// STATE CACHING: Prevent redundant OpenGL calls
		uint32_t activeShader = 0;
		uint32_t activeVAO = 0;
		uint32_t activeTexture = 0;

		// EXECUTION LOOP
		for (const auto& cmd : m_DrawQueueRCs) {

			// --- SHADER BINDING ---
			if (cmd.shaderID != activeShader) {
				glUseProgram(cmd.shaderID);
				activeShader = cmd.shaderID;
			}

			// --- TEXTURE BINDING ---
			if (cmd.textureID != activeTexture) {
				glBindTexture(GL_TEXTURE_2D, cmd.textureID);
				activeTexture = cmd.textureID;
			}

			// --- VAO BINDING ---
			if (cmd.vao != activeVAO) {
				glBindVertexArray(cmd.vao);
				activeVAO = cmd.vao;
			}

			// --- UNIFORM UPLOAD (Per-Object) ---
			// We use the raw matrix from our TransformComponent
			// Ask OpenGL for the location of "u_Model"
			GLint modelLoc = glGetUniformLocation(cmd.shaderID, "u_Model");

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cmd.transform));

			// --- THE ACTUAL DRAW CALL ---
			// We use the indexCount stored in the command
			glDrawElements(GL_TRIANGLES, cmd.indexCount, GL_UNSIGNED_INT, nullptr);


		}

		// Cleaning Up
		glBindVertexArray(0);
		glUseProgram(0);
	}

	void OpenGLRenderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
	}

	
}


