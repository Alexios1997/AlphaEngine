#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "Event.h"

namespace AlphaEngine {

	struct WindowSpecification 
	{
		std::string Title;
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool isResizeable = true;
		bool VSync = true;

		// functional pointer!
		// returns void and takes an Event& as its only argument.
		using EventCallbackFn = std::function<void(Event&)>;
		EventCallbackFn EventCallback;
	};

	class Window {
	public:
		Window(const WindowSpecification& specification = WindowSpecification());
		~Window();

		void Create();
		void Destroy();
		void Update();

		void RaiseEvent(Event& event);

		

		bool ShouldClose() const;

		GLFWwindow* GetHandle() const { return m_Handle; }

	private:
		WindowSpecification m_Specification;
		GLFWwindow* m_Handle = nullptr;
	};
}