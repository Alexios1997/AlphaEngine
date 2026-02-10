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
#include "EngineFramework/Renderer/OpenGLRenderer.h"
#include "EngineFramework/Input.h"


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


		// Args&&... args: A "Universal Reference" that accepts anything(L - values or R - values).
		template<typename TLayer, typename... Args>
			requires(std::is_base_of_v<Layer, TLayer>)
		void PushLayer(Args&&... args) {
			// std:: forward very important
			// Suppose your AppLayer constructor needs a large string or a complex configuration object. If you just pass it normally, 
			// C++ might create 2 or 3 expensive copies of that data as it travels from main.cpp into your PushLayer function.
			m_LayerStack.push_back(std::make_unique<TLayer>(std::forward<Args>(args)...));
			m_LayerStack.back()->OnAttach();
		}

		// Checking the TLayer type and if it is the 
		// same with one of our own in layerstack then return the pointer
		// Applayer : 1
		// UILayer : 2
		// +++
		// It is like saying "Does this layer's ID equal to 2?"
		template<typename TLayer>
		TLayer* GetLayer() {
			for (auto& layer : m_LayerStack) {
				if (layer->GetLayerType() == TLayer::GetStaticType()) {
					return static_cast<TLayer*>(layer.get()); 
				}
			}
			return nullptr;
		}

		// Belongs to the class itself, Implementing a Singleton Pattern
		static Application& Get();
		static float GetTime();

	private:
		ApplicationSpecification m_Specification;

		std::unique_ptr<ECSOrchestrator> m_OrchestratorECS;
		std::unique_ptr<IRenderer> m_Renderer;
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Input> m_Input;
		std::vector<std::unique_ptr<Layer>> m_LayerStack;

		bool m_Running = false;
	};
}