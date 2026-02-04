#pragma once
#include <glm/glm.hpp>
#include "EngineFramework/Layer.h"
#include "EngineFramework/InputEvents.h"
#include "EngineFramework/WindowEvents.h"
#include "EngineFramework/Renderer/IRenderer.h"


namespace AlphaEngine
{
	class AppLayer : public AlphaEngine::Layer
	{
	public:
		AppLayer();
		virtual ~AppLayer();

		// Lifecycle Functions
		virtual void OnAttach(ECSOrchestrator& ecsOrchestrator) override;
		virtual void OnDetach(ECSOrchestrator& ecsOrchestrator) override;

		// Loop Methods
		virtual void OnUpdate(ECSOrchestrator& ecsOrchestrator,float deltaTime) override;
		virtual void OnRender(ECSOrchestrator& ecsOrchestrator, IRenderer& currentRenderer) override;

		// Event Handling 
		virtual void OnEvent(AlphaEngine::Event& event) override;


	private:

		//bool OnMouseButtonPressed(AlphaEngine::MouseButtonPressedEvent& event);
		bool OnMouseMoved(AlphaEngine::MouseMovedEvent& event);
		bool OnWindowClosed(AlphaEngine::WindowClosedEvent& event);

	private:
		
		// Frames Variables
		float m_FPSAccumulator = 0.0f; // Stores time until 1 second passes
		uint32_t m_FrameCounter = 0;   // Counts frames in that second

		glm::vec2 m_MousePosition{ 0.0f };


	};
}
