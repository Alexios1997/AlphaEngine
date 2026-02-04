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
		std::vector<RenderCommand> m_DrawQueueRCs;

	public:
		void BeginFrame() override;
		void FuelRenderCommands(const RenderCommand& command) override;
		void EndFrame() override;
	};
}