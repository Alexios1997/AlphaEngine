#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace AlphaEngine {

	struct TransformComponent
	{
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;

		TransformComponent(glm::vec3 pos = { 0,0,0 }, glm::vec3 sc = { 1,1,1 }, glm::quat rot = glm::quat(1, 0, 0, 0))
			: position(pos), scale(sc), rotation(rot) {
		}

		glm::mat4 GetTransform() const {

			// Translation
			glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), position);

			// Rotation
			glm::mat4 rotationMat = glm::toMat4(rotation);

			// Scaling
			glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

			return translationMat * rotationMat * scaleMat;
		}
	};
}