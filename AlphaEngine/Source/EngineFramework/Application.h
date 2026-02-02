#pragma once

#include "Window.h"
#include "Layer.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>
#include <set>
#include "EngineFramework/ECS/ECS.h"


namespace AlphaEngine {

	struct ApplicationSpecification {
		std::string Name = "Application";
		WindowSpecification windowSpec;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification = ApplicationSpecification());
		~Application();

		void Run();
		void Stop();

		void RaiseEvent(Event& event);

		template<typename TLayer, typename... Args>
			requires(std::is_base_of_v<Layer, TLayer>)
		void PushLayer(Args&&... args) {
			// std:: forward very important
			// Suppose your AppLayer constructor needs a large string or a complex configuration object. If you just pass it normally, 
			// C++ might create 2 or 3 expensive copies of that data as it travels from main.cpp into your PushLayer function.
			m_LayerStack.push_back(std::make_unique<TLayer>(std::forward<Args>(args)...));
			m_LayerStack.back()->OnAttach(m_OrchestratorECS.get());
		}

		// Helper to find a specific layer by type
		template<typename TLayer>
			requires(std::is_base_of_v<Layer, TLayer>)
		TLayer* GetLayer() {
			for (auto& layer : m_LayerStack) {
				if (auto casted = dynamic_cast<TLayer*>(layer.get()))
					return casted;
			}
			return nullptr;
		}

		// Belongs to the class itself, Implementing a Singleton Pattern
		static Application& Get();
		static float GetTime();

	private:
		std::unique_ptr<ECSOrchestrator> m_OrchestratorECS;
		ApplicationSpecification m_Specification;
		std::shared_ptr<Window> m_Window;
		bool m_Running = false;
		std::vector<std::unique_ptr<Layer>> m_LayerStack;
	};
}