#pragma once
#include <string>
#include "Event.h"
#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Renderer/IRenderer.h"
#include <memory>

namespace AlphaEngine
{
	// TODO:: MAYBE MOVE IT TO A HELPER FUNC OR STH! UTILITY! THERE SHOULD 
		// Creating this function to use it so we can generate unique ids
		// that can be used to Identify the Layer later on
	inline uint32_t GenerateNewLayerID() {
		static uint32_t lastID = 0;
		return lastID++;
	}

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer") : m_DebugName(name) {}
		virtual ~Layer() = default;
		virtual uint32_t GetLayerType() const = 0;

		// Added/Removed from the stack
		virtual void OnAttach() {}
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
		std::string m_DebugName;
	};
}