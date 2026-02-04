#include <glad/gl.h> 
#include "AppLayer.h"
#include "EngineFramework/Renderer/IRenderer.h"
#include "EngineFramework/Logger.h"
#include "EngineFramework/Components/TransformComponent.h"
#include "EngineFramework/Components/RendererComponent.h"
#include "EngineFramework/Systems/RenderSystem.h"
#include <iostream>
#include <print>




namespace AlphaEngine
{
	AppLayer::AppLayer() : Layer("AppLayer")
	{
	}

	AppLayer::~AppLayer() {}

	// Initialize OpenGL buffers (VAO, VBO, Shaders) here
	void AppLayer::OnAttach(ECSOrchestrator& ecsOrchestrator)
	{

		Layer::OnAttach(ecsOrchestrator);

		// Add Systems that are needed!

		ecsOrchestrator.AddSystem<RenderSystem>();

		std::cout << "[AppLayer] Attaching:\n";
		// Testing Purposes 

		float vertices[] = {
	-0.5f, -0.5f, 0.0f, // Bottom Left
	 0.5f, -0.5f, 0.0f, // Bottom Right
	 0.0f,  0.5f, 0.0f  // Top Center
		};

		uint32_t VAO, VBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// Define the layout (Location 0 in shader)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);


		uint32_t indices[] = { 0, 1, 2 };

		uint32_t EBO;
		glGenBuffers(1, &EBO);

		// 2. Bind EBO to the VAO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// We need an Index Count for our RenderCommand
		uint32_t myTriangleVAO = VAO;
		uint32_t myIndexCount = 3;


		const char* vertexShaderSource = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"uniform mat4 u_Model;\n"
			"void main() { gl_Position = u_Model * vec4(aPos, 1.0); }\0";

		const char* fragmentShaderSource = "#version 330 core\n"
			"out vec4 FragColor;\n"
			"void main() { FragColor = vec4(1.0, 0.5, 0.2, 1.0); }\0"; // Orange color

		// Standard OpenGL Shader Compilation
		uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		uint32_t myShaderID = glCreateProgram();
		glAttachShader(myShaderID, vertexShader);
		glAttachShader(myShaderID, fragmentShader);
		glLinkProgram(myShaderID);

		// Clean up temporary shaders
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);


		

		auto entity1 = ecsOrchestrator.CreateEntity();
		auto entity2 = ecsOrchestrator.CreateEntity();
		
		ecsOrchestrator.AddComponent<TransformComponent>(entity1, 
			TransformComponent(glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(1.0f)));

		ecsOrchestrator.AddComponent<RenderComponent>(entity1,
			RenderComponent(myTriangleVAO, myShaderID, myIndexCount));


		ecsOrchestrator.AddComponent<TransformComponent>(entity2,
			TransformComponent(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1.0f)));

		ecsOrchestrator.AddComponent<RenderComponent>(entity2,
			RenderComponent(myTriangleVAO, myShaderID, myIndexCount));

	}

	void AppLayer::OnDetach(ECSOrchestrator& ecsOrchestrator) {

		Layer::OnDetach(ecsOrchestrator);
		std::cout << "[AppLayer] Detaching: Cleaning up GPU Resources\n";
		// Crucial: Delete resources from the GPU memory
		//glDeleteVertexArrays(1, &m_VertexArray);
		//glDeleteBuffers(1, &m_VertexBuffer);
		//glDeleteProgram(m_Shader);
	}

	// Update physics, player movement, etc.
	void AppLayer::OnUpdate(ECSOrchestrator& ecsOrchestrator,float deltaTime) {

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
	void AppLayer::OnRender(ECSOrchestrator& ecsOrchestrator, IRenderer& currentRenderer)
	{
		
		ecsOrchestrator.GetSystem<RenderSystem>().RunSystem(currentRenderer, ecsOrchestrator);

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
