#pragma once

#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Components/TransformComponent.h"
#include "EngineFramework/Components/VelocityComponent.h"
#include "EngineFramework/Input.h"
#include "EngineFramework/Logger.h"
#include <glm/glm.hpp>

namespace AlphaEngine
{
	class MovementSystem : public System
	{
	public:
		MovementSystem()
		{
			RequireComponent<TransformComponent>();
			RequireComponent<VelocityComponent>();
		}

		void RunSystem(ECSOrchestrator& ecs, float deltaTime)
		{
			for (auto entity : GetSystemEntities()) {
				auto& transform = ecs.GetComponent<TransformComponent>(entity);
				auto& vel = ecs.GetComponent<VelocityComponent>(entity);

				
				if (!vel.isGrounded) {
					vel.linearVelocity.y += vel.gravity * deltaTime;
				}

				transform.position += vel.linearVelocity * deltaTime;

				if (transform.position.y <= 0.0f) {
					transform.position.y = 0.0f;  
					vel.linearVelocity.y = 0.0f; 
					vel.isGrounded = true;       
				}
				else {
					vel.isGrounded = false;
				}
			}
		}
	};

}