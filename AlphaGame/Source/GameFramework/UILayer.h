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
