#pragma once

#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Components/RendererComponent.h"
#include "EngineFramework/Components/TransformComponent.h"
#include "EngineFramework/Renderer/IRenderer.h"
#include "EngineFramework/Logger.h"

namespace AlphaEngine 
{
	class RenderSystem : public System
	{
	public:
		RenderSystem() 
		{
			RequireComponent<TransformComponent>();
			RequireComponent<RenderComponent>();
		}

		void RunSystem(IRenderer& renderer, ECSOrchestrator& ecsOrchestrator) const
		{
			auto mainCam = ecsOrchestrator.GetPrimaryCamera();

			if (mainCam.IsValid()) {
				// We assume CameraSystem has already calculated 'viewProj'
				auto& cameraComp = ecsOrchestrator.GetComponent<CameraComponent>(mainCam);

				// Pass the pre-calculated matrix to the renderer
				// This is where the Renderer will update the UBO
				renderer.SetViewProjection(cameraComp.viewProj);
			}
			else {
				Logger::Err("Not Valid Camera found!");
				return;
			}
			
			for (auto const& entity : GetSystemEntities())
			{
				auto& transformComp = ecsOrchestrator.GetComponent<TransformComponent>(entity);
				auto& renderComp = ecsOrchestrator.GetComponent<RenderComponent>(entity);

				// Build the command
				RenderCommand rCmd;
				rCmd.shaderID = renderComp.shaderID;
				rCmd.textureID = renderComp.textureID;
				rCmd.vao = renderComp.vao;
				rCmd.indexCount = renderComp.indexCount;
				rCmd.transform = transformComp.GetTransform(); // The 4x4 matrix
				rCmd.layerID = 0;
				
				// Optimization check: Checking X and Y axis
				// TODO: Although this will be changed to fit our needs Or What we consider to be invisible!
				if (rCmd.transform[0][0] == 0.0f && rCmd.transform[1][1] == 0.0f) {
					Logger::Log("WARNING: Entity " + std::to_string(entity.GetId()) + " has zero scale.");
					continue; // Skip rendering invisible objects
				}

				renderer.FuelRenderCommands(rCmd);
			}

			
		}
	};
}