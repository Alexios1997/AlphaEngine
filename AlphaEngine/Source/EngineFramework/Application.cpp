#define JPH_HEADER_INCLUDED
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core//Factory.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <iostream>
#include <ranges>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "EngineFramework/Application.h"
#include "EngineFramework/InputEvents.h"
#include "EngineFramework/WindowEvents.h"
#include "EngineFramework/EventBus.h"
#include "EngineFramework/ServiceLocator.h"
#include <cstdint>



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
		m_Renderer->OnWindowResize(m_Window->GetWidth(), m_Window->GetHeight());

		m_OrchestratorECS = std::make_unique<ECSOrchestrator>();

		m_Input = std::make_unique<Input>(m_Window->GetHandle());

		m_AssetManager = std::make_unique<AssetManager>();

		// Register them so the rest of the engine can find them
		ServiceLocator::Provide<IRenderer>(m_Renderer.get());
		ServiceLocator::Provide<ECSOrchestrator>(m_OrchestratorECS.get());
		ServiceLocator::Provide<Input>(m_Input.get());
		ServiceLocator::Provide<AssetManager>(m_AssetManager.get());

		InitPhysics();
	}

	void Application::InitPhysics()
	{
		JPH::RegisterDefaultAllocator();
		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		std::cout << "Jolt initialized successfully!" << std::endl;
	}

	Application::~Application()
	{
		for (auto& layer : m_LayerStack)
		{
			layer->OnDetach();
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

			// Updating the bitsets for the current frame
			ServiceLocator::Get<Input>().UpdateState();

			ServiceLocator::Get<AssetManager>().UpdateAssetManager();

			// To Check The lifetime of the entities to be added and removed
			m_OrchestratorECS->UpdateEntitiesLifeTime();

			// Here goes Logic and Physics for each layer
			for (auto& layer : m_LayerStack)
			{
				layer->OnUpdate(deltaTime);
			}

			// We use begin frame outside of the loop
			// cause we dont want one layer clearing what other layers rendered already.
			m_Renderer->BeginFrame();
			for (auto& layer : m_LayerStack)
			{
				layer->OnRender();
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

		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<AlphaEngine::WindowResizeEvent>([this](AlphaEngine::WindowResizeEvent& event) {
			m_Renderer->OnWindowResize(event.GetWidth(), event.GetHeight());			
			return false; // Let other layers see the event too
		});

		// Broadcast this event ! 
		EventBus::Publish(event);

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