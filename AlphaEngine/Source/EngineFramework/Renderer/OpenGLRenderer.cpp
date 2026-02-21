#include "EngineFramework/Renderer/OpenGLRenderer.h"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include "EngineFramework/AssetManager.h"
#include "EngineFramework/ServiceLocator.h"

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

	void OpenGLRenderer::SetViewProjection(const glm::mat4& viewProj, const glm::mat4& viewMatrix)
	{
		m_ActiveViewProj = viewProj;
		m_ActiveView = viewMatrix;
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

		Shader* currentShaderObj = nullptr;


		static std::vector<glm::mat4> instanceMatrices;
		instanceMatrices.clear();
		instanceMatrices.reserve(1000);


		glm::mat4 viewInv = glm::inverse(m_ActiveView);
		glm::vec3 cameraPos = glm::vec3(viewInv[3]);

		// EXECUTION LOOP
		// ALSO AVOIDING THE Strings all the time is important !
		for (size_t i = 0; i < m_DrawQueueRCs.size(); ++i) {

			const auto& cmd = m_DrawQueueRCs[i];

			// --- SHADER BINDING ---
			if (cmd.shaderID != activeShader) {

				activeShader = cmd.shaderID;
				currentShaderObj = ServiceLocator::Get<AssetManager>().GetShaderPtr(cmd.shaderID);

				if (currentShaderObj) {

					glUseProgram(currentShaderObj->GetRendererID());

					if (!cmd.isCubemap) {
						int lightLoc = currentShaderObj->GetUniforms().lightDirLoc;
						if (lightLoc != -1) glUniform3f(lightLoc, 0.5f, 1.0f, 0.3f);

						int viewPosLoc = currentShaderObj->GetUniforms().viewPosLoc;
						if (viewPosLoc != -1) {
							// The camera pos for Specular Highlights
							glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
						}

					}
					else {
						// Tell the Skybox shader to look at Texture Slot 0
						int skyboxLoc = currentShaderObj->GetUniforms().textureLoc;
						if (skyboxLoc != -1) glUniform1i(skyboxLoc, 0);
					}

				}
			}

			if (!currentShaderObj) continue;

			// --- TEXTURE BINDING ---
			if (cmd.textureID != activeTexture) {

				glActiveTexture(GL_TEXTURE0);

				if (cmd.isCubemap) {
					glBindTexture(GL_TEXTURE_CUBE_MAP, cmd.textureID);
				}
				else {
					glBindTexture(GL_TEXTURE_2D, cmd.textureID);
				}

				activeTexture = cmd.textureID;
			}

			if (cmd.isCubemap)
			{
				glDepthFunc(GL_LEQUAL);

				int vpLoc = currentShaderObj->GetUniforms().viewProjLoc;
				glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(cmd.skyboxVP));
			}
			else
			{
				glDepthFunc(GL_LESS);

			}

			// --- VAO BINDING ---
			if (cmd.vao != activeVAO) {
				glBindVertexArray(cmd.vao);
				activeVAO = cmd.vao;
			}


			// --- The Actual Drawing ---
			if (cmd.isCubemap) {
				glDisable(GL_CULL_FACE); // Inside looking out
				glDepthFunc(GL_LEQUAL);  // Draw at 1.0 depth

				int vpLoc = currentShaderObj->GetUniforms().viewProjLoc;
				glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(cmd.skyboxVP));		

				if (cmd.indexCount == 0) {
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
				else 
				{
					// --- THE ACTUAL DRAW CALL ---
					// We use the indexCount stored in the command
					glDrawElements(GL_TRIANGLES, cmd.indexCount, GL_UNSIGNED_INT, nullptr);
				}

				glEnable(GL_CULL_FACE);
				glDepthFunc(GL_LESS);
			}
			else
			{

				// <------------ Batch Processing ------------>
				// One of the most common bottlenecks in game engines are Draw Call Overhead.
				// By using instanceMatrices we do pre-calculations that are needed and memory management
				// And here we do The collection The batching
				// The code looks at the next item in the list. If the next item uses a different mesh, shader, or texture, 
				// it means the "Group" is finished.
				// We can only instance objects that share the exact same GPU state


				instanceMatrices.push_back(cmd.transform);

				bool isLast = (i == m_DrawQueueRCs.size() - 1);
				bool nextIsDifferent = !isLast && (
					m_DrawQueueRCs[i + 1].vao != cmd.vao ||
					m_DrawQueueRCs[i + 1].shaderID != cmd.shaderID ||
					m_DrawQueueRCs[i + 1].textureID != cmd.textureID ||
					m_DrawQueueRCs[i + 1].isCubemap 
					);

				if (isLast || nextIsDifferent) {

					//  Get the Instance VBO for this mesh
					uint32_t instanceVBO = ServiceLocator::Get<AssetManager>().GetMeshInstanceVBO(cmd.vao);

					// HERE We do the "orphaning"
					// The "Orphan" tells the GPU Driver 
					// "I am about to overwrite this. Don't wait for the previous frame to finish—just give me a fresh block of memory."

					glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
					glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

					// Now upload the actual data
					glBufferSubData(GL_ARRAY_BUFFER, 0, instanceMatrices.size() * sizeof(glm::mat4), instanceMatrices.data());

					//  ONE DRAW CALL for the whole group
					glDrawElementsInstanced(GL_TRIANGLES, cmd.indexCount, GL_UNSIGNED_INT, nullptr, (GLsizei)instanceMatrices.size());

					instanceMatrices.clear(); // Reset for next batch
				}
			}
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


