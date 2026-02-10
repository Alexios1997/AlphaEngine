#pragma once

#include "EngineFramework/ServiceLocator.h"
#include "EngineFramework/Logger.h"
#include <bitset>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace AlphaEngine
{

	// This class is used for STATE  (Continuous movement and Mouse Look) -> "Is the player holding W right now?"
	// While on the other hand The Events are callbacks pretty much. (Discrete Actions) -> "The player just clicked this once. Does anyone want to know ?"
	class Input : public IService
	{
	public:
		Input(GLFWwindow* window) : m_Window(window) {}
		virtual ~Input() = default;
		virtual void InitService() override { Logger::Log("Initializing Service named : Input"); };

		void UpdateState();


		bool IsKeyDown(int keycode) const;      
		bool IsKeyPressed(int keycode) const;   
		bool IsKeyReleased(int keycode) const;

		bool IsMouseButtonDown(int button) const;
		bool IsMouseButtonPressed(int button) const;
		glm::vec2 GetMousePosition() const;

	private:
		GLFWwindow* m_Window;

		// Bitsets for The keys ( 512 bytes) 
		// Modern CPUs have 64 bytes so 8 cache lines
		// which means that can read this entire array into its fastest memory (L1 Cache) in a single burst
		std::bitset<512> m_CurrentKeys;
		std::bitset<512> m_PreviousKeys;

		std::bitset<8> m_CurrentMouseButtons;
		std::bitset<8> m_PreviousMouseButtons;
	};
}