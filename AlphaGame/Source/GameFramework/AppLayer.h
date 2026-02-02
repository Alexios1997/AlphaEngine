#pragma once
#include <glm/glm.hpp>
#include "EngineFramework/Layer.h"
#include "EngineFramework/InputEvents.h"
#include "EngineFramework/WindowEvents.h"


namespace AlphaEngine
{
	class AppLayer : public AlphaEngine::Layer
	{
	public:
		AppLayer();
		virtual ~AppLayer();

		// Lifecycle Functions
		virtual void OnAttach(ECSOrchestrator* currentOrchestratorECS) override;
		virtual void OnDetach() override;

		// Loop Methods
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender() override;

		// Event Handling 
		virtual void OnEvent(AlphaEngine::Event& event) override;


	private:

		//bool OnMouseButtonPressed(AlphaEngine::MouseButtonPressedEvent& event);
		bool OnMouseMoved(AlphaEngine::MouseMovedEvent& event);
		bool OnWindowClosed(AlphaEngine::WindowClosedEvent& event);

	private:

		// OpenGL Variables
		GLuint gVertexArrayObject;
		GLuint gVertexBufferObject;

		// Program Object (For our shaders) - Graphics pipeline (Fragment + Vertex)
		GLuint gGraphicsPipelineShaderProgram;


		// Frames Variables
		float m_FPSAccumulator = 0.0f; // Stores time until 1 second passes
		uint32_t m_FrameCounter = 0;   // Counts frames in that second

		glm::vec2 m_MousePosition{ 0.0f };

		// Shaders Test
		const char* vertexShaderSource = "#version 410 core\n"
			"in vec4 position;\n"
			"void main() {\n"
			"    gl_Position = vec4(position.x,position.y,position.z,position.w);\n"
			"}\n";

		// GLSL 4.10 Fragment Shader String
		const char* fragmentShaderSource = "#version 410 core\n"
			"out vec4 FragColor;\n"
			"void main() {\n"
			"    FragColor = vec4(1.0f, 0.5f, 0.0f, 1.0f);\n"
			"}\0";

	};
}
