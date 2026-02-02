#include <glad/gl.h> 
#include "AppLayer.h"
#include "EngineFramework/Renderer/Renderer.h"
#include "EngineFramework/Logger.h"
#include <iostream>
#include <print>




namespace AlphaEngine
{
	AppLayer::AppLayer() : Layer("AppLayer")
	{
		gVertexArrayObject = 0;
		gVertexBufferObject = 0;
		gGraphicsPipelineShaderProgram = 0;
	}

	AppLayer::~AppLayer() {}

	// Initialize OpenGL buffers (VAO, VBO, Shaders) here
	void AppLayer::OnAttach(ECSOrchestrator* currentOrchestratorECS) 
	{

		Layer::OnAttach(currentOrchestratorECS);

		std::cout << "[AppLayer] Attaching:\n";

		// Living on CPU
		const std::vector<GLfloat> vertexPosition{
			-0.8f, -0.8f, 0.0f,
			 0.8f, -0.8f, 0.0f,
			 0.0f, 0.8f, 0.0f
		};

		Logger::Log("Created Vertices");
		Logger::Log("cREATE My entity");

		Entity myFirstEntityTest = currentOrchestratorECS->CreateEntity();
		Entity mySecondEntityTest = currentOrchestratorECS->CreateEntity();
		// Start setting thing up on the GPU

		/*
		// Generate a Vertext Array
		glGenVertexArrays(1, &gVertexArrayObject);
		// Select the one I specified
		glBindVertexArray(gVertexArrayObject);

		// Start Generating our VBO
		glGenBuffers(1, &gVertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, vertexPosition.size() * sizeof(GLfloat), vertexPosition.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);
		glDisableVertexAttribArray(0);
		*/

		////////////////Renderer::InitializeRenderedObject(gVertexArrayObject, gVertexBufferObject, vertexPosition);

		// Creating Graphics Pipeline
		////////////////gGraphicsPipelineShaderProgram = Renderer::CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	}

	void AppLayer::OnDetach() {
		std::cout << "[AppLayer] Detaching: Cleaning up GPU Resources\n";
		// Crucial: Delete resources from the GPU memory
		//glDeleteVertexArrays(1, &m_VertexArray);
		//glDeleteBuffers(1, &m_VertexBuffer);
		//glDeleteProgram(m_Shader);
	}

	// Update physics, player movement, etc.
	void AppLayer::OnUpdate(float deltaTime) {

		/*
		std::cout << "Delta Time: " << deltaTime * 1000.0f << "ms" << std::endl;

		// 1. Add current frame's time to our accumulator
		m_FPSAccumulator += deltaTime;
		m_FrameCounter++;

		// 2. If 1 second has passed, display and reset
		if (m_FPSAccumulator >= 1.0f)
		{
			float avgDeltaTime = (m_FPSAccumulator / m_FrameCounter) * 1000.0f; // ms
			uint32_t fps = m_FrameCounter;

			// Print to console
			std::cout << "[Performance] FPS: " << fps
				<< " | Avg Delta: " << avgDeltaTime << "ms" << std::endl;

			// Reset for the next second
			m_FPSAccumulator = 0.0f;
			m_FrameCounter = 0;
		}
		*/
	}

	// Draw objects/world
	void AppLayer::OnRender()
	{
		//Renderer::ClearScreen();
		//Renderer::RenderObject(gGraphicsPipelineShaderProgram, gVertexBufferObject, gVertexArrayObject);
	}

	void AppLayer::OnEvent(AlphaEngine::Event& event)
	{
		std::cout << event.ToString() << std::endl;

		AlphaEngine::EventDispatcher dispatcher(event);
		//dispatcher.Dispatch<AlphaEngine::MouseButtonPressedEvent>([this](AlphaEngine::MouseButtonPressedEvent& e) { return OnMouseButtonPressed(e); });
		dispatcher.Dispatch<AlphaEngine::MouseMovedEvent>([this](AlphaEngine::MouseMovedEvent& e) { return OnMouseMoved(e); });
		dispatcher.Dispatch<AlphaEngine::WindowClosedEvent>([this](AlphaEngine::WindowClosedEvent& e) { return OnWindowClosed(e); });
	}

	bool AppLayer::OnMouseMoved(AlphaEngine::MouseMovedEvent& event)
	{
		m_MousePosition = { static_cast<float>(event.GetX()), static_cast<float>(event.GetY()) };

		return false;
	}

	bool AppLayer::OnWindowClosed(AlphaEngine::WindowClosedEvent& event)
	{
		std::cout << "Window Closed!" << std::endl;

		return false;
	}
}
