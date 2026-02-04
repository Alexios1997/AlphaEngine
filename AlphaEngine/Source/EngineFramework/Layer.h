#pragma once
#include <string>
#include "Event.h"
#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Renderer/IRenderer.h"
#include <memory>

namespace AlphaEngine
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer") : m_DebugName(name) {}
		virtual ~Layer() = default;

		// Added/Removed from the stack
		virtual void OnAttach(ECSOrchestrator& ecsOrchestrator) {}
		virtual void OnDetach(ECSOrchestrator& ecsOrchestrator) {}

		// Every Frame (Physics, Logic)
		virtual void OnUpdate(ECSOrchestrator& ecsOrchestrator, float deltaTime) {}

		// Every Frame (Graphics)
		virtual void OnRender(ECSOrchestrator& ecsOrchestrator, IRenderer& currentRenderer) {}

		// This allows Application to shout an event happened to any layer.
		virtual void OnEvent(Event& event) {}

		// Debugging 
		// Inline because it suggests to the compiler that it should take the actual code inside the function
		// and "paste" it directly where the function is called, 
		// rather than jumping to a different place in memory.
		inline const std::string& GetName() const { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};
}