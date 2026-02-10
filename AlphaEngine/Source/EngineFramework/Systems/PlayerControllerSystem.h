#pragma once

#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Components/PlayerControllerComponent.h"
#include "EngineFramework/Components/VelocityComponent.h"
#include "EngineFramework/Input.h"
#include <glm/glm.hpp>

namespace AlphaEngine
{
	class PlayerControllerSystem : public System
	{
	public:
		PlayerControllerSystem()
		{
			RequireComponent<PlayerControllerComponent>();
			RequireComponent<VelocityComponent>();
		}

		void RunSystem(ECSOrchestrator& ecs, Input& input)
		{

			for (auto entity : GetSystemEntities()) {

				auto& playerController = ecs.GetComponent<PlayerControllerComponent>(entity);
				auto& velocity = ecs.GetComponent<VelocityComponent>(entity);

				glm::vec3 preferedDir(0.0f);

				// Polling for smooth movement
				if (input.IsKeyDown(GLFW_KEY_W)) preferedDir.z -= 1.0f;
				if (input.IsKeyDown(GLFW_KEY_S)) preferedDir.z += 1.0f;
				if (input.IsKeyDown(GLFW_KEY_A)) preferedDir.x -= 1.0f;
				if (input.IsKeyDown(GLFW_KEY_D)) preferedDir.x += 1.0f;

				if (glm::length(preferedDir) > 0) preferedDir = glm::normalize(preferedDir);

				
				velocity.linearVelocity.x = preferedDir.x * playerController.moveSpeed;
				velocity.linearVelocity.z = preferedDir.z * playerController.moveSpeed;

				if (input.IsKeyPressed(GLFW_KEY_SPACE) && velocity.isGrounded) {
					velocity.linearVelocity.y = playerController.jumpForce;
					velocity.isGrounded = false; // Set it to false here!
				}

				
			}
		}
	};
}