#pragma once

#include <cstdint>        
#include <glm/glm.hpp>
#include "EngineFramework/ServiceLocator.h"
#include "EngineFramework/Logger.h"

namespace AlphaEngine
{
	// We need a render command to carry enough info so the
	// Renderer can make smart Decisions (like sorting) without asking ECS for more data

	// Think of it as follows:
	// The ECS writes the note: "Hey, draw Mesh X with Shader Y at this Position."
	// The renderer collects all the notes and put them in the best order and executes them
	struct RenderCommand
	{
		// <--- Sorting vars --->
		// 
		// Layer Id makes sure that UI is always drawn last (on top)
		uint32_t layerID;

		// Used for state management, Grouping commands with the same ID, 
		// you only call glUseProgram once for 100 object for example
		uint32_t shaderID; 
		uint32_t textureID;

		// Opaque objects: Sorted Front-to-Back (closest first) to take advantage of Depth Testing (the GPU skips pixels hidden behind other objects).
		// Transparent objects : Sorted Back - to - Front so they blend correctly.
		float depth;

		// <--- GPU Handles --->
		uint32_t vao;
		uint32_t indexCount;

		// <--- Entity Data --->
		glm::mat4 transform;
	};

	class IRenderer : public IService
	{
	public:
		virtual ~IRenderer() = default;
		virtual void InitService() override { Logger::Log("Initializing Service named : IRenderer"); };
		virtual void BeginFrame() = 0;
		virtual void FuelRenderCommands(const RenderCommand& command) = 0;
		virtual void SetViewProjection(const glm::mat4& viewProj) = 0;
		virtual void OnWindowResize(uint32_t width, uint32_t height) = 0;
		virtual void EndFrame() = 0;
	};

}