#pragma once
#include <glm/glm.hpp>
#include "EngineFramework/Layer.h"
#include "EngineFramework/InputEvents.h"
#include "EngineFramework/WindowEvents.h"
#include "EngineFramework/Renderer/IRenderer.h"
#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Shader.h"


namespace AlphaEngine
{
	class AppLayer : public AlphaEngine::Layer
	{
	public:
		AppLayer();
		virtual ~AppLayer();

		// It will be assigned only once 
		static uint32_t GetStaticType() {
			static uint32_t typeID = GenerateNewLayerID();
			return typeID;
		}
		virtual uint32_t GetLayerType() const override { return GetStaticType(); }

		// Lifecycle Functions
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		// Loop Methods
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender() override;

		// Event Handling 
		virtual void OnEvent(AlphaEngine::Event& event) override;

		// Testing Funcs
		void RespawnBall(Entity entity);

	private:

		// Events
		bool OnMouseMoved(AlphaEngine::MouseMovedEvent& event);
		bool OnWindowClosed(AlphaEngine::WindowClosedEvent& event);
		bool OnResize(AlphaEngine::WindowResizeEvent& event);

	private:
		
		void HandleMouseClick();


		// Just testing vars
		std::unique_ptr<class Shader> m_BasicShader;
		std::shared_ptr<class Mesh> m_CubeMesh;
		
		
		float rotationSpeed = 30.f;
		Entity monkeyA;
		Entity monkeyB;
		Entity sphereA;
		Entity sphereB;
		Entity sphereC;
		Entity sphereD;
		Entity floor;
		Entity MiniGolfModel;
		Entity goalZone;


		// Frames Variables
		float m_FPSAccumulator = 0.0f; // Stores time until 1 second passes
		uint32_t m_FrameCounter = 0;   // Counts frames in that second

		glm::vec2 m_MousePosition{ 0.0f };

		float m_WindowWidth = 1920.0f;
		float m_WindowHeight = 1080.0f;
	};
}
