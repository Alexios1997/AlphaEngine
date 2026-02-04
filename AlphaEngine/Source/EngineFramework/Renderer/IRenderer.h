#pragma once

#include <cstdint>        
#include <glm/glm.hpp>

namespace AlphaEngine
{
	// We need a render command to carry enough info so the
	// Renderer can make smart Decisions (like sorting) without asking ECS for more data

	// Think of it as follows:
	// The ECS writes the note: "Hey, draw Mesh X with Shader Y at this Position."
	// The renderer collects all the notes and put them in the best order and executes them
	struct RenderCommand
	{
		// Sorting vars
		uint32_t layerID;    // Keep UI/Game separate
		uint32_t shaderID;   // Group by Material/Shader for speed
		uint32_t textureID;  // Group by Texture
		float depth;         // For Transparency (Back-to-Front) or Opaque (Front-to-Back)

		// GPU Handles
		uint32_t vao;
		uint32_t indexCount;

		// Entity Data
		glm::mat4 transform;
	};

	class IRenderer 
	{
	public:
		virtual ~IRenderer() = default;
		virtual void BeginFrame() = 0;
		virtual void FuelRenderCommands(const RenderCommand& command) = 0;
		virtual void EndFrame() = 0;
	};

}