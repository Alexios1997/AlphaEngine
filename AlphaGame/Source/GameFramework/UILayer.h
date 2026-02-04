#pragma once

#include "EngineFramework/Layer.h"
#include "EngineFramework/InputEvents.h"
#include "EngineFramework/WindowEvents.h"


namespace AlphaEngine
{

	class UILayer : public AlphaEngine::Layer {
	public:
		UILayer();
		virtual ~UILayer() = default;

		// Lifecycle Functions
		virtual void OnAttach(ECSOrchestrator& ecsOrchestrator) override;
		virtual void OnDetach(ECSOrchestrator& ecsOrchestrator) override;

		// Loop Methods
		virtual void OnUpdate(ECSOrchestrator& ecsOrchestrator,float ts) override;
		virtual void OnRender(ECSOrchestrator& ecsOrchestrator, IRenderer& currentRenderer) override;

		// Event Handling
		virtual void OnEvent(AlphaEngine::Event& event) override;

		// Helper functions for UI frame management
		void BeginFrame();
		void EndFrame();

	private:

		//bool IsButtonHovered() const;
		//bool OnMouseButtonPressed(AlphaEngine::MouseButtonPressedEvent& event);
	};
}
