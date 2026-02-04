#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace AlphaEngine {

	struct TransformComponent
	{
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		TransformComponent(glm::vec3 pos = { 0,0,0 }, glm::vec3 sc = { 1,1,1 }, glm::vec3 rot = { 0,0,0 })
			: position(pos), scale(sc), rotation(rot) {
		}

		glm::mat4 GetTransform() const {

			glm::mat4 model = glm::mat4(1.0f);

			// Translation
			model = glm::translate(model, position);

			// Rotation
			model = glm::rotate(model, glm::radians(rotation.x), { 1, 0, 0 });
			model = glm::rotate(model, glm::radians(rotation.y), { 0, 1, 0 });
			model = glm::rotate(model, glm::radians(rotation.z), { 0, 0, 1 });

			// Scaling
			model = glm::scale(model, scale);

			return model;
		}
	};
}