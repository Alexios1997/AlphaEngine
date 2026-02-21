#pragma once

#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Components/CameraComponent.h"
#include "EngineFramework/Components/TransformComponent.h"
#include "EngineFramework/Renderer/IRenderer.h"
#include "EngineFramework/Logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "EngineFramework/EventBus.h"
#include "EngineFramework/WindowEvents.h"

namespace AlphaEngine
{
	class CameraSystem : public System
	{
	private:
		float m_CurrentAspectRatio = 1.777f;

	public:
		CameraSystem()
		{

			EventBus::Subscribe([this](Event& e) {
				EventDispatcher dispatcher(e);
				dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& ev) {
					this->m_CurrentAspectRatio = (float)ev.GetWidth() / (float)ev.GetHeight();
					return false;
					});
				});


			RequireComponent<TransformComponent>();
			RequireComponent<CameraComponent>();
		}

		

		void RunSystem(ECSOrchestrator& ecsOrchestrator) const
		{
			float currentAspect = m_CurrentAspectRatio;

			for (auto const& entity : GetSystemEntities())
			{
				auto& transformComp = ecsOrchestrator.GetComponent<TransformComponent>(entity);
				auto& cameraComp = ecsOrchestrator.GetComponent<CameraComponent>(entity);

				cameraComp.aspect = currentAspect;

				cameraComp.viewMatrix = glm::lookAt(
					transformComp.position, 
					cameraComp.target,      
					cameraComp.up           
				);

				cameraComp.projectionMatrix = glm::perspective(
					glm::radians(cameraComp.fov),
					cameraComp.aspect,
					cameraComp.nearPlane,
					cameraComp.farPlane
				);

				cameraComp.viewProj = cameraComp.projectionMatrix * cameraComp.viewMatrix;
			}


		}
	};
}