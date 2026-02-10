#pragma once
#include "UILayer.h"
#include <iostream>

namespace AlphaEngine
{

	UILayer::UILayer() : Layer("UILayer") {}

	void UILayer::OnAttach() {

		Layer::OnAttach();

		std::cout << "[UILayer] Attaching: Initializing UI System\n";
		// Example: ImGui::CreateContext();
	}

	void UILayer::OnDetach() {

		Layer::OnDetach();

		std::cout << "[UILayer] Detaching: Shutting down UI System\n";
		// Example: ImGui::DestroyContext();
	}

	void UILayer::OnUpdate(float ts) {
		// UI Logic (e.g., checking if the mouse is over a specific window)
	}

	void UILayer::OnRender() {
		// This is where you describe your UI elements
		// Example:
		// ImGui::Begin("Stats");
		// ImGui::Text("FPS: 144");
		// ImGui::End();
	}

	void UILayer::OnEvent(AlphaEngine::Event& event)
	{
		AlphaEngine::EventDispatcher dispatcher(event);
		//dispatcher.Dispatch<AlphaEngine::MouseButtonPressedEvent>([this](AlphaEngine::MouseButtonPressedEvent& e) { return OnMouseButtonPressed(e); });
	}

	void UILayer::BeginFrame() {
		// Pre-render setup for the UI
	}

	void UILayer::EndFrame() {
		// Finalize UI rendering and send to GPU
	}
}