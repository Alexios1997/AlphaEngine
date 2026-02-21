#pragma once

#include <math.h>
#include "EngineFrameWork/Geometry.h"

namespace AlphaEngine::Math {
	template<typename T>
	// Try to calculate the result of this function/variable at compile time, not while the game is running
	constexpr T Lerp(T a, T b, float t) {
		return a + t * (b - a);
	}

	float EaseInOutQuad(float t) {
		return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
	}
}

namespace AlphaEngine::FrustumUtils
{
	Frustum Extract(const glm::mat4& viewProj)
	{

		Frustum currentFrustum;

		// Left Plane
		currentFrustum.planes[0].normal.x = viewProj[0][3] + viewProj[0][0];
		currentFrustum.planes[0].normal.y = viewProj[1][3] + viewProj[1][0];
		currentFrustum.planes[0].normal.z = viewProj[2][3] + viewProj[2][0];
		currentFrustum.planes[0].distance = viewProj[3][3] + viewProj[3][0];

		// Right Plane
		currentFrustum.planes[1].normal.x = viewProj[0][3] - viewProj[0][0];
		currentFrustum.planes[1].normal.y = viewProj[1][3] - viewProj[1][0];
		currentFrustum.planes[1].normal.z = viewProj[2][3] - viewProj[2][0];
		currentFrustum.planes[1].distance = viewProj[3][3] - viewProj[3][0];

		// Bottom Plane
		currentFrustum.planes[2].normal.x = viewProj[0][3] + viewProj[0][1];
		currentFrustum.planes[2].normal.y = viewProj[1][3] + viewProj[1][1];
		currentFrustum.planes[2].normal.z = viewProj[2][3] + viewProj[2][1];
		currentFrustum.planes[2].distance = viewProj[3][3] + viewProj[3][1];

		// Top Plane
		currentFrustum.planes[3].normal.x = viewProj[0][3] - viewProj[0][1];
		currentFrustum.planes[3].normal.y = viewProj[1][3] - viewProj[1][1];
		currentFrustum.planes[3].normal.z = viewProj[2][3] - viewProj[2][1];
		currentFrustum.planes[3].distance = viewProj[3][3] - viewProj[3][1];

		// Near Plane
		currentFrustum.planes[4].normal.x = viewProj[0][3] + viewProj[0][2];
		currentFrustum.planes[4].normal.y = viewProj[1][3] + viewProj[1][2];
		currentFrustum.planes[4].normal.z = viewProj[2][3] + viewProj[2][2];
		currentFrustum.planes[4].distance = viewProj[3][3] + viewProj[3][2];

		// Far Plane
		currentFrustum.planes[5].normal.x = viewProj[0][3] - viewProj[0][2];
		currentFrustum.planes[5].normal.y = viewProj[1][3] - viewProj[1][2];
		currentFrustum.planes[5].normal.z = viewProj[2][3] - viewProj[2][2];
		currentFrustum.planes[5].distance = viewProj[3][3] - viewProj[3][2];

		// Normalize all planes so the distances are accurate
		for (int i = 0; i < 6; i++) {
			float length = glm::length(currentFrustum.planes[i].normal);
			currentFrustum.planes[i].normal /= length;
			currentFrustum.planes[i].distance /= length;
		}

		return currentFrustum;
	}
}

namespace AlphaEngine
{
	class CameraUtils {
	public:
		static Ray ScreenToWorldRay(float mouseX, float mouseY, float screenWidth, float screenHeight, const CameraComponent& camera, const glm::vec3& camPos) {
			// Pixel -> NDC
			float x = (2.0f * mouseX) / screenWidth - 1.0f;
			float y = 1.0f - (2.0f * mouseY) / screenHeight;

			// Inverse View-Projection
			glm::mat4 invVP = glm::inverse(camera.viewProj);
			glm::vec4 screenPos = glm::vec4(x, y, -1.0f, 1.0f);
			glm::vec4 worldPos = invVP * screenPos;

			glm::vec3 rayDir = glm::normalize(glm::vec3(worldPos) / worldPos.w - camPos);
			return { camPos, rayDir };
		}
	};

}