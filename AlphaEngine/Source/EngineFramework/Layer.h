#pragma once
#include <string>
#include "Event.h"
#include "EngineFramework/ECS/ECS.h"
#include <memory>

namespace AlphaEngine
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer") : m_DebugName(name) {}
		virtual ~Layer() = default;

		// Added/Removed from the stack
		virtual void OnAttach(ECSOrchestrator* currentOrchestratorECS) { if (!m_Orchestrator) { m_Orchestrator = currentOrchestratorECS; } }
		virtual void OnDetach() {}

		// Every Frame (Physics, Logic)
		virtual void OnUpdate(float deltaTime) {}

		// Every Frame (Graphics)
		virtual void OnRender() {}

		// This allows Application to shout an event happened to any layer.
		virtual void OnEvent(Event& event) {}

		// Debugging 
		// Inline because it suggests to the compiler that it should take the actual code inside the function
		// and "paste" it directly where the function is called, 
		// rather than jumping to a different place in memory.
		inline const std::string& GetName() const { return m_DebugName; }

	protected:
		ECSOrchestrator* m_Orchestrator;
		std::string m_DebugName;
	};
}