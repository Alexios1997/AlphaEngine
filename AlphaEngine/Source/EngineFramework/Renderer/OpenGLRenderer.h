#pragma once

#include "EngineFramework/Renderer/IRenderer.h"
#include "EngineFramework/Logger.h"
#include <vector>
#include <glad/gl.h>
#include <glm/glm.hpp>

namespace AlphaEngine
{
	class OpenGLRenderer : public IRenderer
	{
	private:
		//uint32_t m_CameraUBO;
		//glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ActiveViewProj;
		glm::mat4 m_ActiveView;
		uint32_t m_CameraUBO;
		std::vector<RenderCommand> m_DrawQueueRCs;
		
	public:
		OpenGLRenderer();
		~OpenGLRenderer();
		void BeginFrame() override;
		void FuelRenderCommands(const RenderCommand& command) override;
		void SetViewProjection(const glm::mat4& viewProj, const glm::mat4& viewMatrix) override;
		void OnWindowResize(uint32_t width, uint32_t height) override;
		void EndFrame() override;
	};
}