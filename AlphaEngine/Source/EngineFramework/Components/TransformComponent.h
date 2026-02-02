#pragma once
#include <glm/glm.hpp>

struct TransformComponent
{
	glm::vec3 position;
	glm::vec3 scale;

	TransformComponent(glm::vec3 position = glm::vec3(0, 0, 0), glm::vec3 scale = glm::vec3(1, 1, 1))
	{
		this->position = position;
		this->scale = scale;
	}
};