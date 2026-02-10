#include <glad/gl.h> 
#include "AppLayer.h"
#include "EngineFramework/Renderer/IRenderer.h"
#include "EngineFramework/Logger.h"
#include "EngineFramework/Components/TransformComponent.h"
#include "EngineFramework/Components/RendererComponent.h"
#include "EngineFramework/Components/CameraComponent.h"
#include "EngineFramework/Systems/CameraSystem.h"
#include "EngineFramework/Systems/RenderSystem.h"
#include "EngineFramework/Systems/PlayerControllerSystem.h"
#include "EngineFramework/Systems/MovementSystem.h"
#include <iostream>
#include <print>
#include "EngineFramework/ServiceLocator.h"
#include "EngineFramework/FileSystem.h"
#include "EngineFrameWork/Mesh.h"




namespace AlphaEngine
{
	AppLayer::AppLayer() : Layer("AppLayer"), en1{ 0 }, en2{ 1 }
	{
	}

	AppLayer::~AppLayer() {}

	// Initialize OpenGL buffers (VAO, VBO, Shaders) here
	void AppLayer::OnAttach()
	{

		Layer::OnAttach();

		auto& ecsOrchestrator = ServiceLocator::Get<ECSOrchestrator>();

		// Add Systems that are needed!


		ecsOrchestrator.AddSystem<RenderSystem>();
		ecsOrchestrator.AddSystem<CameraSystem>();
		ecsOrchestrator.AddSystem<PlayerControllerSystem>();
		ecsOrchestrator.AddSystem<MovementSystem>();



		std::cout << "[AppLayer] Attaching:\n";

		// Testing Purposes here

		uint32_t indices[] = {
			0, 1, 2, 2, 3, 0, // Front
			1, 5, 6, 6, 2, 1, // Right
			7, 6, 5, 5, 4, 7, // Back
			4, 0, 3, 3, 7, 4, // Left
			3, 2, 6, 6, 7, 3, // Top
			4, 5, 1, 1, 0, 4  // Bottom
		};


		std::vector<Vertex> meshVertices = {
			{ {-0.5f, -0.5f,  0.5f} }, { {0.5f, -0.5f,  0.5f} },
			{ { 0.5f,  0.5f,  0.5f} }, { {-0.5f,  0.5f,  0.5f} },
			{ {-0.5f, -0.5f, -0.5f} }, { {0.5f, -0.5f, -0.5f} },
			{ { 0.5f,  0.5f, -0.5f} }, { {-0.5f,  0.5f, -0.5f} }
		};

		std::vector<uint32_t> meshIndices(indices, indices + sizeof(indices) / sizeof(uint32_t));

		m_CubeMesh = std::make_shared<Mesh>(meshVertices, meshIndices);

		m_BasicShader = std::make_unique<Shader>(
			FileSystem::GetPath("AlphaGame/Shaders/Basic.glsl")
		);


		// To prevet the cube looking like inside-out
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);


		// Creating Entities just for testing

		en1 = ecsOrchestrator.CreateEntity();
		en2 = ecsOrchestrator.CreateEntity();
		auto cameraEntity = ecsOrchestrator.CreateEntity();



		// First Entity Cube

		ecsOrchestrator.AddComponent<TransformComponent>(en1,
			TransformComponent(glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.5f)));

		ecsOrchestrator.AddComponent<RenderComponent>(en1,
			RenderComponent(m_CubeMesh->GetMesh(), m_BasicShader->GetRendererID(), 36));

		ecsOrchestrator.AddComponent<PlayerControllerComponent>(en1,
			PlayerControllerComponent(5.f, 2.f));

		ecsOrchestrator.AddComponent<VelocityComponent>(en1,
			VelocityComponent());


		// Second Entity Cube

		ecsOrchestrator.AddComponent<TransformComponent>(en2,
			TransformComponent(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.5f)));

		ecsOrchestrator.AddComponent<RenderComponent>(en2,
			RenderComponent(m_CubeMesh->GetMesh(), m_BasicShader->GetRendererID(), 36));


		// Camera Entity

		ecsOrchestrator.AddComponent<TransformComponent>(cameraEntity,
			TransformComponent(glm::vec3(0.f, 0.f, 2.f), glm::vec3(1.f))
		);

		ecsOrchestrator.AddComponent<CameraComponent>(cameraEntity,
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		ecsOrchestrator.SetPrimaryCamera(cameraEntity);

	}

	void AppLayer::OnDetach() {

		Layer::OnDetach();
		std::cout << "[AppLayer] Detaching";

	}

	// Update physics, player movement, etc.
	void AppLayer::OnUpdate(float deltaTime) {

		/*
		//std::cout << "Delta Time: " << deltaTime * 1000.0f << "ms" << std::endl;

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
		//*/
		auto& ecsOrchestrator = ServiceLocator::Get<ECSOrchestrator>();
		auto& input = ServiceLocator::Get<Input>();

		auto& trans1 = ecsOrchestrator.GetComponent<TransformComponent>(en1);
		auto& trans2 = ecsOrchestrator.GetComponent<TransformComponent>(en2);

		trans1.rotation.y += rotationSpeed * deltaTime;

		// Cube 2 spins on X and Z for a "tumble" effect
		trans2.rotation.x += rotationSpeed * 0.5f * deltaTime;
		trans2.rotation.z += rotationSpeed * deltaTime;

		ecsOrchestrator.GetSystem<CameraSystem>().RunSystem(ecsOrchestrator);
		ecsOrchestrator.GetSystem<PlayerControllerSystem>().RunSystem(ecsOrchestrator, input);
		ecsOrchestrator.GetSystem<MovementSystem>().RunSystem(ecsOrchestrator, deltaTime);

	}

	// Draw objects/world
	void AppLayer::OnRender()
	{
		auto& ecsOrchestrator = ServiceLocator::Get<ECSOrchestrator>();
		auto& currentRenderer = ServiceLocator::Get<IRenderer>();

		ecsOrchestrator.GetSystem<RenderSystem>().RunSystem(currentRenderer, ecsOrchestrator);

	}

	void AppLayer::OnEvent(AlphaEngine::Event& event)
	{
		std::cout << event.ToString() << std::endl;

		AlphaEngine::EventDispatcher dispatcher(event);




		dispatcher.Dispatch<AlphaEngine::MouseMovedEvent>([this](AlphaEngine::MouseMovedEvent& e) { return OnMouseMoved(e); });
		dispatcher.Dispatch<AlphaEngine::WindowClosedEvent>([this](AlphaEngine::WindowClosedEvent& e) { return OnWindowClosed(e); });
	}



	bool AppLayer::OnMouseMoved(AlphaEngine::MouseMovedEvent& event)
	{
		m_MousePosition = { static_cast<float>(event.GetX()), static_cast<float>(event.GetY()) };

		// Returning false means that other layers can also respond if needed
		return false;
	}

	bool AppLayer::OnWindowClosed(AlphaEngine::WindowClosedEvent& event)
	{
		std::cout << "Window Closed!" << std::endl;

		// Returning false means that other layers can also respond if needed
		return false;
	}
}
