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


	void AlphaEngine::OpenGLRenderer::BeginFrame()
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Logger::Log("Wel nICE");
		m_DrawQueueRCs.clear();
	}

	// Decouple the logic from the rendering by fueling commands
	void OpenGLRenderer::FuelRenderCommands(const RenderCommand& command)
	{
		m_DrawQueueRCs.push_back(command);
	}

	void OpenGLRenderer::EndFrame()
	{


		// TODO: TAKE A BETTER LOOK UNDERSTAND!

		// 1. SORTING: The AAA Optimization Step
		// We sort by Layer first, then by Shader to minimize state changes.
		std::sort(m_DrawQueueRCs.begin(), m_DrawQueueRCs.end(), [](const RenderCommand& a, const RenderCommand& b) {
			if (a.layerID != b.layerID) return a.layerID < b.layerID;
			if (a.shaderID != b.shaderID) return a.shaderID < b.shaderID;
			return a.textureID < b.textureID;
			});

		// 2. STATE CACHING: Prevent redundant OpenGL calls
		uint32_t activeShader = 0;
		uint32_t activeVAO = 0;
		uint32_t activeTexture = 0;

		// 3. THE EXECUTION LOOP
		for (const auto& cmd : m_DrawQueueRCs) {

			// --- SHADER BINDING ---
			if (cmd.shaderID != activeShader) {
				glUseProgram(cmd.shaderID);
				activeShader = cmd.shaderID;
				// When shader changes, we might need to set global uniforms (like Projection/View)
				// UploadGlobalUniforms(cmd.shaderID); 
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
			GLint modelLoc = glGetUniformLocation(cmd.shaderID, "u_Model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cmd.transform));

			// --- THE ACTUAL DRAW CALL ---
			// We use the indexCount stored in the command
			glDrawElements(GL_TRIANGLES, cmd.indexCount, GL_UNSIGNED_INT, nullptr);
		}

		// 4. CLEANUP (Optional)
		// Some developers like to unbind at the end to prevent accidental leaks
		glBindVertexArray(0);
		glUseProgram(0);
	}

	/*
	void InitializeRenderedObject(GLuint& vertexArrayObject, GLuint& vertexBufferObject, std::span<const GLfloat> vertexPosition)
	{
		glGenVertexArrays(1, &vertexArrayObject);
		// Select the one I specified
		glBindVertexArray(vertexArrayObject);

		// Start Generating our VBO
		glGenBuffers(1, &vertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, vertexPosition.size_bytes(), vertexPosition.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);
		glDisableVertexAttribArray(0);
	}

	GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
	{
		GLuint programObject = glCreateProgram();

		GLuint myVertexShader = Renderer::CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
		GLuint myFragmentShader = Renderer::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

		glAttachShader(programObject, myVertexShader);
		glAttachShader(programObject, myFragmentShader);
		glLinkProgram(programObject);

		glValidateProgram(programObject);

		return programObject;
	}

	GLuint CompileShader(GLuint type, const std::string& shaderSource)
	{
		GLuint shaderObject;

		if (type == GL_VERTEX_SHADER)
		{
			shaderObject = glCreateShader(GL_VERTEX_SHADER);
		}
		else if (type == GL_FRAGMENT_SHADER)
		{
			shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
		}

		const char* src = shaderSource.c_str();
		glShaderSource(shaderObject, 1, &src, nullptr);
		glCompileShader(shaderObject);

		return shaderObject;
	}

	void ClearScreen()
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glViewport(0, 0, 1920, 1080);
		glClearColor(1.f, 1.f, 0.f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}

	void RenderObject(GLuint graphicsPipelineShaderProgram, GLuint vertexBufferObject, GLuint vertexArrayObject)
	{
		glUseProgram(graphicsPipelineShaderProgram);

		// Draw now

		glBindVertexArray(vertexArrayObject);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	*/
}


