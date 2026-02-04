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
			
			for (auto entity : GetSystemEntities())
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

				renderer.FuelRenderCommands(rCmd);
			}

			
		}
	};
}