
#include "EngineFramework/Input.h"

namespace AlphaEngine
{
	void Input::UpdateState()
	{

		m_PreviousKeys = m_CurrentKeys;
		m_PreviousMouseButtons = m_CurrentMouseButtons;

		// Poll Keyboard
		// GLFW keys range from 32 to 348.
		for (int i = 32; i < 349; ++i) {
			m_CurrentKeys.set(i, glfwGetKey(m_Window, i) == GLFW_PRESS);
		}

		// Poll Mouse Buttons (8 buttons)
		for (int i = 0; i < 8; ++i) {
			m_CurrentMouseButtons.set(i, glfwGetMouseButton(m_Window, i) == GLFW_PRESS);
		}
	}

	bool Input::IsKeyDown(int keycode) const {
		return m_CurrentKeys.test(keycode);
	}

	bool Input::IsKeyPressed(int keycode) const {
		// Was UP last frame AND is DOWN this frame
		return m_CurrentKeys.test(keycode) && !m_PreviousKeys.test(keycode);
	}

	bool Input::IsKeyReleased(int keycode) const {
		// Was DOWN last frame AND is UP this frame
		return !m_CurrentKeys.test(keycode) && m_PreviousKeys.test(keycode);
	}

	bool Input::IsMouseButtonDown(int button) const {
		return m_CurrentMouseButtons.test(button);
	}

	bool Input::IsMouseButtonPressed(int button) const {
		return m_CurrentMouseButtons.test(button) && !m_PreviousMouseButtons.test(button);
	}

	glm::vec2 Input::GetMousePosition() const {
		double xpos, ypos;
		glfwGetCursorPos(m_Window, &xpos, &ypos);
		return { (float)xpos, (float)ypos };
	}
}