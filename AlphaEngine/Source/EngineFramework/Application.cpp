#include "EngineFramework/Application.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <iostream>
#include <ranges>


namespace AlphaEngine 
{
	// It exists for the entire lifetime of the program
	static Application* s_Application = nullptr;

	static void GLFWErrorCallback(int error, const char* description)
	{
		std::cerr << "[GLFW Error]: " << description << std::endl;
	}

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		glfwSetErrorCallback(GLFWErrorCallback);
		glfwInit();

		if (m_Specification.windowSpec.Title.empty())
			m_Specification.windowSpec.Title = m_Specification.Name;

		// Assigns a lambda expression o the callback variable
		// [this] (The Capture)
		m_Specification.windowSpec.EventCallback = [this](Event& event) {RaiseEvent(event); };

		m_Window = std::make_unique<Window>(m_Specification.windowSpec);
		m_Window->Create();

		m_Renderer = std::make_unique<OpenGLRenderer>();

		m_OrchestratorECS = std::make_unique<ECSOrchestrator>();
	}

	Application::~Application()
	{
		for (auto& layer : m_LayerStack)
		{
			layer->OnDetach(*m_OrchestratorECS);
		}

		m_Window->Destroy();
		glfwTerminate();

	}

	// ASK HERE
	void Application::Run()
	{
		m_Running = true;

		float lastTime = GetTime();

		// Main App loop
		while (m_Running)
		{
			glfwPollEvents();

			if (m_Window->ShouldClose())
			{
				Stop();
				break;
			}

			// Delta Time
			float currentTime = GetTime();
			float deltaTime = glm::clamp(currentTime - lastTime, 0.001f, 0.1f);
			lastTime = currentTime;

			// Here goes Logic and Physics for each layer
			for (auto& layer : m_LayerStack)
			{
				layer->OnUpdate(*m_OrchestratorECS,deltaTime);
			}

			// To Check The lifetime of the entities to be added and removed
			m_OrchestratorECS->UpdateEntitiesLifeTime();

			// We use begin frame outside of the loop
			// cause we dont want one layer clearing what other layers rendered already.
			m_Renderer->BeginFrame();
			for (auto& layer : m_LayerStack)
			{
				layer->OnRender(*m_OrchestratorECS , *m_Renderer);
			}
			// We use end frame outside of the loop
			// cause If Layer 1 uses "Shader A" and Layer 3 also uses "Shader A", the Renderer can draw them together. 
			// This prevents the GPU from switching shaders back and forth, which is the #1 performance killer in OpenGL.
			m_Renderer->EndFrame();

			// swapping buffers, Double buffering (Back and Front)
			m_Window->Update();
		}
	}

	void Application::Stop()
	{
		m_Running = false;
	}

	void Application::RaiseEvent(Event& event)
	{
		// Checking in reverse the layers to apply the Event Blocking
		// Uses a "Short-circuit" mechanism
		// Top-most UI gets event first
		for (auto& layer : std::views::reverse(m_LayerStack))
		{
			layer->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	Application& Application::Get()
	{
		if (s_Application != nullptr) {
			// Log an error or assert here!
		}
		return *s_Application;
	}

	float Application::GetTime()
	{
		return (float)glfwGetTime();
	}
}