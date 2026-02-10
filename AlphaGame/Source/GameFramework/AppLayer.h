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

		

	private:

		// Events
		bool OnMouseMoved(AlphaEngine::MouseMovedEvent& event);
		bool OnWindowClosed(AlphaEngine::WindowClosedEvent& event);

	private:
		
		std::unique_ptr<class Shader> m_BasicShader;
		std::shared_ptr<class Mesh> m_CubeMesh;

		// Just testing vars
		float rotationSpeed = 30.f;
		Entity en1;
		Entity en2;


		// Frames Variables
		float m_FPSAccumulator = 0.0f; // Stores time until 1 second passes
		uint32_t m_FrameCounter = 0;   // Counts frames in that second

		glm::vec2 m_MousePosition{ 0.0f };


	};
}
