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
		virtual void OnAttach(ECSOrchestrator* currentOrchestratorECS) override;
		virtual void OnDetach() override;

		// Loop Methods
		virtual void OnUpdate(float ts) override;
		virtual void OnRender() override;

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
