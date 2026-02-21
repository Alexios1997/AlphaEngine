#pragma once

#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Components/RendererComponent.h"
#include "EngineFramework/Components/TransformComponent.h"
#include "EngineFramework/Renderer/IRenderer.h"
#include "EngineFramework/Logger.h"
#include "EngineFramework/AssetManager.h"
#include "EngineFramework/Utility.h"
#include "EngineFramework/Intersection.h"

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
			auto& assetManager = ServiceLocator::Get<AssetManager>();
			auto& cameraComp = ecsOrchestrator.GetComponent<CameraComponent>(mainCam);

			Frustum cameraFrustum = AlphaEngine::FrustumUtils::Extract(cameraComp.viewProj);
			uint32_t renderedCount = 0;

			if (mainCam.IsValid()) {
				// We assume CameraSystem has already calculated 'viewProj'
				// Pass the pre-calculated matrix to the renderer
				// This is where the Renderer will update the UBO
				renderer.SetViewProjection(cameraComp.viewProj, cameraComp.viewMatrix);
			}
			else {
				Logger::Err("Not Valid Camera found!");
				return;
			}
			
			for (auto const& entity : GetSystemEntities())
			{
				if (!ecsOrchestrator.HasComponent<TransformComponent>(entity)) continue;

				auto& transformComp = ecsOrchestrator.GetComponent<TransformComponent>(entity);
				auto& renderComp = ecsOrchestrator.GetComponent<RenderComponent>(entity);


				if (!renderComp.isSkybox)
				{
					// 1. Get the local radius from the Mesh (cached in AssetManager)
					float localRadius = assetManager.GetMeshRadius(renderComp.meshHandler);

					// 2. Scale the radius based on the entity's transform
					float maxScale = glm::max(transformComp.scale.x, glm::max(transformComp.scale.y, transformComp.scale.z));

					Sphere worldSphere;
					worldSphere.center = transformComp.position; // World position
					worldSphere.radius = localRadius * maxScale;

					// 3. Test: If the sphere is outside, don't even build the RenderCommand
					if (!Intersection::Intersects(cameraFrustum, worldSphere)) {
						continue;
					}
				}
				renderedCount++;

				// Build the command
				RenderCommand rCmd;
				rCmd.shaderID = renderComp.shaderHandler.id;
				rCmd.textureID = renderComp.textureHandler.id;
				rCmd.vao = assetManager.GetMeshVAO(renderComp.meshHandler);
				rCmd.indexCount = assetManager.GetMeshIndexCount(renderComp.meshHandler);
				rCmd.transform = transformComp.GetTransform(); // The 4x4 matrix
				rCmd.isCubemap = renderComp.isSkybox;
				rCmd.layerID = renderComp.layerID;
				
				// Optimization check: Checking X and Y axis
				// TODO: Although this will be changed to fit our needs Or What we consider to be invisible!
				if (rCmd.transform[0][0] == 0.0f && rCmd.transform[1][1] == 0.0f) {
					Logger::Log("WARNING: Entity " + std::to_string(entity.GetId()) + " has zero scale.");
					continue; // Skip rendering invisible objects
				}

				if(rCmd.isCubemap)
				{
					// Remove translation (position) so the skybox stays centered
					glm::mat4 staticView = glm::mat4(glm::mat3(cameraComp.viewMatrix));
					rCmd.skyboxVP = cameraComp.projectionMatrix * staticView;
				}

				renderer.FuelRenderCommands(rCmd);
			}

			static int frameCounter = 0;
			if (frameCounter++ % 500 == 0) {
				Logger::Log("[Culling] Entities in view: " + std::to_string(renderedCount));
			}
		}
	};
}